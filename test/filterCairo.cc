#include <dataCubeTCL.h>
#include <cairo_base.h>
#include <UnitTest++/UnitTest++.h>
#include <ecolab_epilogue.h>
#include <fstream>
using namespace std;
using namespace ravel;
using ecolab::cairo::Surface;

namespace
{
  void clear(Surface& s)
  {
    cairo_save(s.cairo());
    cairo_rectangle(s.cairo(),0,0,150,150);
    cairo_set_source_rgb(s.cairo(),1,1,1);
    cairo_fill(s.cairo());
    cairo_restore(s.cairo());
  }
}

SUITE(FilterCairo)
{
  /*
    Test rendering by outputting a PNG file.  This test is quite
    fragile, as cosmetic "look and feel" changes can break it.
   */
  TEST_FIXTURE(DataCubeTCL, render)
    {
      width=150, height=150;
      fs.reset(new Surface(cairo_image_surface_create(CAIRO_FORMAT_ARGB32,width,height)));
      setG(fs->cairo());

      // check for presence of input file
      ifstream inputFile("input.csv");
      CHECK(inputFile.good());

      nRowAxes=1;
      nColAxes=2;
      separator=',';
      
      loadFile("input.csv");
      array="data";
      DataCube::populateArray(ravel);
      FilterCairo<cairo_t*>::render();
      cairo_surface_write_to_png(fs->surface(),"defaultFCRender.png");

      // adjust filter limits
      onMouseDown(0,0);
      onMouse(0,50);
      onMouseUp(0,50);
      onMouseDown(0,150);
      onMouse(0,100);
      onMouseUp(0,100);
      fs.reset(new Surface(cairo_image_surface_create(CAIRO_FORMAT_ARGB32,width,height)));
      setG(fs->cairo());
      FilterCairo<cairo_t*>::render();
      cairo_surface_write_to_png(fs->surface(),"adjustedFCRender.png");
    }
}
