/*
  Ravel C API. © Ravelation Pty Ltd 2018
*/

#ifndef RAVELCAPI_H
#define RAVELCAPI_H
#include "capiRenderer.h"
#include "capiCivita.h"
#include <cairo/cairo.h>
#include <stdlib.h>

#if defined(__cplusplus) && __cplusplus >= 201103L
#define NOEXCEPT noexcept
#else
#define NOEXCEPT
#endif

struct CAPIRavel;
typedef struct CAPIRavel CAPIRavel;
struct CAPIRavelDC;
typedef struct CAPIRavelDC CAPIRavelDC;

#include "ravelCAPITypes.h"

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

typedef struct CAPIRavelDataSpec CAPIRavelDataSpec;

#ifdef __cplusplus
extern "C" {
#endif
  /// returns number of days until license expires
  int ravel_days_until_expiry() NOEXCEPT;
  /// returns the major version number of this API. This gets bumped
  /// whenever a method is removed or signature changes, but not when
  /// a method is added
  int ravel_capi_version() NOEXCEPT;
  /// returns the what() msg of last exception thrown.
  const char* ravel_lastErr() NOEXCEPT;
  /// returns the release version of this library
  const char* ravel_version() NOEXCEPT;

  /// create a new Ravel control widget
  CAPIRavel* ravel_new(size_t rank) NOEXCEPT;
  /// dispose of ravel created by ravel_new()
  void ravel_delete(CAPIRavel* ravel) NOEXCEPT;
  /// removes all handles
  void ravel_clear(CAPIRavel* ravel) NOEXCEPT;

  /// render ravel widget into a Cairo context
  void ravel_render(CAPIRavel* ravel, CAPIRenderer* cairo) NOEXCEPT;
  /// @{ handle mouse events
  void ravel_onMouseDown(CAPIRavel* ravel, double x, double y) NOEXCEPT;
  void ravel_onMouseUp(CAPIRavel* ravel, double x, double y) NOEXCEPT;
  /// handle mouse motion with button pressed
  /// @ return true if it needs to be rerendered
  int ravel_onMouseMotion(CAPIRavel* ravel, double x, double y) NOEXCEPT;
  /// hande mouse motion without pressed button (tooltips etc)
  /// @ return true if it needs to be rerendered
  int ravel_onMouseOver(CAPIRavel* ravel, double x, double y) NOEXCEPT;
  /// handle mouse movements leaving the ravel
  void ravel_onMouseLeave(CAPIRavel* ravel) NOEXCEPT;
  /// @}
  /// resize a ravel
  void ravel_rescale(CAPIRavel* ravel, double radius) NOEXCEPT;
  /// radius of ravel in screen coordinates
  double ravel_radius(CAPIRavel* ravel) NOEXCEPT;
  /// rank (no. output handles)
  size_t ravel_rank(CAPIRavel* ravel) NOEXCEPT;
  /// descriptive text of the operation of the Ravel (plain English for now)
  /// String buffer remains valid until next Ravel CAPI call.
  const char* ravel_description(CAPIRavel* ravel) NOEXCEPT;

  /// sets an explanatory message displayed as a tooltip @param
  /// explain a message, if empty, then a default explanatory message
  /// for the item at (x,y) is used
  /// @param x,y location to display the tooltip
  void ravel_setExplain(CAPIRavel* ravel, const char* explain, double x, double y) NOEXCEPT;
  /// resets the explain message
  void ravel_resetExplain(CAPIRavel* ravel) NOEXCEPT;
  /// obtain default explanatory message for item at (x,y)
  const char* ravel_explain(CAPIRavel* ravel, double x, double y) NOEXCEPT;
  
  /// return the handle IDs of the output handles, in order x,y,z, etc.
  /// ids must be ravel_rank() in size
  void ravel_outputHandleIds(CAPIRavel* ravel, size_t ids[]) NOEXCEPT;
  /// set the output handles. \a ids[] must be of length \a rank.
  void ravel_setOutputHandleIds(CAPIRavel* ravel, size_t rank, const size_t ids[]) NOEXCEPT;
  /// number of handles (aka axes)
  unsigned ravel_numHandles(CAPIRavel* ravel) NOEXCEPT;
  /// current handle mouse is over, or -1 if none
  int ravel_selectedHandle(CAPIRavel* ravel) NOEXCEPT;
  /// return the description field for handle \a handle.
  const char* ravel_handleDescription(CAPIRavel* ravel, int handle) NOEXCEPT;
  /// set the description field for \a handle
  void ravel_setHandleDescription(CAPIRavel* ravel, int handle, const char* description) NOEXCEPT;
  /// number of slice labels along axis \a axis
  size_t ravel_numSliceLabels(CAPIRavel* ravel, size_t axis) NOEXCEPT;
  /// number of slice labels along axis \a axis, disregarding calipers
  size_t ravel_numAllSliceLabels(CAPIRavel* ravel, size_t axis) NOEXCEPT;
  /** returns the sliceLabels along axis \a axis 
      \a labels must be at least ravel_numSliceLabels in size.  

      The returned pointers are owned by the ravel object, and remain
      valid until the next call to ravel_sliceLabels. Do not delete or
      free these.
  **/
  void ravel_sliceLabels(CAPIRavel* ravel, size_t axis, const char* labels[]) NOEXCEPT;
  /** returns the sliceLabels along axis \a axis 
      \a labels must be at least ravel_numAllSliceLabels in size.  

      The returned pointers are owned by the ravel object, and remain
      valid until the next call to ravel_sliceLabels. Do not delete or
      free these.
  **/
  void ravel_allSliceLabels(CAPIRavel* ravel, size_t axis, enum RavelOrder order, const char* labels[]) NOEXCEPT;
  /// enable/disable the filter calipers on axis \a axis
  void ravel_displayFilterCaliper(CAPIRavel* ravel, size_t axis, BOOL display) NOEXCEPT;
  /// set the slicer to \a sliceLabel, if it exists
  void ravel_setSlicer(CAPIRavel* ravel, size_t axis, const char* sliceLabel) NOEXCEPT;
  /// set calipers to \a l1, l2
  void ravel_setCalipers(CAPIRavel* ravel, size_t axis, const char* l1, const char* l2) NOEXCEPT;
  /// set the ordering on handle \a axis to \a order
  void ravel_orderLabels(CAPIRavel* ravel, size_t axis, enum RavelOrder order, enum RavelOrderType, const char* format) NOEXCEPT;

  /// sets the type of the next reduction operation
  void ravel_nextReduction(CAPIRavel* ravel, enum RavelReductionOp) NOEXCEPT;
  /// set the reduction type for \a handle
  void ravel_handleSetReduction(CAPIRavel* ravel, int handle, enum RavelReductionOp) NOEXCEPT;
  
  /// apply a custom permutation of axis labels (which may be less than the number of labels)
  /// indices is an array of length numIndices
  void ravel_applyCustomPermutation(CAPIRavel* ravel, size_t axis, size_t numIndices, const size_t* indices) NOEXCEPT;
  /// get the current permutation of axis labels. numIndices is the size of the target array, which should be >=numSliceLabels.
  /// elements beyond numSliceLabels are undefined
  void ravel_currentPermutation(CAPIRavel* ravel, size_t axis, size_t numIndices, size_t* indices) NOEXCEPT;
  
  /// add a handle to the Ravel. \a sliceLabels is of length \a numSliceLabels. Ownership of char pointers not passed.
  void ravel_addHandle(CAPIRavel* ravel, const char* description,
                       size_t numSliceLabels, const char* sliceLabels[]) NOEXCEPT;
  
  /// return XML represention of ravel. String buffer remains valid until next Ravel CAPI call.
  const char* ravel_toXML(CAPIRavel* ravel) NOEXCEPT;
  /// populate with XML data. @return true on success
  int ravel_fromXML(CAPIRavel* ravel, const char*) NOEXCEPT;
  /// get the handle state (user modifiable attributes of handle \a
  /// handle). The returned object is valid until the next call to
  /// this function.
  const CAPIRavelHandleState* ravel_getHandleState(CAPIRavel* ravel, size_t handle) NOEXCEPT;
  /// set the handle state
  void ravel_setHandleState(CAPIRavel* ravel, size_t handle, const CAPIRavelHandleState* handleState) NOEXCEPT;
  /// get the ravel state (user modifiable attributes of handle \a
  /// handle). The returned object is valid until the next call to
  /// this function. Never returns NULL.
  const CAPIRavelState* ravel_getRavelState(CAPIRavel* ravel) NOEXCEPT;
  /// set the ravel state
  void ravel_setRavelState(CAPIRavel* ravel, const CAPIRavelState* handleState) NOEXCEPT;
  /// adjust handle slicer mouse is over up or down by n points
  void ravel_adjustSlicer(CAPIRavel* ravel, int) NOEXCEPT; 

  /// redistribute handles according to current state
  void ravel_redistributeHandles(CAPIRavel* ravel) NOEXCEPT;

  /// arrange for the output handle to be sorted in direction \a dir
  /// Has no effect unless \a ravel is rank 1.
  void ravel_sortByValue(CAPIRavel* ravel, const CAPITensor* input, enum RavelOrder dir) NOEXCEPT;

  /// Return a tensor expression representing the application of this ravel on \a arg
  const CAPITensor* ravel_hyperSlice(CAPIRavel* ravel, const CAPITensor* arg) NOEXCEPT;

  /// sets handles and slices from \a hc
  /// @param hc is JSON representation of the hypercube. See \a civita::Hypercube::json()
  void ravel_populateFromHypercube(CAPIRavel* ravel, const char* hc) NOEXCEPT;

  
  /// create a new dataCube object
  CAPIRavelDC* ravelDC_new() NOEXCEPT;
  /// delete a datacube object
  void ravelDC_delete(CAPIRavelDC*) NOEXCEPT;
  /// initialise a ravel from a loaded datacube. @return true on success
  int ravelDC_initRavel(CAPIRavelDC* dc,CAPIRavel* ravel) NOEXCEPT;
  /// open a CSV file. Format is described by \a spec. -1 means figure it out from the data. @return true on success
  int ravelDC_openFile(CAPIRavelDC* dc, const char* fileName, CAPIRavelDataSpec spec) NOEXCEPT;
  /// load data into a datacube. \a ravel specifies the dimensions and labels of the datacube.
  /** on return, the rank of the datacube is the same as the rank of the ravel, ie
      ravelDC_hyperSlice(dc,ravel,dims,data);
      ravelDC_loadData(dc,ravel,data);
      will permanently reduce the datacube to the slice described by \a ravel
  **/
  void ravelDC_loadData(CAPIRavelDC* dc, const CAPIRavel* ravel, const double data[]) NOEXCEPT;
  /** return a hyperslice corresponding to the ravel's configuration
      The returned data is a dense multidimensional array with the
      dimensions returned as the \a dim parameter, which must be the
      ravel's rank in size.
      The returned data is owned by \a dc, and remains valid until the
      next call of hyperSlice
      @return true on success.  data is set to NULL on failure.
  **/
  int ravelDC_hyperSlice(CAPIRavelDC* dc, CAPIRavel *ravel, size_t dims[], double **data) NOEXCEPT;
  /// return XML represention of
  const char* ravelDC_toXML(CAPIRavelDC*) NOEXCEPT;
  /// populate with XML data
  void ravelDC_fromXML(CAPIRavelDC*, const char*) NOEXCEPT;

#ifdef __cplusplus
}
#endif
#if defined(CLASSDESC) || defined(ECOLAB_LIB)
#include "ravelCAPI.cd"
#endif
#endif
