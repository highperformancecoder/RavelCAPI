/*
  Ravel C API. © Ravelation Pty Ltd 2018
*/

#ifndef CAPI_H
#define CAPI_H
#include <cairo/cairo.h>
#include <stdlib.h>

#define RAVEL_VERSION 1

struct CAPIRavel;
struct CAPIRavelDC;

extern "C"
{
  /// returns the major version number of this API. This gets bumped
  /// whenever a method is removed or signature changes, but not when
  /// a method is added
  int ravel_version();
  
  /// create a new Ravel control widget
  CAPIRavel* ravel_new(size_t rank);
  /// dispose of ravel created by ravel_new()
  void ravel_delete(CAPIRavel* ravel);
  /// render ravel widget into a Cairo context
  void ravel_render(CAPIRavel* ravel, cairo_t* cairo);
  /// @{ handle mouse events
  void ravel_onMouseDown(CAPIRavel* ravel, double x, double y);
  void ravel_onMouseUp(CAPIRavel* ravel, double x, double y);
  /// handle mouse motion with button pressed
  /// @ return true if it needs to be rerendered
  bool ravel_onMouseMotion(CAPIRavel* ravel, double x, double y);
  /// hande mouse motion without pressed button (tooltips etc)
  /// @ return true if it needs to be rerendered
  bool ravel_onMouseOver(CAPIRavel* ravel, double x, double y);
  /// handle mouse movements leaving the ravel
  void ravel_onMouseLeave(CAPIRavel* ravel);
  /// @}
  /// resize a ravel
  void ravel_rescale(CAPIRavel* ravel, double radius);
  double ravel_radius(CAPIRavel* ravel);
  size_t ravel_rank(CAPIRavel* ravel);

  /// return the handle IDs of the output handles, in order x,y,z, etc.
  /// ids must be ravel_rank() in size
  void ravel_outputHandleIds(CAPIRavel* ravel, size_t ids[]);
  /// number of slice labels along axis \a axis
  size_t ravel_numSliceLabels(CAPIRavel* ravel, size_t axis);
  /** returns the sliceLabels along axis \a axis 
      \a labels must be at least ravel_numSliceLabels in size.  

      The returned pointers are owned by the ravel object, and remain
      valid until the next call to ravel_sliceLabels. Do not delete or
      free these.
  **/
  void ravel_sliceLabels(CAPIRavel* ravel, size_t axis, const char* labels[]);
  /// return XML represention of
  const char* ravel_toXML(CAPIRavel* ravel);
  /// populate with XML data
  void ravel_fromXML(CAPIRavel* ravel, const char*);

  /// create a new dataCube object
  CAPIRavelDC* ravelDC_new();
  /// delete a datacube object
  void ravelDC_delete(CAPIRavelDC*);
  /// open a flat CSV file format (last column is the data)
  void ravelDC_openFile(CAPIRavelDC* dc, const char* fileName);
  /** return a hyperslice corresponding to the ravel's configuration
      The returned data is a dense multidimensional array with the
      dimensions returned as the \a dim parameter, which must be the
      ravel's rank in size.
      The returned data is owned by \a dc, and remains valid until the
      next call of hyperSlice
  **/
  void ravelDC_hyperSlice(CAPIRavelDC* dc, void *ravel, size_t dims[], double **data);
  /// return XML represention of
  const char* ravelDC_toXML(CAPIRavelDC*);
  /// populate with XML data
  void ravelDC_fromXML(CAPIRavelDC*, const char*);
}
#endif
