/*
  Ravel C API. © Ravelation Pty Ltd 2018
*/

#ifndef CAPI_H
#define CAPI_H
#include "capiRenderer.h"
#include <cairo/cairo.h>
#include <stdlib.h>

#define RAVEL_CAPI_VERSION 2

#if defined(__cplusplus) && __cplusplus >= 201103L
#define NOEXCEPT noexcept
#else
#define NOEXCEPT
#endif

struct CAPIRavel;
struct CAPIRavelDC;

/// describe the structure of a CSV file. -1 means figure it out from the data
struct CAPIRavelDataSpec
{
  int nRowAxes=-1; ///< No. rows describing axes
  int nColAxes=-1; ///< No. cols describing axes
  int nCommentLines=-1; ///< No. comment header lines
  char separator=','; ///< field separator character
};

struct CAPIHandleState
{
  double x,y; ///< handle tip coordinates (only angle important, not length)
  size_t sliceIndex, sliceMin, sliceMax;
  bool collapsed, displayFilterCaliper;
  enum ReductionOp {sum, prod, av, stddev, min, max};
  ReductionOp reductionOp;
  enum HandleSort {none, forward, reverse, numForward, numReverse, custom};
  HandleSort order;
};

#ifdef __cplusplus
extern "C"
#endif
{
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
  double ravel_radius(CAPIRavel* ravel) NOEXCEPT;
  size_t ravel_rank(CAPIRavel* ravel) NOEXCEPT;

  /// return the handle IDs of the output handles, in order x,y,z, etc.
  /// ids must be ravel_rank() in size
  void ravel_outputHandleIds(CAPIRavel* ravel, size_t ids[]) NOEXCEPT;
  /// set the output handles. \a ids[] must be of length \a rank.
  void ravel_setOutputHandleIds(CAPIRavel* ravel, size_t rank, size_t ids[]) NOEXCEPT;
  /// number of handles (aka axes)
  unsigned ravel_numHandles(CAPIRavel* ravel) NOEXCEPT;
  /// current handle mouse is over, or -1 if none
  int ravel_selectedHandle(CAPIRavel* ravel) NOEXCEPT;
  /// return the description field for handle \a handle.
  const char* ravel_handleDescription(CAPIRavel* ravel, size_t handle) NOEXCEPT;
  /// number of slice labels along axis \a axis
  size_t ravel_numSliceLabels(CAPIRavel* ravel, size_t axis) NOEXCEPT;
  /** returns the sliceLabels along axis \a axis 
      \a labels must be at least ravel_numSliceLabels in size.  

      The returned pointers are owned by the ravel object, and remain
      valid until the next call to ravel_sliceLabels. Do not delete or
      free these.
  **/
  void ravel_sliceLabels(CAPIRavel* ravel, size_t axis, const char* labels[]) NOEXCEPT;
  /// enable/disable the filter calipers on axis \a axis
  void ravel_displayFilterCaliper(CAPIRavel* ravel, size_t axis, bool display) NOEXCEPT;
  /// set the ordering on handle \a axis to \a order
  void ravel_orderLabels(CAPIRavel* ravel, size_t axis, CAPIHandleState::HandleSort order) NOEXCEPT;

  /// add a handle to the Ravel. \a sliceLabels is of length \a numSliceLabels. Ownership of char pointers not passed.
  void ravel_addHandle(CAPIRavel* ravel, const char* description,
                       size_t numSliceLabels, const char* sliceLabels[]) NOEXCEPT;
  
  /// return XML represention of
  const char* ravel_toXML(CAPIRavel* ravel) NOEXCEPT;
  /// populate with XML data. @return true on success
  int ravel_fromXML(CAPIRavel* ravel, const char*) NOEXCEPT;
  /// get the handle state (user modifiable attributes of handle \a handle
  void ravel_getHandleState(const CAPIRavel* ravel, size_t handle, CAPIHandleState* handleState) NOEXCEPT;
  /// set the handle state
  void ravel_setHandleState(CAPIRavel* ravel, size_t handle, const CAPIHandleState* handleState) NOEXCEPT;
  /// adjust handle slicer mouse is over up or down by n points
  void ravel_adjustSlicer(CAPIRavel* ravel, int) NOEXCEPT; 
  
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
}
#endif
