#include "capi.h"
#include "ravelCairo.h"
#include <ecolab_epilogue.h>
using namespace ravel;

#include <memory>
using namespace std;

using classdesc::xml_pack_t;
using classdesc::xml_unpack_t;
#undef DLLEXPORT
#ifdef WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

namespace
{
  struct CAPIRavel: public RavelCairo<cairo_t*>
  {
    ostringstream os;
  };
}

extern "C"
{
  DLLEXPORT int ravel_version() {return RAVEL_VERSION;}

  DLLEXPORT void* ravel_new(size_t rank)
  {
    unique_ptr<CAPIRavel> r(new CAPIRavel);
    r->handleIds.clear();
    for (size_t i=0; i<rank; ++i)
      r->handleIds.push_back(i);
    // TODO - temporaily here to get basic functionality up
    r->addHandle();
    r->addHandle();
    return r.release();
  }

  DLLEXPORT void ravel_delete(void* ravel)
  {
    delete static_cast<RavelCairo<cairo_t*>*>(ravel);
  }

  DLLEXPORT void ravel_render(void* ravel, cairo_t* cairo)
  {
    if (ravel)
      {
        auto& r=*static_cast<CAPIRavel*>(ravel);
        r.setG(cairo);
        r.render();
      }
  }
  

  DLLEXPORT void ravel_onMouseDown(void* ravel, double x, double y)
  {
    if (ravel)
      {
        auto& r=*static_cast<CAPIRavel*>(ravel);
        r.onMouseDown(x,y);
      }
  }

  
  DLLEXPORT void ravel_onMouseUp(void* ravel, double x, double y)
  {
    if (ravel)
      {
        auto& r=*static_cast<CAPIRavel*>(ravel);
        r.onMouseUp(x,y);
      }
  }

  DLLEXPORT bool ravel_onMouseMotion(void* ravel, double x, double y)
  {
    if (ravel)
      {
        auto& r=*static_cast<CAPIRavel*>(ravel);
        return r.onMouseMotion(x,y);
      }
    return false;
  }

  DLLEXPORT bool ravel_onMouseOver(void* ravel, double x, double y)
  {
    if (ravel)
      {
        auto& r=*static_cast<CAPIRavel*>(ravel);
        return r.onMouseOver(x,y);
      }
    return false;
  }

  DLLEXPORT void ravel_onMouseLeave(void* ravel)
  {
    if (ravel)
      {
        auto& r=*static_cast<CAPIRavel*>(ravel);
        r.onMouseLeave();
      }
  }

  DLLEXPORT void ravel_rescale(void* ravel, double radius)
  {
    if (ravel)
      {
        auto& r=*static_cast<CAPIRavel*>(ravel);
        r.rescale(radius);
      }
  }

  DLLEXPORT double ravel_radius(void* ravel)
  {
    if (ravel)
      {
        auto& r=*static_cast<CAPIRavel*>(ravel);
        return r.radius();
      }
    return 0;
  }

  DLLEXPORT size_t ravel_rank(void* ravel)
  {
    if (ravel)
      {
        auto& r=*static_cast<CAPIRavel*>(ravel);
        return r.rank();
      }
    return 0;
  }

  DLLEXPORT void ravel_outputHandleIds(void* ravel, size_t ids[])
  {
    if (ravel)
      {
        auto& r=*static_cast<CAPIRavel*>(ravel);
        for (size_t i=0; i<r.rank(); ++i)
          ids[i]=r.handleIds[i];
      }
  }
  
  DLLEXPORT size_t ravel_numSliceLabels(void* ravel, size_t axis)
  {
    if (ravel)
      {
        auto& r=*static_cast<CAPIRavel*>(ravel);
        if (axis<r.handles.size())
          return r.handles[axis].sliceLabels.size();
      }
    return 0;
  }

  DLLEXPORT void ravel_sliceLabels(void* ravel, size_t axis, const char* labels[])
  {
    if (ravel)
      {
        auto& r=*static_cast<CAPIRavel*>(ravel);
        if (axis<r.handles.size())
          {
            auto& h=r.handles[axis];
            for (size_t i=0; i<h.sliceLabels.size(); ++i)
              labels[i]=h.sliceLabels[i].c_str();
          }
      }
  }

  DLLEXPORT const char* ravel_toXML(void* ravel)
  {
    if (ravel)
      {
        auto& r=*static_cast<CAPIRavel*>(ravel);
        ostringstream os;
        xml_pack_t x(os);
        xml_pack(x,"",static_cast<Ravel&>(r));
        os.swap(r.os); //stash the string result
        return r.os.str().c_str();
      }
    return "";
  }
  
  /// populate with XML data
  DLLEXPORT void ravel_fromXML(void* ravel, const char* data)
  {
    if (ravel)
      {
        auto& r=*static_cast<CAPIRavel*>(ravel);
        istringstream is(data);
        xml_unpack_t x(is);
        xml_unpack(x,"",static_cast<Ravel&>(r));
      }
  }

}
