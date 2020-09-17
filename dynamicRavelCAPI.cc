/*
  Ravel C API. © Ravelation Pty Ltd 2020
  Open source licensed under the MIT license.
*/

#include "dynamicRavelCAPI.h"
#include "ravelCAPITypes.h"
#ifdef WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include <stdexcept>
using namespace std;

/// C API version this is compiled against
static const int ravelVersion=4;

namespace
{
#ifdef WIN32
  typedef HINSTANCE libHandle;
  libHandle loadLibrary(const string& lib)
  {return LoadLibraryA((lib+".dll").c_str());}

  FARPROC WINAPI dlsym(HMODULE lib, const char* name)
  {return GetProcAddress(lib,name);}

  void dlclose(HINSTANCE) {}

  const string dlerror() {
    char msg[1024];
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM,nullptr,GetLastError(),0,msg,sizeof(msg),nullptr);
    return msg;
  }
#else
  typedef void* libHandle;
  libHandle loadLibrary(const string& lib)
  {return dlopen((lib+".so").c_str(),RTLD_NOW);}
#endif
  
  struct RavelLib
  {
    libHandle lib;
    string errorMsg;
    string versionFound="unavailable";
    RavelLib(): lib(loadLibrary("libravel"))
    {
      if (!lib)
        {
          errorMsg=dlerror();
          return;
        }
      
      auto version=(const char* (*)())dlsym(lib,"ravel_version");
      auto capi_version=(int (*)())dlsym(lib,"ravel_capi_version");
      if (version) versionFound=version();
      if (!version || !capi_version || ravelVersion!=capi_version())
        { // incompatible API
          errorMsg="Incompatible libravel dynamic library found";
          dlclose(lib);
          lib=nullptr;
        }
    }
    ~RavelLib() {
      if (lib)
        dlclose(lib);
      lib=nullptr;
    }
    template <class F>
    void asgFnPointer(F& f, const char* name)
    {
      if (lib)
        {
          f=(F)dlsym(lib,name);
          if (!f)
            {
              errorMsg=dlerror();
              errorMsg+="\nLooking for ";
              errorMsg+=name;
              errorMsg+="\nProbably libravel dynamic library is too old";
              dlclose(lib);
              lib=nullptr;
            }
        }
    }
  };

  RavelLib ravelLib;

  template <class... T> struct RavelFn;
    
  template <class R>
  struct RavelFn<R>
  {
    R (*f)()=nullptr;
    RavelFn(const char*name, libHandle lib) {ravelLib.asgFnPointer(f,name);}
    R operator()() {return f? f(): R();}
  };
  template <>
  struct RavelFn<void>
  {
    void (*f)()=nullptr;
    RavelFn(const char*name, libHandle lib) {ravelLib.asgFnPointer(f,name);}
    void operator()() {if (f) f();}
  };
  template <class R, class A>
  struct RavelFn<R,A>
  {
    R (*f)(A)=nullptr;
    RavelFn(const char*name, libHandle lib) {ravelLib.asgFnPointer(f,name);}
    R operator()(A a) {return f? f(a): R{};}
  };
  template <class A>
  struct RavelFn<void,A>
  {
    void (*f)(A)=nullptr;
    RavelFn(const char*name, libHandle lib) {ravelLib.asgFnPointer(f,name);}
    void operator()(A a) {if (f) f(a);}
  };
  template <class R, class A0, class A1>
  struct RavelFn<R,A0,A1>
  {
    R (*f)(A0,A1)=nullptr;
    RavelFn(const char*name, libHandle lib) {ravelLib.asgFnPointer(f,name);}
    R operator()(A0 a0, A1 a1) {return f? f(a0,a1): R{};}
  };
  template <class A0, class A1>
  struct RavelFn<void,A0,A1>
  {
    void (*f)(A0,A1)=nullptr;
    RavelFn(const char*name, libHandle lib) {ravelLib.asgFnPointer(f,name);}
    void operator()(A0 a0, A1 a1) {if (f) f(a0,a1);}
  };
  template <class R, class A0, class A1, class A2>
  struct RavelFn<R,A0,A1,A2>
  {
    R (*f)(A0,A1,A2)=nullptr;
    RavelFn(const char*name, libHandle lib) {ravelLib.asgFnPointer(f,name);}
    R operator()(A0 a0, A1 a1,A2 a2) {return f? f(a0,a1,a2): R{};}
  };
  template <class A0, class A1, class A2>
  struct RavelFn<void,A0,A1,A2>
  {
    void (*f)(A0,A1,A2)=nullptr;
    RavelFn(const char*name, libHandle lib) {ravelLib.asgFnPointer(f,name);}
    void operator()(A0 a0, A1 a1, A2 a2) {if (f) f(a0,a1,a2);}
  };
  template <class R, class A0, class A1, class A2, class A3>
  struct RavelFn<R,A0,A1,A2,A3>
  {
    R (*f)(A0,A1,A2,A3)=nullptr;
    RavelFn(const char*name, libHandle lib) {ravelLib.asgFnPointer(f,name);}
    R operator()(A0 a0, A1 a1,A2 a2,A3 a3) {return f? f(a0,a1,a2,a3): R{};}
  };
  template <class A0, class A1, class A2,class A3>
  struct RavelFn<void,A0,A1,A2,A3>
  {
    void (*f)(A0,A1,A2,A3)=nullptr;
    RavelFn(const char*name, libHandle lib) {ravelLib.asgFnPointer(f,name);}
    void operator()(A0 a0, A1 a1, A2 a2,A3 a3) {if (f) f(a0,a1,a2,a3);}
  };
  template <class A0, class A1, class A2,class A3,class A4>
  struct RavelFn<void,A0,A1,A2,A3,A4>
  {
    void (*f)(A0,A1,A2,A3,A4)=nullptr;
    RavelFn(const char*name, libHandle lib) {ravelLib.asgFnPointer(f,name);}
    void operator()(A0 a0, A1 a1, A2 a2,A3 a3,A4 a4) {if (f) f(a0,a1,a2,a3,a4);}
  };
   
#define DEFFN(f,...) RavelFn<__VA_ARGS__> f(#f,ravelLib.lib);
    
  DEFFN(ravel_lastErr, const char*);
  DEFFN(ravel_version, const char*);
  DEFFN(ravel_new, CAPIRavel*,size_t);
  DEFFN(ravel_delete, void, CAPIRavel*);
  DEFFN(ravel_clear, void, CAPIRavel*);
  DEFFN(ravel_render, void, CAPIRavel*, CAPIRenderer*);
  DEFFN(ravel_onMouseDown, void, CAPIRavel*, double, double);
  DEFFN(ravel_onMouseUp,void, CAPIRavel*, double, double);
  DEFFN(ravel_onMouseMotion, bool, CAPIRavel*, double, double);
  DEFFN(ravel_onMouseOver, bool, CAPIRavel*, double, double);
  DEFFN(ravel_onMouseLeave, void, CAPIRavel*);
  DEFFN(ravel_rescale, void, CAPIRavel*, double);
  DEFFN(ravel_radius, double, CAPIRavel*);
  DEFFN(ravel_rank, size_t, CAPIRavel*);
  DEFFN(ravel_description, const char*, CAPIRavel*);
  DEFFN(ravel_explain, const char*, CAPIRavel*, double, double);
  DEFFN(ravel_setExplain, void, CAPIRavel*, const char*, double, double);
  DEFFN(ravel_resetExplain, void, CAPIRavel*);
  DEFFN(ravel_outputHandleIds, void, CAPIRavel*, size_t*);
  DEFFN(ravel_setOutputHandleIds, void, CAPIRavel*, size_t, const size_t*);
  DEFFN(ravel_addHandle, void, CAPIRavel*, const char*, size_t, const char**);
  DEFFN(ravel_numHandles, unsigned, CAPIRavel*);
  DEFFN(ravel_selectedHandle, int, CAPIRavel*);
  DEFFN(ravel_handleDescription, const char*, CAPIRavel*, int);
  DEFFN(ravel_setHandleDescription, void, CAPIRavel*, int, const char*);
  DEFFN(ravel_numSliceLabels, size_t, CAPIRavel*, size_t);
  DEFFN(ravel_numAllSliceLabels, size_t, CAPIRavel*, size_t);
  DEFFN(ravel_sliceLabels, void, CAPIRavel*, size_t, const char**);
  DEFFN(ravel_allSliceLabels, void, CAPIRavel*, size_t, RavelOrder, const char**);
  DEFFN(ravel_displayFilterCaliper, void, CAPIRavel*, size_t, bool);
  DEFFN(ravel_setSlicer,void,CAPIRavel*,size_t,const char*);
  DEFFN(ravel_setCalipers,void,CAPIRavel*,size_t,const char*,const char*);
  DEFFN(ravel_orderLabels, void, CAPIRavel*, size_t,RavelOrder,RavelOrderType, const char*);
  DEFFN(ravel_applyCustomPermutation, void, CAPIRavel*, size_t, size_t, const size_t*);
  DEFFN(ravel_currentPermutation, void, CAPIRavel*, size_t, size_t, size_t*);
  DEFFN(ravel_toXML, const char*, CAPIRavel*);
  DEFFN(ravel_fromXML, int, CAPIRavel*, const char*);
  DEFFN(ravel_getHandleState, CAPIRavelHandleState*, const CAPIRavel*, size_t);
  DEFFN(ravel_setHandleState, void, CAPIRavel*, size_t, const CAPIRavelHandleState*);
  DEFFN(ravel_getRavelState, CAPIRavelState*, const CAPIRavel*);
  DEFFN(ravel_setRavelState, void, CAPIRavel*, const CAPIRavelState*);
  DEFFN(ravel_adjustSlicer, void, CAPIRavel*, int);
  DEFFN(ravel_redistributeHandles,void, CAPIRavel*);
    
  //    DEFFN(ravelDC_new, Ravel::CAPIRavelDC*);
  //    DEFFN(ravelDC_delete, void, CAPIRavelDC*);
  //    DEFFN(ravelDC_initRavel, bool, CAPIRavelDC*, CAPIRavel*);
  //    DEFFN(ravelDC_openFile, bool, CAPIRavelDC*, const char*, RavelDataSpec);
  //    DEFFN(ravelDC_loadData, void, CAPIRavelDC*, const CAPIRavel*, const double*);
  //    DEFFN(ravelDC_hyperSlice, int, CAPIRavelDC*, CAPIRavel*, size_t*, double**);

}

namespace ravel
{

  Ravel::Ravel()
  {
    if (ravelLib.lib)
      {
        if (!(ravel=ravel_new(0)))
          throw std::runtime_error(ravel_lastErr());
      }
    else
      throw std::runtime_error(ravelLib.errorMsg);
  }
  
  Ravel::~Ravel() {
    if (ravel) ravel_delete(ravel);
  }

  std::string Ravel::version() {return ravelLib.versionFound;}
  void Ravel::clear() {ravel_clear(ravel);}
  void Ravel::render(CAPIRenderer& renderer) const {ravel_render(ravel, &renderer);}
  void Ravel::onMouseDown(double x, double y) {ravel_onMouseDown(ravel,x,y);}
  void Ravel::onMouseUp(double x,double y) {ravel_onMouseUp(ravel,x,y);}
  bool Ravel::onMouseMotion(double x,double y) {return ravel_onMouseMotion(ravel,x,y);}
  bool Ravel::onMouseOver(double x,double y) {return ravel_onMouseOver(ravel,x,y);}
  void Ravel::onMouseLeave() {return ravel_onMouseLeave(ravel);}
  void Ravel::rescale(double radius) {ravel_rescale(ravel,radius);}
  double Ravel::radius() const {
      return ravel? ravel_radius(ravel): ravelDefaultRadius;
  }
  size_t Ravel::rank() const {return ravel_rank(ravel);}
  std::string Ravel::description() const {return ravel_description(ravel);}
  void Ravel::setExplain(const std::string& explain, double x, double y)
  {ravel_setExplain(ravel,explain.c_str(), x,y);}
  void Ravel::resetExplain() {ravel_resetExplain(ravel);}
  std::string Ravel::explain(double x, double y) {return ravel_explain(ravel,x,y);}
  
  std::vector<size_t> Ravel::outputHandleIds() const {
    std::vector<size_t> ids(rank());
    ravel_outputHandleIds(ravel, ids.data());
    return ids;
  }
  void Ravel::setOutputHandleIds(const std::vector<size_t>& ids) {
    ravel_setOutputHandleIds(ravel, ids.size(), ids.data());
  }
  unsigned Ravel::numHandles() const {return ravel_numHandles(ravel);}
  int Ravel::selectedHandle() const {return ravel_selectedHandle(ravel);}
  std::string Ravel::handleDescription(int handle) const {
    return ravel_handleDescription(ravel,handle);
  }
  void Ravel::setHandleDescription(int handle, const std::string& description) {
    ravel_setHandleDescription(ravel, handle, description.c_str());
  }
  size_t Ravel::numSliceLabels(size_t axis) const {return ravel_numSliceLabels(ravel, axis);}
  std::vector<std::string> Ravel::sliceLabels(size_t axis) const {
    std::vector<const char*> tmp(numSliceLabels(axis));
    ravel_sliceLabels(ravel,axis,tmp.data());
    return std::vector<std::string>(tmp.begin(), tmp.end());
  }
  size_t Ravel::numAllSliceLabels(size_t axis) const {return ravel_numAllSliceLabels(ravel, axis);}
  std::vector<std::string> Ravel::allSliceLabels(size_t axis, HandleSort::Order order) const {
    std::vector<const char*> tmp(numAllSliceLabels(axis));
    ravel_allSliceLabels(ravel,axis,toEnum<RavelOrder>(order),tmp.data());
    return std::vector<std::string>(tmp.begin(), tmp.end());
  }
  void Ravel::displayFilterCaliper(size_t axis, bool display)
  {ravel_displayFilterCaliper(ravel,axis,display);}
  void Ravel::setSlicer(size_t axis, const std::string& sliceLabel)
  {ravel_setSlicer(ravel,axis,sliceLabel.c_str());}
  void Ravel::setCalipers(size_t axis, const std::string& l1, const std::string& l2)
  {ravel_setCalipers(ravel,axis,l1.c_str(),l2.c_str());}
  void Ravel::orderLabels(size_t axis, HandleSort::Order order,
                          HandleSort::OrderType type, const std::string& format){
    ravel_orderLabels(ravel,axis,toEnum<RavelOrder>(order),toEnum<RavelOrderType>(type),
                      format.c_str());
  }
  void Ravel::applyCustomPermutation(size_t axis, const std::vector<size_t>& indices)
  {ravel_applyCustomPermutation(ravel,axis,indices.size(), indices.data());}
  std::vector<size_t> Ravel::currentPermutation(size_t axis) const {
    std::vector<size_t> r(numSliceLabels(axis));
    ravel_currentPermutation(ravel, axis, r.size(), r.data());
    return r;
  }
  
  void Ravel::addHandle(const std::string& description, const std::vector<std::string>& sliceLabels) {
    std::vector<const char*> sl;
    for (auto& i: sliceLabels) sl.push_back(i.c_str());
    ravel_addHandle(ravel, description.c_str(), sl.size(), sl.data());
  }
  
  std::string Ravel::toXML() const {return ravel_toXML(ravel);}
  void Ravel::fromXML(const std::string& xml) {
    if (!ravel_fromXML(ravel, xml.c_str())) throw std::runtime_error(ravel_lastErr());
  }
  HandleState Ravel::getHandleState(size_t handle) const {
    return *ravel_getHandleState(ravel, handle);
  }
  void Ravel::setHandleState(size_t handle, const HandleState& handleState) {
    RavelHandleStateX hs(handleState);
    ravel_setHandleState(ravel, handle, &hs);
  }
  RavelState Ravel::getRavelState() const {return *ravel_getRavelState(ravel);}
  void Ravel::setRavelState(const RavelState& rState) {
    RavelStateX cState(rState);
    ravel_setRavelState(ravel, &cState);
  }
  void Ravel::adjustSlicer(int n) {ravel_adjustSlicer(ravel,n);}

  void Ravel::redistributeHandles() {ravel_redistributeHandles(ravel);}
}
