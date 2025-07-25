/*
  Ravel C API. © Ravelation Pty Ltd 2020
  Open source licensed under the MIT license.
*/

#include "dynamicRavelCAPI.h"
#include "ravelCAPITypes.h"
#include "ravelCivita.h"
#ifdef WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include <stdexcept>
using namespace std;

/// C API version this is compiled against
static const int ravelVersion=RAVEL_CAPI_VERSION;

#include "ravelRelease.h"

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
  {
    return dlopen((lib+".so").c_str(),RTLD_NOW);
  }
#endif
  
  struct RavelLib
  {
    libHandle lib=nullptr;
    string errorMsg;
    string versionFound="unavailable";
    RavelLib()
    {
#ifndef WIN32
      // get ravel plugin from special place
      if (auto home=getenv("HOME"))
        if (!(lib=loadLibrary(string(home)+"/.ravel/libravel")))
          {
            errorMsg=dlerror();
            errorMsg+=" & ";
            lib=loadLibrary("libravel");
          }
#else
      lib=loadLibrary("libravel");
#endif
      
      if (!lib)
        {
          errorMsg+=dlerror();
          return;
        }
      
      auto version=(const char* (*)())dlsym(lib,"ravel_version");
      auto capi_version=(int (*)())dlsym(lib,"ravel_capi_version");
      if (version) versionFound=version();
      if (!version || !capi_version || ravelVersion!=capi_version())
        { // incompatible API
          errorMsg="Incompatible libravel dynamic library found";
          errorMsg+=string("\nBuilt against: ")+ravelRelease;
          errorMsg+=string("\nVersion found:")+versionFound;
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
  template <>
  struct RavelFn<const char*>
  {
    const char* (*f)()=nullptr;
    RavelFn(const char*name, libHandle lib) {ravelLib.asgFnPointer(f,name);}
    const char* operator()() {return f? f(): "";}
  };
  template <class R, class A>
  struct RavelFn<R,A>
  {
    R (*f)(A)=nullptr;
    RavelFn(const char*name, libHandle lib) {ravelLib.asgFnPointer(f,name);}
    R operator()(A a) {return f? f(a): R{};}
  };
  template <class A>
  struct RavelFn<const char*,A>
  {
    const char* (*f)(A)=nullptr;
    RavelFn(const char*name, libHandle lib) {ravelLib.asgFnPointer(f,name);}
    const char* operator()(A a) {return f? f(a): "";}
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
  struct RavelFn<const char*,A0,A1>
  {
    const char* (*f)(A0,A1)=nullptr;
    RavelFn(const char*name, libHandle lib) {ravelLib.asgFnPointer(f,name);}
    const char* operator()(A0 a0, A1 a1) {return f? f(a0,a1): "";}
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
  struct RavelFn<const char*,A0,A1,A2>
  {
    const char* (*f)(A0,A1,A2)=nullptr;
    RavelFn(const char*name, libHandle lib) {ravelLib.asgFnPointer(f,name);}
    const char* operator()(A0 a0, A1 a1,A2 a2) {return f? f(a0,a1,a2): "";}
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
  DEFFN(ravel_pro, bool);
  DEFFN(ravel_days_until_expiry, int);
  DEFFN(ravel_new, CAPIRavel*,size_t);
  DEFFN(ravel_delete, void, CAPIRavel*);
  DEFFN(ravel_clear, void, CAPIRavel*);
  DEFFN(ravel_cancel, void, bool);
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
  DEFFN(ravel_state_description, const char*, const CAPIRavelState*);
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
  DEFFN(ravel_getCaliperPositions,void,CAPIRavel*,size_t,size_t*,size_t*);
  DEFFN(ravel_setCaliperPositions,void,CAPIRavel*,size_t,size_t,size_t);
  DEFFN(ravel_orderLabels, void, CAPIRavel*, size_t,RavelOrder);
  DEFFN(ravel_nextReduction, void, CAPIRavel*, RavelReductionOp);
  DEFFN(ravel_handleSetReduction, void, CAPIRavel*, int, RavelReductionOp);

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
  DEFFN(ravel_sortByValue,void, CAPIRavel*, const CAPITensor*, RavelOrder);
  DEFFN(ravel_hyperSlice,CAPITensor*, CAPIRavel*, const CAPITensor*);
  DEFFN(ravel_populateFromHypercube,int, CAPIRavel*, const char*);
  DEFFN(ravel_connect,CAPIRavelDatabase*, const char*, const char*, const char*);
  DEFFN(ravel_close,void,CAPIRavelDatabase*);
  DEFFN(ravel_createTable,bool,CAPIRavelDatabase*,const char*,const CAPIRavelDataSpec*);
  DEFFN(ravel_loadDatabase,bool,CAPIRavelDatabase*,const char**,const CAPIRavelDataSpec*);
  DEFFN(ravel_loadDatabaseCallback,void,CAPIRavelDatabase*,void(*)(const char*,double));
  DEFFN(ravel_deduplicate,void,CAPIRavelDatabase*, CAPIRavelDuplicateKey,const CAPIRavelDataSpec*);
  DEFFN(ravel_dbTableNames, const char**, CAPIRavelDatabase*, size_t*);
  DEFFN(ravel_dbNumericalColumnNames, const char**, CAPIRavelDatabase*, size_t*);
  DEFFN(ravel_setAxisNames,void,CAPIRavelDatabase*, const char**, size_t, const char*);
  DEFFN(ravel_dbFullHypercube,void,CAPIRavel*, CAPIRavelDatabase*);
  DEFFN(ravel_dbHyperSlice,const CAPITensor*,CAPIRavel*, CAPIRavelDatabase*);
}

namespace ravelCAPI
{

  bool available() {return ravelLib.lib;}
  bool ravelPro() {return ravel_pro();}
  std::string lastError() {return ravelLib.errorMsg;}
  std::string version() {return ravelLib.versionFound;}
  int daysUntilExpired() {return ravelLib.lib?  ravel_days_until_expiry(): -1;}
  std::string description(const RavelState& s) {
    RavelStateX tmp(s);
    return ravel_state_description(&tmp);
  }

  Ravel::Ravel()
  {
    if (ravelLib.lib)
      {
        if (!(ravel=ravel_new(0)))
          ravelLib.errorMsg=ravel_lastErr();
        else if (daysUntilExpired()<0)
          ravelLib.errorMsg="Expired";
      }
  }
  
  Ravel::~Ravel() {
    if (ravel) ravel_delete(ravel);
  }

  void Ravel::clear() {ravel_clear(ravel);}
  void Ravel::cancel(bool x) {ravel_cancel(x);}
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
  std::pair<size_t,size_t> Ravel::getCaliperPositions(size_t axis)
  {
    std::pair<size_t,size_t> r;
    ravel_getCaliperPositions(ravel,axis,&r.first,&r.second);
    return r;
  }
  void Ravel::setCaliperPositions(size_t axis, size_t p1, size_t p2)
  {
    ravel_setCaliperPositions(ravel,axis,p1,p2);
  }

  void Ravel::orderLabels(size_t axis, HandleSort::Order order){
    ravel_orderLabels(ravel,axis,toEnum<RavelOrder>(order));
  }
  void Ravel::nextReduction(Op::ReductionOp op)
  {ravel_nextReduction(ravel,toEnum<RavelReductionOp>(op));}
  void Ravel::handleSetReduction(int handle, Op::ReductionOp op)
  {ravel_handleSetReduction(ravel,handle,toEnum<RavelReductionOp>(op));}
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
  HandleState Ravel::getHandleState(size_t handle) const
  {
    if (ravel)
      return *ravel_getHandleState(ravel, handle);
    return {};
  }
  void Ravel::setHandleState(size_t handle, const HandleState& handleState)
  {
    RavelHandleStateX hs(handleState);
    ravel_setHandleState(ravel, handle, &hs);
  }
  RavelState Ravel::getRavelState() const
  {
    if (ravel)
      return *ravel_getRavelState(ravel);
    return {};
  }
  void Ravel::setRavelState(const RavelState& rState) {
    RavelStateX cState(rState);
    ravel_setRavelState(ravel, &cState);
  }
  void Ravel::adjustSlicer(int n) {ravel_adjustSlicer(ravel,n);}

  void Ravel::redistributeHandles() {ravel_redistributeHandles(ravel);}

  void Ravel::sortByValue(const civita::TensorPtr& input, HandleSort::Order dir)
  {
    ravel::CAPITensor capiTensor(*input);
    ravel_sortByValue(ravel, &capiTensor, toEnum<RavelOrder>(dir));
  }

  namespace
  {
    // structure to maintain lifetimes of input and wrappers in a chain
    struct Chain: public TensorWrap
    {
      civita::TensorPtr input;
      std::unique_ptr<ravel::CAPITensor> capiTensor;
      Chain(const ::CAPITensor& output, const civita::TensorPtr& input,
            std::unique_ptr<ravel::CAPITensor>&& capiTensor):
        TensorWrap(output), input(input), capiTensor(std::move(capiTensor)) {}
    };
  }
  
  civita::TensorPtr Ravel::hyperSlice(const civita::TensorPtr& arg) const
  {
    if (!arg) return nullptr;
    std::unique_ptr<ravel::CAPITensor> capiTensor(new ravel::CAPITensor(*arg));
    auto r=ravel_hyperSlice(ravel, capiTensor.get());
    if (!r) throw std::runtime_error(ravel_lastErr());
    return make_shared<Chain>(*r,arg,std::move(capiTensor));
  }

  /// sets handles and slices from \a hc
  void Ravel::populateFromHypercube(const civita::Hypercube& hc)
  {
    if (!ravel_populateFromHypercube(ravel, hc.json().c_str()))
      throw std::runtime_error(ravel_lastErr());
  }

  void Database::connect(const string& dbType, const string& connect, const string& table)
  {
    close();
    if (!(db=ravel_connect(dbType.c_str(),connect.c_str(),table.c_str())))
      throw runtime_error(ravel_lastErr());
    m_connection={dbType,connect,table};
  }

  void Database::close() {ravel_close(db); m_connection={}; db=nullptr;}

  void Database::createTable(const string& filename, const DataSpec& spec)
  {
    RavelDataSpec s(spec);
    if (!ravel_createTable(db,filename.c_str(),&s))
       throw std::runtime_error(ravel_lastErr());
  }

  void Database::loadDatabase(const vector<string>& filenames, const DataSpec& spec)
  {
    if (filenames.empty()) return;
    RavelDataSpec s(spec);
    vector<const char*> f;
    for (auto& i: filenames) f.push_back(i.c_str());
    f.push_back(nullptr);
    if (!ravel_loadDatabase(db,&f[0],&s))
       throw std::runtime_error(ravel_lastErr());
  }

  void Database::loadDatabaseCallback(void(*progress)(const char* filename,double fraction))
  {ravel_loadDatabaseCallback(db,progress);}
  
  void Database::deduplicate(DuplicateKeyAction::Type duplicateKeyAction, const DataSpec& spec)
  {
    RavelDataSpec s(spec);
    ravel_deduplicate(db,toEnum<CAPIRavelDuplicateKey>(duplicateKeyAction),&s);
  }
  
  vector<string> Database::tableNames() const
  {
    if (db)
      {
        size_t numTables;
        auto tn=ravel_dbTableNames(db,&numTables);
        return vector<string>(tn,tn+numTables);
      }
    return {};
  }
  
  vector<string> Database::numericalColumnNames() const
  {
    if (db)
      {
        size_t size;
        auto names=ravel_dbNumericalColumnNames(db,&size);
        return vector<string>(names,names+size);
      }
    return {};
  }

  
  void Database::setAxisNames(const std::vector<std::string>& axisNames, const std::string& horizontaDimension)
  {
    vector<const char*> aNames;
    for (auto& i: axisNames) aNames.push_back(i.c_str());
    ravel_setAxisNames(db,aNames.data(),aNames.size(),horizontaDimension.c_str());
  }
  
    void Database::fullHypercube(Ravel& ravel)
    {
      ravel_dbFullHypercube(ravel.ravel, db);
    }
  
    /// Extract the datacube corresponding to the state of the ravel applied to the database
    civita::TensorPtr Database::hyperSlice(const Ravel& ravel)
    {
      if (auto r=ravel_dbHyperSlice(ravel.ravel, db))
        return make_shared<TensorWrap>(*r);
      throw std::runtime_error(ravel_lastErr());
    }
}
