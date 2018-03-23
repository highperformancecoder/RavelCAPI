#include "capi.h"
#include "ravelCairo.h"
#include "dataCube.h"
#include "ravelVersion.h"
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

struct CAPIRavel: public RavelCairo<CAPIRenderer*>
{
  string xml;
};

// canary failure if CAPIRenderer interface changes.
// If it does, RAVEL_CAPI_VERSION needs to be bumped, and this assert fixed
static_assert(sizeof(CAPIRenderer)==22*sizeof(void*),"Unexpected CAPIRenderer size - bump RAVEL_CAPI_VERSION");

struct CAPIRavelDC: public DataCube
{
  // not using this callback
  void setDataElement(size_t, size_t, double) override {}
  RawData slice;
};

static string lastErr;

#define CONSUME_EXCEPTION(ret)                     \
  catch (const std::exception& ex)                 \
    {                                              \
      try {lastErr=ex.what();}                     \
      catch(...) {}                                \
      return ret;                                  \
    }                                              \
  catch (...)                                      \
    {                                              \
      try {lastErr="unknown exception caught";}    \
      catch(...) {}                                \
      return ret;                                  \
    }


extern "C"
{
  DLLEXPORT int ravel_capi_version() noexcept {return RAVEL_CAPI_VERSION;}
  DLLEXPORT const char* ravel_lastErr() noexcept  {return lastErr.c_str();}
  DLLEXPORT const char* ravel_version() noexcept  {return RAVEL_VERSION;}

  DLLEXPORT CAPIRavel* ravel_new(size_t rank) noexcept 
  {
    try
      {
        unique_ptr<CAPIRavel> r(new CAPIRavel);
        r->handleIds.clear();
        for (size_t i=0; i<rank; ++i)
          r->handleIds.push_back(i);
        return r.release();
      }
    CONSUME_EXCEPTION(nullptr);
  }

  DLLEXPORT void ravel_delete(CAPIRavel* ravel) noexcept 
  {
    delete ravel;
  }

  DLLEXPORT void ravel_render(CAPIRavel* ravel, CAPIRenderer* cairo) noexcept 
  {
    if (ravel)
      {
        ravel->setG(cairo);
        ravel->render();
      }
  }
  

  DLLEXPORT void ravel_onMouseDown(CAPIRavel* ravel, double x, double y) noexcept 
  {
    if (ravel)
      ravel->onMouseDown(x,y);
  }

  
  DLLEXPORT void ravel_onMouseUp(CAPIRavel* ravel, double x, double y) noexcept 
  {
    if (ravel)
      ravel->onMouseUp(x,y);
  }

  DLLEXPORT int ravel_onMouseMotion(CAPIRavel* ravel, double x, double y) noexcept 
  {
    if (ravel)
      return ravel->onMouseMotion(x,y);
    return false;
  }

  DLLEXPORT int ravel_onMouseOver(CAPIRavel* ravel, double x, double y) noexcept 
  {
    if (ravel)
      return ravel->onMouseOver(x,y);
    return false;
  }

  DLLEXPORT void ravel_onMouseLeave(CAPIRavel* ravel) noexcept 
  {
    if (ravel)
      ravel->onMouseLeave();
  }

  DLLEXPORT void ravel_rescale(CAPIRavel* ravel, double radius) noexcept 
  {
    if (ravel)
      ravel->rescale(radius);
  }

  DLLEXPORT double ravel_radius(CAPIRavel* ravel) noexcept 
  {
    if (ravel)
      return ravel->radius();
    return 0;
  }

  DLLEXPORT size_t ravel_rank(CAPIRavel* ravel) noexcept 
  {
    if (ravel)
      return ravel->rank();
    return 0;
  }

  DLLEXPORT void ravel_outputHandleIds(CAPIRavel* ravel, size_t ids[]) noexcept 
  {
    if (ravel)
      for (size_t i=0; i<ravel->rank(); ++i)
        ids[i]=ravel->handleIds[i];
  }
  
  DLLEXPORT size_t ravel_numSliceLabels(CAPIRavel* ravel, size_t axis) noexcept 
  {
    if (ravel)
      if (axis<ravel->handles.size())
        return ravel->handles[axis].sliceLabels.size();
    return 0;
  }

  DLLEXPORT void ravel_sliceLabels(CAPIRavel* ravel, size_t axis, const char* labels[]) noexcept 
  {
    if (ravel && axis<ravel->handles.size())
      {
        auto& h=ravel->handles[axis];
        for (size_t i=0; i<h.sliceLabels.size(); ++i)
          labels[i]=h.sliceLabels[i].c_str();
      }
  }

  DLLEXPORT void ravel_displayFilterCaliper(CAPIRavel* ravel, size_t axis, bool display) noexcept 
  {
    if (ravel && axis<ravel->handles.size())
      ravel->handles[axis].displayFilterCaliper=display;
  }

  
  
  DLLEXPORT const char* ravel_toXML(CAPIRavel* ravel) noexcept 
  {
    if (ravel)
      try
        {
          ostringstream os;
          xml_pack_t x(os);
          xml_pack(x,"ravel",static_cast<Ravel&>(*ravel));
          ravel->xml=os.str();
          return ravel->xml.c_str();
        }
    CONSUME_EXCEPTION("");
    return "";
  }
  
  /// populate with XML data
  DLLEXPORT int ravel_fromXML(CAPIRavel* ravel, const char* data) noexcept 
  {
    if (ravel)
      try
      {
        istringstream is(data);
        xml_unpack_t x(is);
        xml_unpack(x,"ravel",static_cast<Ravel&>(*ravel));
      }
    CONSUME_EXCEPTION(false);
    return true;
  }

  DLLEXPORT CAPIRavelDC* ravelDC_new() noexcept 
  {
    try
      {
        return new CAPIRavelDC;
      }
    CONSUME_EXCEPTION(nullptr);
  }
  
  DLLEXPORT void ravelDC_delete(CAPIRavelDC* dc) noexcept 
  {delete dc;}
  
  DLLEXPORT int ravelDC_initRavel(CAPIRavelDC* dc,CAPIRavel* ravel) noexcept 
  {
    if (dc && ravel)
      try
        {
          dc->initRavel(*ravel);
        }
    CONSUME_EXCEPTION(false);
    return true;
  }
  
  DLLEXPORT int ravelDC_openFile(CAPIRavelDC* dc, const char* fileName, CAPIRavelDataSpec spec) noexcept 
  {
    if (dc)
      try
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
    CONSUME_EXCEPTION(false);
    return true;
  }

  DLLEXPORT int ravelDC_hyperSlice
  (CAPIRavelDC* dc, CAPIRavel *ravel, size_t dims[], double **data) noexcept
  {
    *data=nullptr;
    try
      {
        dc->hyperSlice(dc->slice,*ravel);
        if (ravel->rank()==dc->slice.rank())
          {
            *data=&dc->slice[0];
            for (size_t i=0; i<dc->slice.rank(); ++i)
              dims[i]=dc->slice.dim(i);
            return true;
          }
      }
    CONSUME_EXCEPTION(false);
    return false;
  }

}
