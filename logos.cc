/* generate SVG logo */
#include <ravelCairo.h>
#include <cairo_base.h>
#include <ecolab_epilogue.h>
#include <cairo/cairo-svg.h>

using namespace ravel;
using namespace ecolab::cairo;

int main()
{
  RavelCairo<cairo_t*> rc;
  rc.rescale(50);
  rc.addHandle("Year",{"1990","1991","1992"});
  rc.addHandle("Gender",{"Male","Female"});
  rc.addHandle("Country",{"Australia","UK","USA"});

  Surface surf(cairo_image_surface_create(CAIRO_FORMAT_ARGB32,100,100));
  cairo_surface_set_device_offset(surf.surface(),50,50);
  rc.setG(surf.cairo());
  rc.render();
  cairo_surface_write_to_png(surf.surface(),"logo100.png");
}
