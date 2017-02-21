#ifndef RAVEL_H
#define RAVEL_H

#include "sortedVector.h"

#include <vector>
#include <string>
#include <algorithm>
#include "cda.h"
#include <set>
#include <math.h>

namespace ravel
{
  struct Op
  {
    enum ReductionOp {sum, prod, av, stddev, min, max};
  };

  // wasteful, but declaring this here ensures one can line up the
  // number of labels with the number of ReductionOps
  static const char* opLabels[]={"Σ","Π","av","σ","min","max"};

  // represents an anchor point for text, including alignment
  struct AnchorPoint
  {
    double x,y;
    enum Anchor {ne,nw,se,sw};
    Anchor anchor;
  };


  class Handle
  {
  private:

    double m_x, m_y, home_x, home_y;
    bool m_collapsed=false;
    friend class Ravel;
    void setHome(double x, double y) {home_x=x; home_y=y; snap();}
    void scaleHome(double scale) {home_x*=scale; home_y*=scale; snap();}
    void swapHome(Handle& x) {
      std::swap(m_x,x.m_x); std::swap(m_y,x.m_y);
      std::swap(home_x,x.home_x); std::swap(home_y,x.home_y);
    }
    CLASSDESC_ACCESS(Handle);
  public:
    /// @{
    /// x & y coordinates of handle tip, relative to Ravel origin
    double x() const {return m_x;}
    double y() const {return m_y;}
    /// @}

    Op::ReductionOp reductionOp=Op::sum; // op used when collapsing handle

    /// descriptor of dimension represented by this handle
    std::string description;

    /// as above if uncollapsed, otherwise something like "sum of xxx" if collapsed
    std::string reductionDescription() const;   
    /// labels of individual slices along this dimension
    SortedVector sliceLabels;
    size_t numSliceLabels() const {return sliceLabels.size();}
    /// current slice, filter bounds
    size_t sliceIndex, sliceMin=0, sliceMax=std::numeric_limits<size_t>::max();

    /// masked sliceLabels (used by filter functionality to remove
    /// empty row/cols from output sheet
    std::set<unsigned long> mask;

    /// interpolates \a x by idx/sliceLabels.size()
    double sliceCoordInterp(size_t idx, double x) const
    {return (x*(idx+1))/(sliceLabels.size()+1);}

    /// @{
    /// coordinates of slice control, relative to ravel origin
    double sliceX() const {return sliceCoordInterp(sliceIndex,m_x);}
    double sliceY() const {return sliceCoordInterp(sliceIndex,m_y);}
    /// @}

    /// display filter caliper 
    bool displayFilterCaliper=false;
    /// caliper length as a percentage of radius
    static const unsigned caliperLength=7;

    /// @{
    /// coordinates of slice filter caliper control
    double minSliceX() const {return sliceCoordInterp(sliceMin,m_x);}
    double minSliceY() const {return sliceCoordInterp(sliceMin,m_y);}
    double maxSliceX() const 
    {return sliceCoordInterp(std::min(sliceMax,sliceLabels.size()-1),m_x);}
    double maxSliceY() const 
    {return sliceCoordInterp(std::min(sliceMax,sliceLabels.size()-1),m_y);}
    /// @}


    /// @{
    /// coordinates of the operation label reference point
    double opX() const;
    double opY() const;
    /// @}

    /// return the anchor point for the axis label
    AnchorPoint labelAnchor() const;

    /// set coordinates of a slice control, relative to ravel
    /// origin, which snap to the nearest available slice, recorded in \a index
    void setSliceCoordinates(size_t& index,double x, double y);
      
    

    /// label of current slice
    const std::string& sliceLabel() const; 
    /// @{
    /// filter caliper labels
    const std::string& minSliceLabel() const;
    const std::string& maxSliceLabel() const;
    /// @}

    Handle(): description("?"), sliceLabels(1), sliceIndex(0)  {}

    /// true if handle is collapsed (rolled up dimension)
    bool collapsed() const {return m_collapsed;}
    /// move handle tip to (x,y)
    void moveTo(double x, double y, bool dontCollapse);
    
    /// on mouse button release
    void snap();
    /// toggle collapsed status
    void toggleCollapsed();
  };

  class Ravel
  {
  public:
    enum ElementMoving {handle, slicer, filterMin, filterMax};
  protected:
    double m_radius;
    CLASSDESC_ACCESS(Ravel);

    /// id of handle for previous event. Used for tracking motion events
    int lastHandle=-1;
    ElementMoving elementMoving;

  public:

    /// coordinates of Ravel origin
    double x,y;
    Op::ReductionOp nextRedOp=Op::sum;
    /// prevent changes to handle vector, but each handle is fully writable
    class Handles: private std::vector<Handle> 
    {
      typedef std::vector<Handle> Super;
      friend class Ravel;
      CLASSDESC_ACCESS(Handles);
      void addHandle(const std::string& description, 
                     const std::vector<std::string>& sliceLabels);
      void clearHandles() {Super::clear();}
    public:
      using Super::operator[];
      using Super::begin;
      using Super::end;
      using Super::empty;
      using Super::size_type;
      using Super::value_type;
      using Super::const_iterator;
      using Super::iterator;
      const Handle& at(size_t i) const {return Super::operator[](i);}
      size_t size() const {return Super::size();}
      void clear() {/* clear does nothing by design */}
      void resize(size_type) {/* clear does nothing by design */}
      void push_back(value_type) {/* clear does nothing by design */}
      void emplace(value_type) {/* clear does nothing by design */}
    };

    /// the handles assciated with this Ravel
    Handles handles;

    /// @{
    /// global coordinates of handle tip
    double handleX(size_t handle) const {return handles[handle].x()+x;}
    double handleY(size_t handle) const {return handles[handle].y()+y;}
    ///@}

    /// @{
    /// global coordinates of slice control of \a handle
    double sliceX(size_t handle) const {return handles[handle].sliceX()+x;}
    double sliceY(size_t handle) const {return handles[handle].sliceY()+y;}
    ///@}

    /// set coordinates of the slice control, which snap to the
    /// nearest available slice
    void setSliceCoordinates(size_t handle, double xs, double ys)
    {handles[handle].setSliceCoordinates(handles[handle].sliceIndex,xs-x,ys-y);}

    /// return number of output handles 
    size_t rank() const {return handleIds.size();}
    /// indices of the handles representing x, y, z etc coordinates
    std::vector<size_t> handleIds{0,1};
    
    /// set output handles
    //    void setOutputHandle(size_t); 

    /// returns true if the ith handle is an output handle
    bool isOutputHandle(size_t i) const {
      return std::find(handleIds.begin(), handleIds.end(), i)
        !=handleIds.end();
    }

    /// distribute handles uniformly around the 1st to 3rd quadrants
    void redistributeHandles();
    
    Ravel(double radius=1): m_radius(radius) {}

    /// add a handle (and dimension it controls) to system
    /// @return handleId for newly added handle
    size_t addHandle(const std::string& description="", 
                  const std::vector<std::string>& sliceLabels=
                  std::vector<std::string>());
    void clear() {handles.clearHandles();}
    /// move \a handle to \a x, \a y due to mouse motion
    void moveHandleTo(unsigned handle, double xx, double yy); 
    /// snap handle to final position on mouse up
    void snapHandle(unsigned handle);

    //size of the Ravel
    double radius() const {return m_radius;}

    /// rescale item to \a radius
    void rescale(double radius);

    /// returns a list of slice labels, looped over the collapsed handles
    //    std::vector<std::vector<std::string> > rolledupKeys() const;

    /// handle mouse motion events. Returns true if the ravel has
    /// changed, and needs to be redrawn
    bool onMouseMotion(double x, double y);
    /// handle mouse button press events
    void onMouseDown(double x, double y);
    /// handle mouse button release events
    void onMouseUp(double x, double y);

    /// returns handle if mouse is over a handle, -1 if not. If \a
    /// exclude is provided, do not check that handle
    /// \a x and \a y are relative to Ravel origin, not window coordinates
    //int handleIfMouseOver(double x, double y) const;
    int handleIfMouseOver(double x, double y, int exclude=-1) const;
    /// returns what element of \a handle is selected
    ElementMoving sliceCtlHandle(int handle, double x, double y) const;

    /// descriptive text of the operation of the Ravel (plain English for now)
    std::string description() const;

  };

}

#if defined(CLASSDESC) || defined(ECOLAB_LIB)
#ifdef _CLASSDESC
// this is supplied in ravelTCL.h
#pragma omit TCL_obj ravel::Ravel::Handles
#endif

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#endif

#include "ravel.cd"

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
#endif
#endif
