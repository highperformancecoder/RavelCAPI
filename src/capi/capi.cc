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
  string temp;
};

// canary failure if CAPIRenderer interface changes.
// If it does, RAVEL_CAPI_VERSION needs to be bumped, and this assert fixed
static_assert(sizeof(CAPIRenderer)==23*sizeof(void*),"Unexpected CAPIRenderer size - bump RAVEL_CAPI_VERSION");
static_assert(sizeof(CAPIRavelDataSpec)==4*sizeof(int),"Unexpected CAPIRavelDataSpec size - bump RAVEL_CAPI_VERSION");
#ifndef WIN32
static_assert(sizeof(CAPIHandleState)==56,"Unexpected CAPIHandleState size - bump RAVEL_CAPI_VERSION");
#endif

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

  DLLEXPORT void ravel_clear(CAPIRavel* ravel) noexcept 
  {
    if (ravel) ravel->clear();
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

  /// descriptive text of the operation of the Ravel (plain English for now)
  DLLEXPORT const char* ravel_description(CAPIRavel* ravel) noexcept
  {
    ravel->temp=ravel->description();
    return ravel->temp.c_str();
  }

  DLLEXPORT void ravel_setExplain(CAPIRavel* ravel, const char* explain, double x, double y) noexcept
  {
    if (ravel)
      ravel->setExplain(explain,x,y);
  }

  DLLEXPORT void ravel_resetExplain(CAPIRavel* ravel) noexcept
  {
    if (ravel)
      ravel->explain.clear();
  }

  DLLEXPORT const char* ravel_explain(CAPIRavel* ravel, double x, double y) noexcept
  {
    return ravel->Ravel::explain(x,y);
  }
  
  DLLEXPORT void ravel_outputHandleIds(CAPIRavel* ravel, size_t ids[]) noexcept 
  {
    if (ravel)
      for (size_t i=0; i<ravel->rank(); ++i)
        ids[i]=ravel->handleIds[i];
  }

  DLLEXPORT void ravel_setOutputHandleIds(CAPIRavel* ravel, size_t rank, size_t ids[]) noexcept 
  {
    if (ravel)
      {
        ravel->handleIds.clear();
        for (auto i=0; i<rank; ++i)
          ravel->handleIds.push_back(ids[i]);
        ravel_redistributeHandles(ravel);
      }
  }

   
  
  DLLEXPORT unsigned ravel_numHandles(CAPIRavel* ravel) noexcept
  {return ravel? ravel->handles.size(): 0;}

  DLLEXPORT  int ravel_selectedHandle(CAPIRavel* ravel) noexcept
  {return ravel? ravel->selectedHandleId(): -1;}
  
  DLLEXPORT const char* ravel_handleDescription(CAPIRavel* ravel, int handle) noexcept
  {
    if (ravel && handle>=0 && handle<ravel->handles.size())
      return ravel->handles[handle].description.c_str();
    return "";
  }
    
  DLLEXPORT void ravel_setHandleDescription
  (CAPIRavel* ravel, int handle, const char* description) noexcept
  {
    if (ravel && handle>=0 && handle<ravel->handles.size())
      ravel->handles[handle].description=description;
  }
    
  DLLEXPORT size_t ravel_numSliceLabels(CAPIRavel* ravel, size_t axis) noexcept 
  {
    if (ravel)
      if (axis<ravel->handles.size())
        {
          auto& h=ravel->handles[axis];
          return h.collapsed()? 1: h.sliceLabels.size();
        }
    return 0;
  }

  DLLEXPORT void ravel_sliceLabels(CAPIRavel* ravel, size_t axis, const char* labels[]) noexcept 
  {
    if (ravel && axis<ravel->handles.size())
      {
        auto& h=ravel->handles[axis];
        if (h.collapsed())
          labels[0]=opLabels[h.reductionOp];
        else
          for (size_t i=0; i<h.sliceLabels.size(); ++i)
            labels[i]=h.sliceLabels[i].c_str();
      }
  }

  DLLEXPORT void ravel_displayFilterCaliper(CAPIRavel* ravel, size_t axis, bool display) noexcept 
  {
    if (ravel && axis<ravel->handles.size())
      ravel->handles[axis].displayFilterCaliper(display);
  }

  DLLEXPORT void ravel_setSlicer(CAPIRavel* ravel, size_t axis,
                                   const char* sliceLabel) noexcept
  {
    if (ravel && axis<ravel->handles.size())
      {
        auto& h=ravel->handles[axis];
        h.setSlicer(sliceLabel);
      }
  }
  
  DLLEXPORT void ravel_setCalipers(CAPIRavel* ravel, size_t axis,
                                   const char* l1, const char* l2) noexcept
  {
    if (ravel && axis<ravel->handles.size())
      ravel->handles[axis].sliceLabels.setCalipers(l1,l2);
  }
  
  DLLEXPORT void ravel_orderLabels(CAPIRavel* ravel, size_t axis, CAPIHandleState::HandleSort order, CAPIHandleState::HandleSortType type, const char* format) noexcept 
  {
    HandleSort::Order o;
    switch (order)
      {
      case CAPIHandleState::forward: o=HandleSort::forward; break;
      case CAPIHandleState::reverse: o=HandleSort::reverse; break;
      default: o=HandleSort::none; break;
      }

    if (ravel && axis<ravel->handles.size())
      ravel->handles[axis].sliceLabels.order(o, HandleSort::OrderType(type), format);
  }

  DLLEXPORT void ravel_applyCustomPermutation
  (CAPIRavel* ravel, size_t axis, size_t numIndices, const size_t* indices)
    noexcept
  {
    if (ravel && axis<ravel->handles.size())
      {
        vector<size_t> perm(indices,indices+numIndices);
        auto& h=ravel->handles[axis];
        h.sliceLabels.customPermutation(perm);
        if (h.sliceIndex>h.sliceMax())
          h.sliceIndex=h.sliceMax();
        else if (h.sliceIndex<h.sliceMin())
          h.sliceIndex=h.sliceMin();
      }
  }
  
  DLLEXPORT void ravel_currentPermutation
  (CAPIRavel* ravel, size_t axis, size_t numIndices, size_t* indices) noexcept
  {
    if (ravel && axis<ravel->handles.size())
      {
        auto& idx=ravel->handles[axis].sliceLabels.currentPermutation();
        for (size_t i=0; i<idx.size() && i<numIndices; ++i)
          indices[i]=idx[i];
      }
  }

  
  DLLEXPORT void ravel_addHandle(CAPIRavel* ravel, const char* description, size_t numSliceLabels, const char* sliceLabels[]) noexcept
  {
    if (ravel)
        ravel->addHandle
          (description, vector<string>(sliceLabels,sliceLabels+numSliceLabels));
  }

  
  DLLEXPORT const char* ravel_toXML(CAPIRavel* ravel) noexcept 
  {
    if (ravel)
      try
        {
          ostringstream os;
          xml_pack_t x(os);
          xml_pack(x,"ravel",static_cast<Ravel&>(*ravel));
          ravel->temp=os.str();
          return ravel->temp.c_str();
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

  DLLEXPORT void ravel_getHandleState(const CAPIRavel* ravel, size_t handle,
                            CAPIHandleState* hs) noexcept
  {
    if (ravel && handle<ravel->handles.size())
      {
        const Handle& h=ravel->handles[handle];
        hs->x=h.x();
        hs->y=h.y();
        hs->sliceIndex=h.sliceIndex;
        hs->sliceMin=h.sliceLabels.m_sliceMin;
        hs->sliceMax=h.sliceLabels.m_sliceMax;
        hs->collapsed=h.collapsed();
        hs->displayFilterCaliper=h.displayFilterCaliper();
        hs->reductionOp=CAPIHandleState::ReductionOp(h.reductionOp);
        switch (h.sliceLabels.order())
          {
          case HandleSort::none:
            hs->order=CAPIHandleState::none;
            break;
          case HandleSort::forward: case HandleSort::numForward: case HandleSort::timeForward:
            hs->order=CAPIHandleState::forward;
            break;
          case HandleSort::reverse: case HandleSort::numReverse: case HandleSort::timeReverse:
            hs->order=CAPIHandleState::reverse;
            break;
          case HandleSort::custom:
            hs->order=CAPIHandleState::custom;
            break;
          }
      }
  }
    
    
  /// set the handle state
  DLLEXPORT void ravel_setHandleState(CAPIRavel* ravel, size_t handle,
                            const CAPIHandleState* hs) noexcept
  {
    if (ravel && handle<ravel->handles.size())
      {
        Handle& h=ravel->handles[handle];
        h.moveTo(hs->x,hs->y,false);
        h.sliceLabels.order(HandleSort::Order(hs->order));
        h.sliceIndex=hs->sliceIndex<h.sliceLabels.size()? hs->sliceIndex: 0;
        h.sliceLabels.min(hs->sliceMin);
        h.sliceLabels.max(hs->sliceMax);
        if (hs->collapsed!=h.collapsed())
          h.toggleCollapsed();
        h.displayFilterCaliper(hs->displayFilterCaliper);
        h.reductionOp=Op::ReductionOp(hs->reductionOp);
      }
  }
  
  DLLEXPORT void ravel_adjustSlicer(CAPIRavel* ravel, int n) noexcept
  {
    if (ravel)
      if (auto h=ravel->selectedHandle())
        {
          auto i=find(ravel->handleIds.begin(), ravel->handleIds.end(), h-&ravel->handles[0]);
          if (i==ravel->handleIds.end())
            {
              if (n<-int(h->sliceIndex)) // off end, stop slicing
                ravel->handleIds.push_back(h-&ravel->handles[0]);
              h->moveSliceIdx(n);
            }
          else if (n>0) // start slicing
            {
              h->sliceIndex=0;
              h->moveSliceIdx(n);
              ravel->handleIds.erase(i);
            }
        }
  }

  DLLEXPORT void ravel_redistributeHandles(CAPIRavel* ravel) noexcept
  {
    if (ravel)
      try
        {ravel->redistributeHandles();}
      CONSUME_EXCEPTION();
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

  DLLEXPORT void ravelDC_loadData(CAPIRavelDC* dc, const CAPIRavel* ravel, const double data[]) noexcept
  {
    if (dc && ravel)
      {
        LabelsVector lv;
        for (auto& h: ravel->handles)
          {
            LabelsVector::value_type l;
            l.first=h.description;
            l.second.assign(h.sliceLabels.begin(), h.sliceLabels.end());
            lv.push_back(l);
          }
        dc->loadData(RawDataIdx(lv),data);
      }
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
            if (dc->slice.size()>0)
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