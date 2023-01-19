/*
  Ravel C API. © Ravelation Pty Ltd 2020
  Open source licensed under the MIT license.
*/

#ifndef RAVEL_DYNAMMICRAVEL_H
#define RAVEL_DYNAMMICRAVEL_H

#include "ravelState.h"
#include "hypercube.h"
struct CAPIRavel;
struct CAPIRavelDC;
struct CAPIRenderer;

namespace ravel
{
  class Ravel
  {
    CAPIRavel* ravel=nullptr;
    Ravel(const Ravel&)=delete;
    void operator=(const Ravel&)=delete;
  public:
    Ravel();
    ~Ravel();
    /// true if Ravel availabe for use
    operator bool() const {return ravel;}
    /// true if ravel is available on the system
    static bool available();
    /// returns last ravel error message
    static std::string lastError();
    /// ravel version (if successfully loaded)
    static std::string version();
    /// number of days until expired - -ve values means expired.
    static int daysUntilExpired();
    /// removes all handles
    void clear();
    /// render ravel widget into a Cairo context
    void render(CAPIRenderer&) const;
    /// @{ handle mouse events
    void onMouseDown(double x, double y);
    void onMouseUp(double x,double y);
    /// handle mouse motion with button pressed
    /// @ return true if it needs to be rerendered
    bool onMouseMotion(double x,double y);
    /// hande mouse motion without pressed button (tooltips etc)
    /// @ return true if it needs to be rerendered
    bool onMouseOver(double x,double y);
    /// handle mouse movements leaving the ravel
    void onMouseLeave();
    /// @}
    /// resize a ravel
    void rescale(double radius);
    /// radius of ravel in screen coordinates
    double radius() const;
    /// rank (no. output handles)
    size_t rank() const;
    /// descriptive text of the operation of the Ravel (plain English for now)
    std::string description() const;

  /// sets an explanatory message displayed as a tooltip @param
  /// explain a message, if empty, then a default explanatory message
  /// for the item at (x,y) is used
  /// @param x,y location to display the tooltip
    void setExplain(const std::string& explain, double x, double y);
    /// resets the explain message
    void resetExplain();
    /// obtain default explanatory message for item at (x,y)
    std::string explain(double x, double y);
  
    /// return the handle IDs of the output handles, in order x,y,z, etc.
    std::vector<size_t> outputHandleIds() const;
    /// set the output handles. \a ids[] must be of length \a rank.
    void setOutputHandleIds(const std::vector<size_t>& ids);
    /// number of handles (aka axes)
    unsigned numHandles() const;
    /// current handle mouse is over, or -1 if none
    int selectedHandle() const;
    /// return the description field for handle \a handle.
    std::string handleDescription(int handle) const;
    /// set the description field for \a handle
    void setHandleDescription(int handle, const std::string& description);
    /// number of slice labels along axis \a axis
    size_t numSliceLabels(size_t axis) const;
    /// number of slice labels along axis \a axis, unrestricted by calipers
    size_t numAllSliceLabels(size_t axis) const;
    /// returns the sliceLabels along axis \a axis 
    std::vector<std::string> sliceLabels(size_t axis) const;
    /// returns all slice labels, unrestricted by calipers, sorted according to order
    std::vector<std::string> allSliceLabels(size_t axis, HandleSort::Order order) const;
    
    /// enable/disable the filter calipers on axis \a axis
    void displayFilterCaliper(size_t axis, bool display);
    /// set the slicer to \a sliceLabel, if it exists
    void setSlicer(size_t axis, const std::string& sliceLabel);
    /// set calipers to \a l1, l2
    void setCalipers(size_t axis, const std::string& l1, const std::string& l2);
    /// set the ordering on handle \a axis to \a order
    void orderLabels(size_t axis, HandleSort::Order order, HandleSort::OrderType, const std::string& format);

    /// sets the type of the next reduction operation
    void nextReduction(Op::ReductionOp);
    /// set the reduction type for \a handle
    void handleSetReduction(int handle, Op::ReductionOp);
    
    /// apply a custom permutation of axis labels (which may be less than the number of labels)
    /// indices is an array of length numIndices
    void applyCustomPermutation(size_t axis, const std::vector<size_t>& indices);
    /// get the current permutation of axis labels.
    std::vector<size_t> currentPermutation(size_t axis) const;
  
    /// add a handle to the Ravel. \a sliceLabels is of length \a numSliceLabels. Ownership of char pointers not passed.
    void addHandle(const std::string& description, const std::vector<std::string>& sliceLabels);
  
    /// return XML represention of ravel. String buffer remains valid until next Ravel CAPI call.
    std::string toXML() const;
    /// populate with XML data.
    void fromXML(const std::string&);
    /// get the handle state (user modifiable attributes of handle \a
    /// handle).
    HandleState getHandleState(size_t handle) const;
    /// set the handle state
    void setHandleState(size_t handle, const HandleState& handleState);
    /// get the ravel state (user modifiable attributes of handle \a
    /// handle).
    RavelState getRavelState() const;
    /// set the ravel state
    void setRavelState(const RavelState& handleState);
    /// adjust handle slicer mouse is over up or down by n points
    void adjustSlicer(int n);

    /// redistribute handles according to current state
    void redistributeHandles();

        /// sort a rank 1 tensor by value along its output handle, given \a input, in direction \a dir
    /// If rank != 1, this method does nothing.
    void sortByValue(const civita::TensorPtr& input, HandleSort::Order dir);

    /// Return a tensor expression representing the application of this ravel on \a arg
    civita::TensorPtr hyperSlice(const civita::TensorPtr& arg) const;

    /// sets handles and slices from \a hc
    void populateFromHypercube(const civita::Hypercube& hc);

    
  };
}

#if defined(CLASSDESC) || defined(ECOLAB_LIB)
#include "dynamicRavelCAPI.cd"
#endif

#endif
