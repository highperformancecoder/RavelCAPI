#include "capi.h"
#include "ravelCairo.h"
#include "dataCube.h"
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

struct CAPIRavel: public RavelCairo<cairo_t*>
{
  ostringstream os;
};

struct CAPIRavelDC: public DataCube
{
  // not using this callback
  void setDataElement(size_t, size_t, double) override {}
};

extern "C"
{
  DLLEXPORT int ravel_version() {return RAVEL_VERSION;}

  DLLEXPORT CAPIRavel* ravel_new(size_t rank)
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

  DLLEXPORT void ravel_delete(CAPIRavel* ravel)
  {
    delete ravel;
  }

  DLLEXPORT void ravel_render(CAPIRavel* ravel, cairo_t* cairo)
  {
    if (ravel)
      {
        ravel->setG(cairo);
        ravel->render();
      }
  }
  

  DLLEXPORT void ravel_onMouseDown(CAPIRavel* ravel, double x, double y)
  {
    if (ravel)
      ravel->onMouseDown(x,y);
  }

  
  DLLEXPORT void ravel_onMouseUp(CAPIRavel* ravel, double x, double y)
  {
    if (ravel)
      ravel->onMouseUp(x,y);
  }

  DLLEXPORT bool ravel_onMouseMotion(CAPIRavel* ravel, double x, double y)
  {
    if (ravel)
      return ravel->onMouseMotion(x,y);
    return false;
  }

  DLLEXPORT bool ravel_onMouseOver(CAPIRavel* ravel, double x, double y)
  {
    if (ravel)
      return ravel->onMouseOver(x,y);
    return false;
  }

  DLLEXPORT void ravel_onMouseLeave(CAPIRavel* ravel)
  {
    if (ravel)
      ravel->onMouseLeave();
  }

  DLLEXPORT void ravel_rescale(CAPIRavel* ravel, double radius)
  {
    if (ravel)
      ravel->rescale(radius);
  }

  DLLEXPORT double ravel_radius(CAPIRavel* ravel)
  {
    if (ravel)
      return ravel->radius();
    return 0;
  }

  DLLEXPORT size_t ravel_rank(CAPIRavel* ravel)
  {
    if (ravel)
      return ravel->rank();
    return 0;
  }

  DLLEXPORT void ravel_outputHandleIds(CAPIRavel* ravel, size_t ids[])
  {
    if (ravel)
      for (size_t i=0; i<ravel->rank(); ++i)
        ids[i]=ravel->handleIds[i];
  }
  
  DLLEXPORT size_t ravel_numSliceLabels(CAPIRavel* ravel, size_t axis)
  {
    if (ravel)
      if (axis<ravel->handles.size())
        return ravel->handles[axis].sliceLabels.size();
    return 0;
  }

  DLLEXPORT void ravel_sliceLabels(CAPIRavel* ravel, size_t axis, const char* labels[])
  {
    if (ravel && axis<ravel->handles.size())
      {
        auto& h=ravel->handles[axis];
        for (size_t i=0; i<h.sliceLabels.size(); ++i)
          labels[i]=h.sliceLabels[i].c_str();
      }
  }

  DLLEXPORT const char* ravel_toXML(CAPIRavel* ravel)
  {
    if (ravel)
      {
        ostringstream os;
        xml_pack_t x(os);
        xml_pack(x,"",static_cast<Ravel&>(*ravel));
        os.swap(ravel->os); //stash the string result
        return ravel->os.str().c_str();
      }
    return "";
  }
  
  /// populate with XML data
  DLLEXPORT void ravel_fromXML(CAPIRavel* ravel, const char* data)
  {
    if (ravel)
      {
        istringstream is(data);
        xml_unpack_t x(is);
        xml_unpack(x,"",static_cast<Ravel&>(*ravel));
      }
  }

  CAPIRavelDC* ravelDC_new()
  {return new CAPIRavelDC;}
  void ravelDC_delete(CAPIRavelDC* dc)
  {delete dc;}
  
  void ravelDC_initRavel(CAPIRavelDC* dc,CAPIRavel* ravel)
  {
    if (dc && ravel)
      dc->initRavel(*ravel);
  }
  
  void ravelDC_openFile(CAPIRavelDC* dc, const char* fileName, CAPIRavelDataSpec spec)
  {
    if (dc)
      {
        DataSpec dSpec;
        if (spec.nRowAxes<0 || spec.nColAxes<0 || spec.nCommentLines < 0)
          {
            ifstream input(fileName);
            CSVFTokeniser tok(input, spec.separator);
            dSpec=dc->initDataSpec(tok);
          }
        if (spec.nRowAxes>=0) dSpec.nRowAxes=spec.nRowAxes;
        if (spec.nColAxes>=0) dSpec.nColAxes=spec.nColAxes;
        if (spec.nCommentLines>=0)
          {
            dSpec.commentRows.clear();
            for (unsigned i=0; i<spec.nCommentLines; ++i)
              dSpec.commentRows.insert(i);
          }
        
        ifstream input(fileName);
        CSVFTokeniser tok(input, spec.separator);
        dc->loadData(tok,dSpec);
      }
  }

}
