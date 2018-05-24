#ifndef HANDLE_H
#define HANDLE_H
#include "sortedVector.h"
#include "partialReduction.h"
#include <algorithm>
#include <memory>
#include <set>
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
    double x,y,width,height;
    enum Anchor {ne,nw,se,sw};
    Anchor anchor;
  };

  inline double sqr(double x) {return x*x;}
  // distance squared between two points
  inline double dsq(double x0, double y0, double x1, double y1)
  {return sqr(x1-x0)+sqr(y1-y0);}
  
  class Handle
  {
  private:

    double m_x, m_y, home_x, home_y;
    bool m_collapsed=false;
    bool m_displayFilterCaliper=false;
    /** @{
        cached text extents for the handle description, op and caliper labels
        see ticket #957284
    */
    mutable double descLabelWidth=0, descLabelHeight=0;
    mutable double opLabelWidth=0, opLabelHeight=0;
    mutable double minSliceWidth=0, minSliceHeight=0;
    mutable double maxSliceWidth=0, maxSliceHeight=0;
    /// @}
    friend class Ravel;
    void setHome(double x, double y) {home_x=x; home_y=y; snap();}
    void scaleHome(double scale) {home_x*=scale; home_y*=scale; snap();}
    void swapHome(Handle& x) {
      std::swap(m_x,x.m_x); std::swap(m_y,x.m_y);
      std::swap(home_x,x.home_x); std::swap(home_y,x.home_y);
    }
    /// for caching the original slice labels prior to applying partial reductions
    SortedVector unreducedSliceLabels;
    std::vector<std::shared_ptr<PartialReduction>> m_partialReductions;
    
    CLASSDESC_ACCESS(Handle);
  public:
    static constexpr double hubRadius=0.1;

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
    /// current slice
    size_t sliceIndex;
    /// move slice index by \a p, taking into acount the bounds
    void moveSliceIdx(int p) {
      if (long(sliceIndex)+p >= 0 && long(sliceIndex) + p < long(sliceLabels.size()))
        sliceIndex+=p;
    }

    /// current filter bounds
    size_t sliceMin() const {return sliceLabels.min();}
    size_t sliceMax() const {return sliceLabels.max();}
  
    
    /// masked sliceLabels where slices are empty of all values
    std::set<unsigned long> mask;

    /// interpolates \a x by idx/sliceLabels.size()
    double sliceCoordInterp(size_t idx, double x) const;

    /// @{
    /// coordinates of slice control, relative to ravel origin
    double sliceX() const {return sliceCoordInterp(sliceIndex,m_x);}
    double sliceY() const {return sliceCoordInterp(sliceIndex,m_y);}
    /// @}

    /// @{
    /// display filter caliper 
    bool displayFilterCaliper() const {return m_displayFilterCaliper;}
    bool displayFilterCaliper(bool d);
    ///@}
    
    /// caliper length as a percentage of radius
    static const unsigned caliperLength=7;

    /// @{
    /// coordinates of slice filter caliper control
    double minSliceX() const {return sliceCoordInterp(sliceMin(),m_x);}
    double minSliceY() const {return sliceCoordInterp(sliceMin(),m_y);}
    double maxSliceX() const {return sliceCoordInterp(sliceMax(),m_x);}
    double maxSliceY() const {return sliceCoordInterp(sliceMax(),m_y);}
    /// @}


    /// @{
    /// coordinates of the operation label reference point
    double opX() const;
    double opY() const;
    /// @}

    /// @{ the text data for the description, operation and slice labels
    AnchorPoint labelAnchor() const;
    AnchorPoint opLabelAnchor() const;
    AnchorPoint minCaliperLabelAnchor() const;
    AnchorPoint maxCaliperLabelAnchor() const;
    /// @}

    /// @{ cache text extents
    template <class GC>
    void setLabelExtents(GC& g, const std::string& label, double& width, double& height) const {
      g.setTextExtents(label);
      width=g.textWidth();
      height=g.textHeight();
    }
    template <class GC>
    void setDescLabelExtents(GC& g) const
    {setLabelExtents(g,description,descLabelWidth,descLabelHeight);}
    
    template <class GC>
    void setOpLabelExtents(GC& g) const
    {setLabelExtents(g,opLabels[reductionOp],opLabelWidth,opLabelHeight);}

    template <class GC>
    void setMinSliceLabelExtents(GC& g) const
    {setLabelExtents(g,minSliceLabel(),minSliceWidth,minSliceHeight);}
    
    template <class GC>
    void setMaxSliceLabelExtents(GC& g) const
    {setLabelExtents(g,maxSliceLabel(),maxSliceWidth,maxSliceHeight);}
    /// @}
  
    
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

    Handle(): description("?"), sliceLabels(1), sliceIndex(0) {}

    /// true if handle is collapsed (rolled up dimension)
    bool collapsed() const {return m_collapsed;}
    /// move handle tip to (x,y)
    void moveTo(double x, double y, bool dontCollapse);
    
    /// on mouse button release
    void snap();
    /// toggle collapsed status
    void toggleCollapsed();

    /// sets the slicer to the position occupied by \a label. Does
    /// nothing if \a label is not present
    void setSlicer(const std::string& label);

    /// vector of partial reduction transforms
    const std::vector<std::shared_ptr<PartialReduction>>& partialReductions() const
    {return m_partialReductions;}
    void addPartialReduction(const std::shared_ptr<PartialReduction>&);
    /// ownership of argument passed
    void addPartialReduction(PartialReduction*x)
    {addPartialReduction(std::shared_ptr<PartialReduction>(x));}
    void clearPartialReductions();
  };

    }
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#endif

#include "handle.cd"

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
#endif
