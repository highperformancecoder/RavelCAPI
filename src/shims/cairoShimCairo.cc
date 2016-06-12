#include "cairoShim.h"
#define CAIRO_WIN32_STATIC_BUILD
#include <cairo.h>
#undef CAIRO_WIN32_STATIC_BUILD

using namespace std;

namespace ravel
{
  template <> class CairoShimImpl<cairo_t*>
  {
  public:
    cairo_t* cairo;
    cairo_text_extents_t extents;
    CairoShimImpl(cairo_t* cairo): cairo(cairo) {}
  };

  template <> CairoShim<cairo_t*>::CairoShim(cairo_t* cairo): 
    impl(new CairoShimImpl<cairo_t*>(cairo)) {}
  template <> CairoShim<cairo_t*>::~CairoShim()
  {delete impl;}

  // template parameter G = cairo_t* or HDC
  template <> void CairoShim<cairo_t*>::moveTo(double x, double y) 
  {cairo_move_to(impl->cairo,x,y);}

  template <> void CairoShim<cairo_t*>::lineTo(double x, double y)
  {cairo_line_to(impl->cairo,x,y);}

  template <> void CairoShim<cairo_t*>::relMoveTo(double x, double y)
  {cairo_rel_move_to(impl->cairo,x,y);}

  template <> void CairoShim<cairo_t*>::relLineTo(double x, double y)
  {cairo_rel_line_to(impl->cairo,x,y);}

  template <> void CairoShim<cairo_t*>::arc
  (double x, double y, double radius, double start, double end)
  {cairo_arc(impl->cairo,x,y,radius,start,end);}

    // paths
  template <> void CairoShim<cairo_t*>::newPath()
  {cairo_new_path(impl->cairo);}

  template <> void CairoShim<cairo_t*>::closePath()
  {cairo_close_path(impl->cairo);}

  template <> void CairoShim<cairo_t*>::fill()
  {cairo_fill(impl->cairo);}

  template <> void CairoShim<cairo_t*>::stroke()
  {cairo_stroke(impl->cairo);}
  template <> void CairoShim<cairo_t*>::strokePreserve()
  {cairo_stroke_preserve(impl->cairo);}

  template <> void CairoShim<cairo_t*>::setLineWidth(double w)
  {cairo_set_line_width(impl->cairo, w);}

  // sources
  template <> void CairoShim<cairo_t*>::setSourceRGB
  (double r, double g, double b)
  {cairo_set_source_rgb(impl->cairo,r,g,b);}

  // text. Argument is in UTF8 encoding
  template <> void CairoShim<cairo_t*>::showText(const std::string& text)
  {cairo_show_text(impl->cairo,text.c_str());}

  template <> void CairoShim<cairo_t*>::setTextExtents(const std::string& text)
  {cairo_text_extents(impl->cairo,text.c_str(),&impl->extents);}

  template <> double CairoShim<cairo_t*>::textWidth() const
  {return impl->extents.width;}

  template <> double CairoShim<cairo_t*>::textHeight() const
  {return impl->extents.height;}

  // matrix transformation
  template <> void CairoShim<cairo_t*>::identityMatrix()
  {cairo_identity_matrix(impl->cairo);}
  
  template <> void CairoShim<cairo_t*>::translate(double x, double y)
  {cairo_translate(impl->cairo,x,y);}

  template <> void CairoShim<cairo_t*>::scale(double sx, double sy)
  {cairo_scale(impl->cairo,sx,sy);}

  template <> void CairoShim<cairo_t*>::rotate(double angle)
  {cairo_rotate(impl->cairo,angle);}

    // context manipulation
  template <> void CairoShim<cairo_t*>::save()
  {cairo_save(impl->cairo);}

  template <> void CairoShim<cairo_t*>::restore()
  {cairo_restore(impl->cairo);}


}
