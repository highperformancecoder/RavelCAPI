#include <ravelCairo.h>
#include <cairo_base.h>
#include <UnitTest++/UnitTest++.h>
#include <ecolab_epilogue.h>
using namespace ravel;
using ecolab::cairo::Surface;

typedef RavelCairo<cairo_t*> RCCairo;

namespace
{
  void clear(Surface& s)
  {
    cairo_rectangle(s.cairo(),-150,-150,300,300);
    cairo_set_source_rgb(s.cairo(),1,1,1);
    cairo_fill(s.cairo());
  }
}

SUITE(RavelCairo)
{
  /*
    Test rendering by outputting a PNG file.  This test is quite
    fragile, as cosmetic "look and feel" changes can break it.
   */
  TEST_FIXTURE(RCCairo, render)
    {
      Surface surf(cairo_image_surface_create(CAIRO_FORMAT_ARGB32,300,300));
      cairo_surface_set_device_offset(surf.surface(), 150,150);
      setG(surf.cairo());
      handleIds={0,1};
      addHandle("x",{"a","b","c"});
      addHandle("y",{"a","b","c"});
      addHandle("z",{"a","b","c"});
      rescale(100);
      render();
      cairo_surface_write_to_png(surf.surface(),"defaultRender.png");

      CHECK(onMouseOver(-50,-50));
      ::clear(surf);
      render(); // checks that rotation annotations are displayed
      cairo_surface_write_to_png(surf.surface(),"overZRender.png");
      CHECK_EQUAL(-1,handleIfMouseOverAxisLabel(100,20));
      CHECK_EQUAL(0,handleIfMouseOverAxisLabel(246-150,143-150));
      CHECK_EQUAL(1,handleIfMouseOverAxisLabel(145-150,245-150));
      CHECK_EQUAL(2,handleIfMouseOverAxisLabel(86-150,73-150));


      // move mouse away from a handle
      CHECK(onMouseOver(50,50)); 
      handles[0].displayFilterCaliper=true;
      ::clear(surf);
      render(); // checks that calipers are displayed
      cairo_surface_write_to_png(surf.surface(),"caliperRender.png");
      handles[0].displayFilterCaliper=false;
  
      // collapse a handle
      handles[2].toggleCollapsed();
      ::clear(surf);
      render();
      cairo_surface_write_to_png(surf.surface(),"collapsedHandleRender.png");
      CHECK_EQUAL(2,handleIfMouseOverOpLabel(handles[2].opX()+x,handles[2].opY()+y));
      CHECK_EQUAL(-1,handleIfMouseOverOpLabel(0,0));
      handles[2].toggleCollapsed();
      

      // check sort order annotations
      handles[0].sliceLabels.order(HandleSort::forward);
      ::clear(surf);
      render();
      cairo_surface_write_to_png(surf.surface(),"forwardSortRender.png");
   
      handles[0].sliceLabels.order(HandleSort::reverse);
      ::clear(surf);
      render();
      cairo_surface_write_to_png(surf.surface(),"reverseSortRender.png");
   
    }
}
