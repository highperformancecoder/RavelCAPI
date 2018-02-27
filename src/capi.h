/*
  Ravel C API. © Ravelation Pty Ltd 2018
*/

#ifndef CAPI_H
#define CAPI_H
#include <cairo/cairo.h>
#include <stdlib.h>

extern "C"
{
  /// create a new Ravel control widget
  void* ravel_new(size_t rank);
  /// dispose of ravel created by ravel_new()
  void ravel_delete(void* ravel);
  /// render ravel widget into a Cairo context
  void ravel_render(void* ravel, cairo_t* cairo);
  /// @{ handle mouse events
  void ravel_onMouseDown(void* ravel, double x, double y);
  void ravel_onMouseUp(void* ravel, double x, double y);
  void ravel_onMouseMotion(void* ravel, double x, double y);
  void ravel_onMouseLeave(void* ravel, double x, double y);
  /// @}
  /// resize a ravel
  void ravel_rescale(void* ravel, double radius);
  double ravel_radius(void* ravel);
  size_t ravel_rank(void* ravel);

  /// return the handle IDs of the output handles, in order x,y,z, etc.
  /// ids must be ravel_rank() in size
  void ravel_outputHandleIds(void* ravel, size_t ids[]);
  /// number of slice labels along axis \a axis
  size_t ravel_numSliceLabels(void* ravel, size_t axis);
  /** returns the sliceLabels along axis \a axis 
      \a labels must be at least ravel_numSliceLabels in size.  

      The returned pointers are owned by the ravel object, and remain
      valid until the next call to ravel_sliceLabels. Do not delete or
      free these.
  **/
  void ravel_sliceLabels(void* ravel, size_t axis, const char* labels[]);
  /// return XML represention of
  const char* ravel_toXML();
  /// populate with XML data
  void ravel_fromXML(const char*);

  /// create a new dataCube object
  void* ravelDC_new();
  /// delete a datacube object
  void ravelDC_delete(void*);
  /// open a flat CSV file format (last column is the data)
  void ravelDC_openFile(void* dc, const char* fileName);
  /** return a hyperslice corresponding to the ravel's configuration
      The returned data is a dense multidimensional array with the
      dimensions returned as the \a dim parameter, which must be the
      ravel's rank in size.
      The returned data is owned by \a dc, and remains valid until the
      next call of hyperSlice
  **/
  void ravelDC_hyperSlice(void* dc, void *ravel, size_t dims[], double **data);
  /// return XML represention of
  const char* ravelDC_toXML();
  /// populate with XML data
  void ravelDC_fromXML(const char*);
}
#endif
