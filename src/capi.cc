#include "capi.h"
#include "ravelCairo.h"
#include <ecolab_epilogue.h>
using namespace ravel;

#include <memory>
using namespace std;

extern "C"
{
  int ravel_version() {return RAVEL_VERSION;}

  void* ravel_new(size_t rank)
  {
    unique_ptr<RavelCairo<cairo_t*>> r(new RavelCairo<cairo_t*>);
    r->handleIds.clear();
    for (size_t i=0; i<rank; ++i)
      r->handleIds.push_back(i);
    // TODO - temporaily here to get basic functionality up
    r->addHandle();
    r->addHandle();
    return r.release();
  }

  void ravel_delete(void* ravel)
  {
    delete static_cast<RavelCairo<cairo_t*>*>(ravel);
  }

  void ravel_render(void* ravel, cairo_t* cairo)
  {
    if (ravel)
      {
        auto& r=*static_cast<RavelCairo<cairo_t*>*>(ravel);
        r.setG(cairo);
        r.render();
      }
  }
  

  void ravel_onMouseDown(void* ravel, double x, double y)
  {
    if (ravel)
      {
        auto& r=*static_cast<RavelCairo<cairo_t*>*>(ravel);
        r.onMouseDown(x,y);
      }
  }

  
  void ravel_onMouseUp(void* ravel, double x, double y)
  {
    if (ravel)
      {
        auto& r=*static_cast<RavelCairo<cairo_t*>*>(ravel);
        r.onMouseUp(x,y);
      }
  }

  bool ravel_onMouseMotion(void* ravel, double x, double y)
  {
    if (ravel)
      {
        auto& r=*static_cast<RavelCairo<cairo_t*>*>(ravel);
        return r.onMouseMotion(x,y);
      }
    return false;
  }

  bool ravel_onMouseOver(void* ravel, double x, double y)
  {
    if (ravel)
      {
        auto& r=*static_cast<RavelCairo<cairo_t*>*>(ravel);
        return r.onMouseOver(x,y);
      }
    return false;
  }

  void ravel_onMouseLeave(void* ravel)
  {
    if (ravel)
      {
        auto& r=*static_cast<RavelCairo<cairo_t*>*>(ravel);
        r.onMouseLeave();
      }
  }

  void ravel_rescale(void* ravel, double radius)
  {
    if (ravel)
      {
        auto& r=*static_cast<RavelCairo<cairo_t*>*>(ravel);
        r.rescale(radius);
      }
  }

}
