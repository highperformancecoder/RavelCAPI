/*
  Ravel C API. © Ravelation Pty Ltd 2018
*/

#ifndef CAPI_H
#define CAPI_H
#include "capiRenderer.h"
#include <cairo/cairo.h>
#include <stdlib.h>

#define RAVEL_CAPI_VERSION 4

#if defined(__cplusplus) && __cplusplus >= 201103L
#define NOEXCEPT noexcept
#else
#define NOEXCEPT
#endif

struct CAPIRavel;
struct CAPIRavelDC;

#ifdef __cplusplus
namespace ravel
{
  struct HandleState;
  struct RavelState;
}
#define BOOL bool
#else
#define BOOL int
#endif

enum RavelReductionOp {ravel_sum, ravel_prod, ravel_av, ravel_stddev, ravel_min, ravel_max};

/// enum describing the sorting properties of handle
// num* and time* deprecated here
enum RavelOrder {ravel_none, ravel_forward, ravel_reverse, ravel_numForward, ravel_numReverse, ravel_timeForward, ravel_timeReverse, ravel_custom };
enum RavelOrderType {ravel_string, ravel_time, ravel_value};

// interface, for use on CAPI
struct CAPIRavelHandleState
{
  double x,y; ///< handle tip coordinates (only angle important, not length)
  BOOL collapsed, displayFilterCaliper;
  enum RavelReductionOp reductionOp;
  enum RavelOrder order;
  
  // To manage the memory pointed to by these pointers, best
  // practice is to extend this class, and ensure these pointers are
  // updated into internally managed memory. Lifetime of the
  // overall object needs to be clarified on the API. Beware object slicing.
  const char* description;
  const char* minLabel;
  const char* maxLabel;
  const char* sliceLabel;
  const char** customOrder; ///< used if order==custom, null terminated

#ifdef __cplusplus
  explicit CAPIRavelHandleState(double x=0, double y=0, bool collapsed=false,
                       bool displayFilterCaliper=false, RavelReductionOp reductionOp=ravel_sum,
                       RavelOrder order=ravel_none):
    x(x), y(y), collapsed(collapsed), displayFilterCaliper(displayFilterCaliper),
    reductionOp(reductionOp), order(order),
    description(nullptr), minLabel(nullptr), maxLabel(nullptr), sliceLabel(nullptr),
    customOrder(nullptr) {}
  /// initialises just the simple data members
  CAPIRavelHandleState(const ravel::HandleState& state);
#endif
};

struct CAPIRavelState
{
  double radius;
  /// sort 1D ravel by value. Ignored for any other rank.
  enum RavelOrder sortByValue;
  const struct CAPIHandleState** handleStates; ///< null terminated list of handle states
  const char** outputHandles; ///< null terminated list of output handles

#ifdef __cplusplus
  CAPIRavelState(double radius=ravelDefaultRadius, RavelOrder sortByValue=ravel_none):
    radius(radius), sortByValue(sortByValue), handleStates(nullptr), outputHandles(nullptr) {}
  /// initialises just the simple data members
  CAPIRavelState(const RavelState& state): CAPIRavelState(state.radius, state.sortByValue) {}
#endif
};



/// describe the structure of a CSV file. -1 means figure it out from the data
struct CAPIRavelDataSpec
{
  int nRowAxes;      ///< No. rows describing axes
  int nColAxes;      ///< No. cols describing axes
  int nCommentLines; ///< No. comment header lines
  char separator;    ///< field separator character

#ifdef __cplusplus
  CAPIRavelDataSpec(): nRowAxes(-1), nColAxes(-1), nCommentLines(-1), separator(',') {}
#endif
};

#ifdef __cplusplus
extern "C" {
#endif
  /// returns the major version number of this API. This gets bumped
  /// whenever a method is removed or signature changes, but not when
  /// a method is added
  int ravel_capi_version() NOEXCEPT;
  /// returns the what() msg of last exception thrown.
  const char* ravel_lastErr() NOEXCEPT;
  /// returns the release version of this library
  const char* ravel_version() NOEXCEPT;

  /// create a new Ravel control widget
  struct CAPIRavel* ravel_new(size_t rank) NOEXCEPT;
  /// dispose of ravel created by ravel_new()
  void ravel_delete(struct CAPIRavel* ravel) NOEXCEPT;
  /// removes all handles
  void ravel_clear(struct CAPIRavel* ravel) NOEXCEPT;

  /// render ravel widget into a Cairo context
  void ravel_render(struct CAPIRavel* ravel, struct CAPIRenderer* cairo) NOEXCEPT;
  /// @{ handle mouse events
  void ravel_onMouseDown(struct CAPIRavel* ravel, double x, double y) NOEXCEPT;
  void ravel_onMouseUp(struct CAPIRavel* ravel, double x, double y) NOEXCEPT;
  /// handle mouse motion with button pressed
  /// @ return true if it needs to be rerendered
  int ravel_onMouseMotion(struct CAPIRavel* ravel, double x, double y) NOEXCEPT;
  /// hande mouse motion without pressed button (tooltips etc)
  /// @ return true if it needs to be rerendered
  int ravel_onMouseOver(struct CAPIRavel* ravel, double x, double y) NOEXCEPT;
  /// handle mouse movements leaving the ravel
  void ravel_onMouseLeave(struct CAPIRavel* ravel) NOEXCEPT;
  /// @}
  /// resize a ravel
  void ravel_rescale(struct CAPIRavel* ravel, double radius) NOEXCEPT;
  /// radius of ravel in screen coordinates
  double ravel_radius(struct CAPIRavel* ravel) NOEXCEPT;
  /// rank (no. output handles)
  size_t ravel_rank(struct CAPIRavel* ravel) NOEXCEPT;
  /// descriptive text of the operation of the Ravel (plain English for now)
  /// String buffer remains valid until next Ravel CAPI call.
  const char* ravel_description(struct CAPIRavel* ravel) NOEXCEPT;

  /// sets an explanatory message displayed as a tooltip @param
  /// explain a message, if empty, then a default explanatory message
  /// for the item at (x,y) is used
  /// @param x,y location to display the tooltip
  void ravel_setExplain(struct CAPIRavel* ravel, const char* explain, double x, double y) NOEXCEPT;
  /// resets the explain message
  void ravel_resetExplain(struct CAPIRavel* ravel) NOEXCEPT;
  /// obtain default explanatory message for item at (x,y)
  const char* ravel_explain(struct CAPIRavel* ravel, double x, double y) NOEXCEPT;
  
  /// return the handle IDs of the output handles, in order x,y,z, etc.
  /// ids must be ravel_rank() in size
  void ravel_outputHandleIds(struct CAPIRavel* ravel, size_t ids[]) NOEXCEPT;
  /// set the output handles. \a ids[] must be of length \a rank.
  void ravel_setOutputHandleIds(struct CAPIRavel* ravel, size_t rank, size_t ids[]) NOEXCEPT;
  /// number of handles (aka axes)
  unsigned ravel_numHandles(struct CAPIRavel* ravel) NOEXCEPT;
  /// current handle mouse is over, or -1 if none
  int ravel_selectedHandle(struct CAPIRavel* ravel) NOEXCEPT;
  /// return the description field for handle \a handle.
  const char* ravel_handleDescription(struct CAPIRavel* ravel, int handle) NOEXCEPT;
  /// set the description field for \a handle
  void ravel_setHandleDescription(struct CAPIRavel* ravel, int handle, const char* description) NOEXCEPT;
  /// number of slice labels along axis \a axis
  size_t ravel_numSliceLabels(struct CAPIRavel* ravel, size_t axis) NOEXCEPT;
  /** returns the sliceLabels along axis \a axis 
      \a labels must be at least ravel_numSliceLabels in size.  

      The returned pointers are owned by the ravel object, and remain
      valid until the next call to ravel_sliceLabels. Do not delete or
      free these.
  **/
  void ravel_sliceLabels(struct CAPIRavel* ravel, size_t axis, const char* labels[]) NOEXCEPT;
  /// enable/disable the filter calipers on axis \a axis
  void ravel_displayFilterCaliper(struct CAPIRavel* ravel, size_t axis, BOOL display) NOEXCEPT;
  /// set the slicer to \a sliceLabel, if it exists
  void ravel_setSlicer(struct CAPIRavel* ravel, size_t axis, const char* sliceLabel) NOEXCEPT;
  /// set calipers to \a l1, l2
  void ravel_setCalipers(struct CAPIRavel* ravel, size_t axis, const char* l1, const char* l2) NOEXCEPT;
  /// set the ordering on handle \a axis to \a order
  void ravel_orderLabels(struct CAPIRavel* ravel, size_t axis, enum RavelOrder order, enum RavelOrderType, const char* format) NOEXCEPT;

  /// apply a custom permutation of axis labels (which may be less than the number of labels)
  /// indices is an array of length numIndices
  void ravel_applyCustomPermutation(struct CAPIRavel* ravel, size_t axis, size_t numIndices, const size_t* indices) NOEXCEPT;
  /// get the current permutation of axis labels. numIndices is the size of the target array, which should be >=numSliceLabels.
  /// elements beyond numSliceLabels are undefined
  void ravel_currentPermutation(struct CAPIRavel* ravel, size_t axis, size_t numIndices, size_t* indices) NOEXCEPT;
  
  /// add a handle to the Ravel. \a sliceLabels is of length \a numSliceLabels. Ownership of char pointers not passed.
  void ravel_addHandle(struct CAPIRavel* ravel, const char* description,
                       size_t numSliceLabels, const char* sliceLabels[]) NOEXCEPT;
  
  /// return XML represention of ravel. String buffer remains valid until next Ravel CAPI call.
  const char* ravel_toXML(struct CAPIRavel* ravel) NOEXCEPT;
  /// populate with XML data. @return true on success
  int ravel_fromXML(struct CAPIRavel* ravel, const char*) NOEXCEPT;
  /// get the handle state (user modifiable attributes of handle \a
  /// handle). The returned object is valid until the next call to
  /// this function.
  struct CAPIRavelHandleState* ravel_getHandleState(struct CAPIRavel* ravel, size_t handle) NOEXCEPT;
  /// set the handle state
  void ravel_setHandleState(struct CAPIRavel* ravel, size_t handle, const struct CAPIRavelHandleState* handleState) NOEXCEPT;
  /// get the ravel state (user modifiable attributes of handle \a
  /// handle). The returned object is valid until the next call to
  /// this function.
  struct CAPIRavelState* ravel_getRavelState(struct CAPIRavel* ravel) NOEXCEPT;
  /// set the ravel state
  void ravel_setRavelState(struct CAPIRavel* ravel, const struct CAPIRavelState* handleState) NOEXCEPT;
  /// adjust handle slicer mouse is over up or down by n points
  void ravel_adjustSlicer(struct CAPIRavel* ravel, int) NOEXCEPT; 

  /// redistribute handles according to current state
  void ravel_redistributeHandles(struct CAPIRavel* ravel) NOEXCEPT;
  
  /// create a new dataCube object
  struct CAPIRavelDC* ravelDC_new() NOEXCEPT;
  /// delete a datacube object
  void ravelDC_delete(struct CAPIRavelDC*) NOEXCEPT;
  /// initialise a ravel from a loaded datacube. @return true on success
  int ravelDC_initRavel(struct CAPIRavelDC* dc,struct CAPIRavel* ravel) NOEXCEPT;
  /// open a CSV file. Format is described by \a spec. -1 means figure it out from the data. @return true on success
  int ravelDC_openFile(struct CAPIRavelDC* dc, const char* fileName, struct CAPIRavelDataSpec spec) NOEXCEPT;
  /// load data into a datacube. \a ravel specifies the dimensions and labels of the datacube.
  /** on return, the rank of the datacube is the same as the rank of the ravel, ie
      ravelDC_hyperSlice(dc,ravel,dims,data);
      ravelDC_loadData(dc,ravel,data);
      will permanently reduce the datacube to the slice described by \a ravel
  **/
  void ravelDC_loadData(struct CAPIRavelDC* dc, const struct CAPIRavel* ravel, const double data[]) NOEXCEPT;
  /** return a hyperslice corresponding to the ravel's configuration
      The returned data is a dense multidimensional array with the
      dimensions returned as the \a dim parameter, which must be the
      ravel's rank in size.
      The returned data is owned by \a dc, and remains valid until the
      next call of hyperSlice
      @return true on success.  data is set to NULL on failure.
  **/
  int ravelDC_hyperSlice(struct CAPIRavelDC* dc, struct CAPIRavel *ravel, size_t dims[], double **data) NOEXCEPT;
  /// return XML represention of
  const char* ravelDC_toXML(struct CAPIRavelDC*) NOEXCEPT;
  /// populate with XML data
  void ravelDC_fromXML(struct CAPIRavelDC*, const char*) NOEXCEPT;

#ifdef __cplusplus
}
#endif
#endif
