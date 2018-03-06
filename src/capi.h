/*
  Ravel C API. © Ravelation Pty Ltd 2018
*/

#ifndef CAPI_H
#define CAPI_H
#include "capiRenderer.h"
#include <cairo/cairo.h>
#include <stdlib.h>

#define RAVEL_CAPI_VERSION 1

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

#ifdef __cplusplus
extern "C"
#endif
{
  /// returns the major version number of this API. This gets bumped
  /// whenever a method is removed or signature changes, but not when
  /// a method is added
  int ravel_capi_version() noexcept;
  /// returns the last exception thrown.
  /// NULL returned if the last command completed successfully
  const char* ravel_lastErr() noexcept;
  /// returns the release version of this library
  const char* ravel_version() noexcept;

  /// create a new Ravel control widget
  CAPIRavel* ravel_new(size_t rank) noexcept;
  /// dispose of ravel created by ravel_new()
  void ravel_delete(CAPIRavel* ravel) noexcept;
  /// render ravel widget into a Cairo context
  void ravel_render(CAPIRavel* ravel, CAPIRenderer* cairo) noexcept;
  /// @{ handle mouse events
  void ravel_onMouseDown(CAPIRavel* ravel, double x, double y) noexcept;
  void ravel_onMouseUp(CAPIRavel* ravel, double x, double y) noexcept;
  /// handle mouse motion with button pressed
  /// @ return true if it needs to be rerendered
  int ravel_onMouseMotion(CAPIRavel* ravel, double x, double y) noexcept;
  /// hande mouse motion without pressed button (tooltips etc)
  /// @ return true if it needs to be rerendered
  int ravel_onMouseOver(CAPIRavel* ravel, double x, double y) noexcept;
  /// handle mouse movements leaving the ravel
  void ravel_onMouseLeave(CAPIRavel* ravel) noexcept;
  /// @}
  /// resize a ravel
  void ravel_rescale(CAPIRavel* ravel, double radius) noexcept;
  double ravel_radius(CAPIRavel* ravel) noexcept;
  size_t ravel_rank(CAPIRavel* ravel) noexcept;

  /// return the handle IDs of the output handles, in order x,y,z, etc.
  /// ids must be ravel_rank() in size
  void ravel_outputHandleIds(CAPIRavel* ravel, size_t ids[]) noexcept;
  /// number of slice labels along axis \a axis
  size_t ravel_numSliceLabels(CAPIRavel* ravel, size_t axis) noexcept;
  /** returns the sliceLabels along axis \a axis 
      \a labels must be at least ravel_numSliceLabels in size.  

      The returned pointers are owned by the ravel object, and remain
      valid until the next call to ravel_sliceLabels. Do not delete or
      free these.
  **/
  void ravel_sliceLabels(CAPIRavel* ravel, size_t axis, const char* labels[]) noexcept;
  /// return XML represention of
  const char* ravel_toXML(CAPIRavel* ravel) noexcept;
  /// populate with XML data. @return true on success
  int ravel_fromXML(CAPIRavel* ravel, const char*) noexcept;

  /// create a new dataCube object
  CAPIRavelDC* ravelDC_new() noexcept;
  /// delete a datacube object
  void ravelDC_delete(CAPIRavelDC*) noexcept;
  /// initialise a ravel from a loaded datacube. @return true on success
  int ravelDC_initRavel(CAPIRavelDC* dc,CAPIRavel* ravel) noexcept;
  /// open a CSV file. Format is described by \a spec. -1 means figure it out from the data
  int ravelDC_openFile(CAPIRavelDC* dc, const char* fileName, CAPIRavelDataSpec spec) noexcept;
  /** return a hyperslice corresponding to the ravel's configuration
      The returned data is a dense multidimensional array with the
      dimensions returned as the \a dim parameter, which must be the
      ravel's rank in size.
      The returned data is owned by \a dc, and remains valid until the
      next call of hyperSlice
      @return true on success. On failure, retrieve error message with ravel_lastErr
  **/
  void ravelDC_hyperSlice(CAPIRavelDC* dc, void *ravel, size_t dims[], double **data) noexcept;
  /// return XML represention of
  const char* ravelDC_toXML(CAPIRavelDC*) noexcept;
  /// populate with XML data
  void ravelDC_fromXML(CAPIRavelDC*, const char*) noexcept;
}
#endif
