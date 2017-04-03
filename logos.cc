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
  rc.rescale(200);
  rc.addHandle("Year",{"1990","1991","1992"});
  rc.addHandle("Gender",{"Male","Female"});
  rc.addHandle("Country",{"Australia","UK","USA"});

  Surface surf(cairo_svg_surface_create("logo.svg",500,500));
  cairo_surface_set_device_offset(surf.surface(),250,250);
  rc.setG(surf.cairo());
  rc.render();
}
