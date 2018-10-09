#ifndef RAVEL_H
#define RAVEL_H

#include "handle.h"

#include "cda.h"
#include <math.h>

namespace ravel
{
  class Ravel
  {
  public:
    enum ElementMoving {none, handle, slicer, filterMin, filterMax, hub};
  protected:
    double m_radius;
    CLASSDESC_ACCESS(Ravel);

    /// id of handle for previous event. Used for tracking motion events
    int lastHandle=-1;
    ElementMoving elementMoving;
    bool moved=false; // indicates if a mouse motion even has been
                      // received since onMouseDown

  public:

    /// coordinates of Ravel origin
    double x=0,y=0;
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
    std::vector<size_t> handleIds;
    
    /// returns true if the ith handle is an output handle
    bool isOutputHandle(size_t i) const {
      return std::find(handleIds.begin(), handleIds.end(), i)
        !=handleIds.end();
    }
    
    bool isOutputHandle(const Handle& h) const {
      return isOutputHandle(&h-&*handles.begin());
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

    /// return an explanatory message about the item at (x,y)
    const char* explain(double x, double y) const;
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
