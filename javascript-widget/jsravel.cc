#include "ravelCairo.h"
#include "dataCube.h"
#include "ravelCairoImpl.h"
#include "ravelVersion.h"
#include <string>
#include <emscripten/emscripten.h>
#include <emscripten/bind.h>
#include <iostream>
#include <fstream>

using namespace std;
using namespace ravel;
using namespace emscripten;

namespace ravel
{
  template class RavelCairo<val*>;
}

namespace {
  /// turn a container into an array
  template <class I>
  val arrayFromContainer(I begin, I end) {
    val r=val::array();
    for (size_t i=0; begin!=end; ++i, ++begin)
      r.set(i,*begin);
    return r;
  }
  
  struct JSDataCube: public DataCube
  {
    /// assign a Javascript function to this value
    val dataCallback=val::null();
    void setDataElement(size_t col, size_t row, double v) override
    {
      val key=val::array();
      key.set(0,col);
      key.set(1,row);
      dataCallback(key,v);
    }
    
    void loadData(const val& data) {
      // fill any nonexistent data with NaNs
      for (size_t i=0; i<rawData.size(); ++i) rawData[i] = nan("");
      for (size_t i=0; i<data["length"].as<size_t>()-1; i+=2)
        rawData[data[i].as<size_t>()]=data[i+1].as<double>();
    }
    
    void loadJSONData(const std::string& jsonData) {
      if (jsonData.empty()) return;
      const char* p=jsonData.c_str();
      // fill any nonexistent data with NaNs
      for (size_t i=0; i<rawData.size(); ++i) rawData[i] = nan("");

      if (p) p++; // skip leading '['
      while (*p)
        {
          char* p1;
          size_t i=strtold(p,&p1);
          if (p1==p) break; //invalid data
          p=p1;
          while (*p && *p!=',') p++;
          if (*p) p++; //skip comma
          
          double v=strtod(p,&p1);
          rawData[i] = (i<rawData.size() && p1>p)? v: nan("");
          p=p1;
          while (*p && *p!=',') p++;
          if (*p) p++; //skip comma
        }
    }

    // javascript interface to dimension
    void jdimension(const emscripten::val& v) {
      auto l=v["length"].as<unsigned>();
      LabelsVector lv;
      for (size_t i=0; i<l; ++i)
        {
          auto vi=v[i];
          lv.emplace_back(vi["axis"].as<string>(), vecFromJSArray<string>(vi["slice"]));
        }
      dimension(lv);
    }
    
    // set the dimensions of this datacube
    void dimension(const LabelsVector& lv) {
      rawData=RawDataIdx(lv);
      m_sortBy.resize(lv.size());
    }
  };
  
  struct JSRawData: public RawData
  {
    JSRawData() {}
    JSRawData(RawData&& x): RawData(x) {}
    // a bit clumsy, but couldn't figure out any natural way of converting a javascript array of objects to a Key
    double val(const emscripten::val& v) {
      auto l=v["length"].as<unsigned>();
      Key k(l);
      for (size_t i=0; i<l; ++i)
        {
          k[i].axis=v[i]["axis"].as<string>();
          k[i].slice=v[i]["slice"].as<string>();
        }
      try {return operator[](k);}
      catch (const std::exception& ex) {return nan("");} //invalid key
    }

    emscripten::val labelsVector() const {
      auto lv=RawData::labelsVector();
      auto v=emscripten::val::array();
      for (auto& i: lv)
        {
          auto vi=emscripten::val::object();
          vi.set("axis", i.first);
          vi.set("slice",arrayFromContainer(i.second.begin(), i.second.end()));
          v.call<void>("push",vi);
        }
      return v;
    }
  };

  // implements a function calling interface around SortedVector, as a sort of fake reference handling type, which current embind does not support
  class JSSortedVector
  {
    SortedVector* v;  //weak ref
    friend class JSHandle;
  public:
    void resize(size_t sz) {v->resize(sz);}
    void clear() {v->clear();}
    /// adds a label to the end and resets order to none.
    void push_back(const std::string& x) {v->push_back(x);}
    size_t idx(size_t i) const {return v->idx(i);}
    size_t size() const {return v->size();}
    bool empty() const {return v->empty();}
    /// returns current order in operation
    HandleSort::Order getOrder() const {return v->order();}
    /// sets order to \a o
    HandleSort::Order setOrder(HandleSort::Order o) {return v->order(o);}
    HandleSort::Order setOrder(HandleSort::Order o, HandleSort::OrderType t, const std::string& format={}) {return v->order(o,t,format);}
   /// returns true if order is numerically or lexicograpgically reverse
    bool orderReversed() const {return v->orderReversed();}
    void customPermutation(const val& x)
    {v->customPermutation(vecFromJSArray<size_t>(x));}
    bool isPermValid() const {return v->isPermValid();}
    /// set sets the labels to a Javascript array - resets order to none.
    void set(const val& x) {*v=SortedVector(vecFromJSArray<std::string>(x));}
    val get() const {
      val r{val::array()};
      for (size_t i=0; i<size(); ++i)
        r.set(i,(*v)[i]);
      return r;
    }
  };

  
  
  // implements a function calling interface around Handle, as a sort of fake reference handling type, which current embind does not support
  class JSHandle
  {
    Handle* h;  //weak ref
    Ravel::Handles* handles;
  public:
    JSHandle(Ravel::Handles& handles): handles(&handles) {get(0); /* best bet to avoid invalid dereferencing */}
    JSSortedVector sliceLabels;
    void get(size_t i) {h=&(*handles)[i]; sliceLabels.v=&h->sliceLabels;}
    
    double x() const {return h->x();}
    double y() const {return h->y();}
    Op::ReductionOp getReductionOp() const {return h->reductionOp;}
    Op::ReductionOp setReductionOp(Op::ReductionOp op) {
      h->reductionOp=op;
      if (!h->collapsed()) h->toggleCollapsed();
      return op;
    }
    std::string getDescription() const {return h->description;}
    std::string setDescription(const std::string& d) {return h->description=d;}
    std::string reductionDescription() const {return h->reductionDescription();}  
    size_t getSliceIndex() const {return h->sliceIndex;}
    size_t setSliceIndex(size_t x) {return h->sliceIndex=x;}
    size_t getSliceMin() const {return h->sliceMin();}
    //size_t setSliceMin(size_t x) {return h->sliceMin=x;}
    size_t getSliceMax() const {return h->sliceMax();}
    //size_t setSliceMax(size_t x) {return h->sliceMax=x;}
    void setSlicer(const std::string& label) {h->setSlicer(label);}
    const std::string& sliceLabel() const {return h->sliceLabel();}
    const std::string& sliceLabelAt(size_t i) const {return h->sliceLabels[i];}
    const std::string& minSliceLabel() const {return h->minSliceLabel();}
    const std::string& maxSliceLabel() const {return h->maxSliceLabel();}
    bool collapsed() const {return h->collapsed();}
    void toggleCollapsed() {h->toggleCollapsed();}
    bool getDisplayFilterCaliper() const {return h->displayFilterCaliper();}
    void setDisplayFilterCaliper(bool x) {h->displayFilterCaliper(x);}

    /// return an array of true/false according to whether there is
    /// data present for the slicelabel at that array index
    val mask() const {
      val r{val::array()};
      for (auto& i: h->mask)
        r.set(i,true);
      return r;
    }

    /// partial reductions
    void clearPartialReductions() {h->clearPartialReductions();}
    void addBinReduction(Bin::Op op, size_t size)
    {h->addPartialReduction(new Bin(op,size));}
    void addScanReduction(Scan::Op op, size_t window)
    {h->addPartialReduction(new Scan(op,window));}
    void addChangeReduction(Change::Op op, size_t offset)
    {h->addPartialReduction(new Change(op,offset));}
  };

  template <class Ravel>
  struct JSRavel: public Ravel
  {
    JSRavel() {
      EM_ASM(
             var uuid=document.getElementById("ravelInfo");
             if (uuid===null)
               {
                 uuid=document.createElement("div");
                 uuid.setAttribute("id","ravelInfo");
               }                 
             uuid.setAttribute("style","display:none");
             uuid.innerHTML="c45b9a57-e174-4732-8955-a7b6d23f1387";
             var bodies=document.getElementsByTagName("body");
             if (bodies.length>0)
               {
                 bodies[0].appendChild(uuid);
               }
             );
      val::global("document").call<val>("getElementById",string("ravelInfo")).
        call<void>("setAttribute",string("version"),string(RAVEL_VERSION));
    }

    void setRank(int r) {
      Ravel::handleIds.resize(r);
      for (size_t i=0; i<r; ++i)
        Ravel::handleIds[i]=i;
    }
    size_t addHandle(const std::string& description, const val& sliceLabels) {
      return Ravel::addHandle(description, vecFromJSArray<std::string>(sliceLabels));
    }

    JSHandle handle{Ravel::handles};
    val getHandleIds() const {return arrayFromContainer(Ravel::handleIds.begin(), Ravel::handleIds.end());}
    void setHandleIds(const val& x) {Ravel::handleIds=vecFromJSArray<size_t>(x);}
    size_t handleId(size_t i) const {return Ravel::handleIds[i];}
    string version() const {return RAVEL_VERSION;}
  };

  template <class Ravel>
  struct JSRavelBindings: public class_<JSRavel<Ravel>,base<Ravel>>
  {
    JSRavelBindings(const char* name): class_<JSRavel<Ravel>,base<Ravel>>(name)
      {
        this->constructor<>()
          .property("handle",&JSRavel<Ravel>::handle)
          .function("setRank",&JSRavel<Ravel>::setRank)
          .function("addHandle",&JSRavel<Ravel>::addHandle)
          .function("getHandleIds",&JSRavel<Ravel>::getHandleIds)
          .function("setHandleIds",&JSRavel<Ravel>::setHandleIds)
          .function("handleId",&JSRavel<Ravel>::handleId)
          .function("version",&JSRavel<Ravel>::version)
          .function("getState",&Ravel::getState)
          .function("setState",&Ravel::setState)
        ;
      }
  };

  struct JSRavelCanvas: public JSRavel<RavelCairo<val*>>
  {
    unique_ptr<val> canvasContext;
    void setCanvas(const val& x) { canvasContext.reset(new val(x)); setG(canvasContext.get());}
    void handleLeftKey() {
      if (auto h=selectedHandle()) h->moveSliceIdx(1);
    }
    void handleRightKey() {
      if (auto h=selectedHandle()) h->moveSliceIdx(-1);
    }
  };

  template<class Ravel>
  struct JSRavelDataCube: public Ravel
  {

    JSDataCube dc;
    JSRawData output;
    // method below doesn't work if placed in JSDataCube, but works here. Go figure!
    void setDataCallback(val f) {dc.dataCallback=f;}
    /// arg has the following structure:
    /// { dimensions: {axis: string, slice: string[]}[], data: double[]} 
    void loadData(const val& x) {
      dimension(x["dimensions"]);
      dc.loadData(x["data"]);
    }
    
    void hyperSlice() {output=dc.hyperSlice(*this);}
    void populateData() {dc.populateArray(*this);}
    /// dimension the datacube according to info in Ravel
    /// arg has the following structure:
    /// { dimensions: {axis: string, slice: string[]}[], data: double[]} 
    void dimension(const val& arg) {
      this->clear();
      for (size_t i=0; i<arg["length"].as<unsigned>(); ++i)
        ravel::Ravel::addHandle(arg[i]["axis"].as<string>(),
                                vecFromJSArray<string>(arg[i]["slice"]));
      dc.jdimension(arg);
      Ravel::redistributeHandles();
    }
  };

  template <class Ravel>
  struct JSRavelDataCubeBindings: public class_<JSRavelDataCube<Ravel>,base<Ravel>>
  {
    JSRavelDataCubeBindings(const char* name):
      class_<JSRavelDataCube<Ravel>,base<Ravel>>(name)
      {
        this->constructor<>()
          .property("dc",&JSRavelDataCube<Ravel>::dc)
          .property("output",&JSRavelDataCube<Ravel>::output)
          .function("setDataCallback",&JSRavelDataCube<Ravel>::setDataCallback)
          .function("hyperSlice",&JSRavelDataCube<Ravel>::hyperSlice)
          .function("populateData",&JSRavelDataCube<Ravel>::populateData)
          .function("dimension",&JSRavelDataCube<Ravel>::dimension)
          .function("loadData",&JSRavelDataCube<Ravel>::loadData)
          ;
      }
  };
}

EMSCRIPTEN_BINDINGS(Ravel) {

  enum_<Op::ReductionOp>("ReductionOp")
    .value("sum",Op::sum)
    .value("prod",Op::prod)
    .value("av",Op::av)
    .value("stddev",Op::stddev)
    .value("min",Op::min)
    .value("max",Op::max);
    
  enum_<HandleSort::Order>("Order")
    .value("none",HandleSort::none)
    .value("forward",HandleSort::forward)
    .value("reverse",HandleSort::reverse)
    .value("numForward",HandleSort::numForward)
    .value("numReverse",HandleSort::numReverse)
    .value("timeForward",HandleSort::timeForward)
    .value("timeReverse",HandleSort::timeReverse);

  enum_<HandleSort::OrderType>("OrderType")
    .value("string",HandleSort::string)
    .value("time",HandleSort::time)
    .value("value",HandleSort::value);

  enum_<Bin::Op>("BinOp")
    .value("add",Bin::add)
    .value("multiply",Bin::multiply)
    ;

   enum_<Scan::Op>("ScanOp")
    .value("add",Scan::add)
    .value("multiply",Scan::multiply)
    ;

   enum_<Change::Op>("ChangeOp")
    .value("subtract",Change::subtract)
    .value("divide",Change::divide)
    .value("percent",Change::percent)
    .value("relative",Change::relative)
    ;


  class_<HandleState>("HandleState")
    .property("description",&HandleState::description)
    .property("x",&HandleState::x)
    .property("y",&HandleState::y)
    .property("collapsed",&HandleState::collapsed)
    .property("displayFilterCaliper",&HandleState::displayFilterCaliper)
    .property("reductionOp",&HandleState::reductionOp)
    .property("order",&HandleState::order)
    .property("customOrder",&HandleState::customOrder)
    .property("minLabel",&HandleState::minLabel)
    .property("maxLabel",&HandleState::maxLabel)
    .property("sliceLabel",&HandleState::sliceLabel)
    ;

  class_<RavelState>("RavelState")
    .property("radius",&RavelState::radius)
    .property("sortByValue",&RavelState::sortByValue)
    .property("handleStates",&RavelState::handleStates)
    .property("outputHandles",&RavelState::outputHandles)
    ;

  class_<JSSortedVector>("SortedVector")
    .function("resize",&JSSortedVector::resize)
    .function("clear",&JSSortedVector::clear)
    .function("push_back", &JSSortedVector::push_back)
    .function("idx",&JSSortedVector::idx)
    .function("size",&JSSortedVector::size)
    .function("empty",&JSSortedVector::empty)
    .function("getOrder",&JSSortedVector::getOrder)
    .function("setOrder",select_overload<HandleSort::Order(HandleSort::Order)>(&JSSortedVector::setOrder))
    .function("setOrder",select_overload<HandleSort::Order(HandleSort::Order,HandleSort::OrderType,const std::string&)>(&JSSortedVector::setOrder))
    .function("orderReversed",&JSSortedVector::orderReversed)
    .function("customPermutation",&JSSortedVector::customPermutation)
    .function("isPermValid",&JSSortedVector::isPermValid)
    .function("get",&JSSortedVector::get)
    .function("set",&JSSortedVector::set)
    .function("setCalipers",&SortedVector::setCalipers)
    ;
  
  class_<JSHandle>("Handle")
    .property("sliceLabels",&JSHandle::sliceLabels)
    .function("get",&JSHandle::get)
    .function("x",&JSHandle::x)
    .function("y",&JSHandle::y)
    .function("getReductionOp",&JSHandle::getReductionOp)
    .function("setReductionOp",&JSHandle::setReductionOp)
    .function("getDescription",&JSHandle::getDescription)
    .function("setDescription",&JSHandle::setDescription)
    .function("reductionDescription",&JSHandle::reductionDescription)
    .function("getSliceIndex",&JSHandle::getSliceIndex)
    .function("setSliceIndex",&JSHandle::setSliceIndex)
    .function("getSliceMin",&JSHandle::getSliceMin)
    //.function("setSliceMin",&JSHandle::setSliceMin)
    .function("getSliceMax",&JSHandle::getSliceMax)
    //.function("setSliceMax",&JSHandle::setSliceMax)
    .function("setSlicer",&JSHandle::setSlicer)
    .function("sliceLabel",&JSHandle::sliceLabel)
    .function("sliceLabelAt",&JSHandle::sliceLabelAt)
    .function("minSliceLabel",&JSHandle::minSliceLabel)
    .function("maxSliceLabel",&JSHandle::maxSliceLabel)
    .function("collapsed",&JSHandle::collapsed)
    .function("toggleCollapsed",&JSHandle::toggleCollapsed)
    .function("getDisplayFilterCaliper",&JSHandle::getDisplayFilterCaliper)
    .function("setDisplayFilterCaliper",&JSHandle::setDisplayFilterCaliper)
    .function("mask",&JSHandle::mask)
    .function("clearPartialReductions",&JSHandle::clearPartialReductions)
    .function("addBinReduction",&JSHandle::addBinReduction)
    .function("addScanReduction",&JSHandle::addScanReduction)
    .function("addChangeReduction",&JSHandle::addChangeReduction)
    .function("getHandleState",&Handle::getHandleState)
    .function("setHandleState",&Handle::setHandleState)
    ;
  
  class_<Ravel>("RavelBase")
    .constructor<>()
    .property("x",&Ravel::x)
    .property("y",&Ravel::y)
    .function("numHandles",optional_override([](const Ravel& self){return self.handles.size();}))
    .function("rank",&Ravel::rank)
    .function("radius",&Ravel::radius)
    .function("clear",&Ravel::clear)
    .function("moveHandleTo",&Ravel::moveHandleTo)
    .function("rescale",&Ravel::rescale)
    .function("onMouseMotion",&Ravel::onMouseMotion)
    .function("onMouseDown",&Ravel::onMouseDown)
    .function("onMouseUp",&Ravel::onMouseUp)
    .function("handleIfMouseOver",&Ravel::handleIfMouseOver)
    .function("description",&Ravel::description)
    .function("redistributeHandles",&Ravel::redistributeHandles)
    ;

  JSRavelBindings<Ravel>("Ravel");
                  
  class_<RavelCairo<val*>,base<Ravel>>("RavelCairoVal*")
    .function("onMouseDown",&RavelCairo<val*>::onMouseDown)
    .function("onMouseOver",&RavelCairo<val*>::onMouseOver)
    .function("onMouseLeave",&RavelCairo<val*>::onMouseLeave)
    .function("render",&RavelCairo<val*>::render)
    .function("selectedHandleId",&RavelCairo<val*>::selectedHandleId)
    ;

  JSRavelBindings<RavelCairo<val*>>("RavelCanvasBase");
  
  class_<JSRavelCanvas,base<JSRavel<RavelCairo<val*>>>>("RavelCanvas")
    .constructor<>()
    .function("setCanvas",&JSRavelCanvas::setCanvas)
    .function("handleLeftKey",&JSRavelCanvas::handleLeftKey)
    .function("handleRightKey",&JSRavelCanvas::handleRightKey)
    ;

  class_<DataSpec>("DataSpec")
    .property("nRowAxes",&DataSpec::nRowAxes)
    .property("nColAxes",&DataSpec::nColAxes)
    .function("getCommentRows",optional_override([](const DataSpec& self){
          val c=val::array();
          size_t i=0;
          for (auto j: self.commentRows) c.set(i++,j);
          return c;
        }))
    .function("getCommentCols",optional_override([](const DataSpec& self){
          val c=val::array();
          size_t i=0;
          for (auto j: self.commentCols) c.set(i++,j);
          return c;
        }))
    .function("setCommentRows",optional_override([](DataSpec& self, const val& x){
          auto xv=vecFromJSArray<unsigned>(x);
          self.commentRows.clear();
          self.commentRows.insert(xv.begin(), xv.end());
        }))
    .function("setCommentCols",optional_override([](DataSpec& self, const val& x){
          auto xv=vecFromJSArray<unsigned>(x);
          self.commentCols.clear();
          self.commentCols.insert(xv.begin(), xv.end());
        }))
    ;

  class_<DataCube>("_DataCube")
    .function("size",&DataCube::size)
    .function("rank",&DataCube::rank)
    ;
  
  class_<JSDataCube,base<DataCube>>("DataCube")
    .constructor<>()
    .function("loadData",&JSDataCube::loadData)
    .function("dimension",&JSDataCube::jdimension)
    ;

  JSRavelDataCubeBindings<Ravel>("RavelDataCube");
  JSRavelDataCubeBindings<JSRavelCanvas>("RavelCanvasDataCube");
  
  class_<RawDataIdx>("RawDataIdx")
    .function("size",&RawData::size)
    .function("rank",&RawData::rank)
    .function("dim",&RawDataIdx::dim)
    .function("axis",&RawDataIdx::axis)
    .function("offset",&RawDataIdx::offset)
    ;

  class_<RawData,base<RawDataIdx>>("RawDataBase");

  class_<JSRawData,base<RawData>>("RawData")
    .function("val",&JSRawData::val)
    .function("labelsVector",&JSRawData::labelsVector)
    ;

  register_vector<std::string>("vector<string>");
  register_vector<HandleState>("vector<HandleState>");
  
}


