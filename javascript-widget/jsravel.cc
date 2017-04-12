#include "ravelCairo.h"
#include "dataCube.h"
#include "ravelCairoImpl.h"
#include "ravelVersion.h"
#include <string>
#include <emscripten/emscripten.h>
#include <emscripten/bind.h>
#include <consoleLog.h>

using namespace std;
using namespace ravel;
using namespace emscripten;

namespace ravel
{
  template class RavelCairo<val*>;
  ConsoleLog console;
  ConsoleLogEndl endl;
}

namespace {
using ravel::endl;
  struct JSDataCube: public DataCube
  {
    /// assign a Javascript function to this value
    val dataCallback=val::null();
    void setDataElement(size_t col, size_t row, double v) override
    {dataCallback(col,row,v);}
    
    void loadData(const std::string& jsonData) {
      if (jsonData.empty()) return;
      const char* p=jsonData.c_str();
      // fill any nonexistent data with NaNs
      for (size_t i=0; i<rawData.size(); ++i) rawData[i++] = nan("");

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

    // set the dimensions of this datacube
    void dimension(const LabelsVector& lv) {
      rawData=RawDataIdx(lv);
      m_sortBy.resize(lv.size());
    }
  };
  
  struct JSRawData: public RawData
  {
    JSRawData(RawData&& x): RawData(x) {}
    //    double val(size_t i){return operator[](i);}
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
  };

  /// turn a container into an array
  template <class I>
  val arrayFromContainer(I begin, I end) {
    val r=val::array();
    for (size_t i=0; begin!=end; ++i, ++begin)
      r.set(i,*begin);
    return r;
  }
  
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
   /// returns true if order is numerically or lexicograpgically reverse
    bool orderReversed() const {return v->orderReversed();}
    void customPermutation(const val& x) {v->customPermutation(vecFromJSArray<size_t>(x));}
    bool isPermValid() const {return v->isPermValid();}
    /// set sets the labels to a Javascript array - resets order to none.
    void set(const val& x) {*v=SortedVector(vecFromJSArray<std::string>(x));}
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
    Op::ReductionOp setReductionOp(Op::ReductionOp op) {return h->reductionOp=op;}
    std::string getDescription() const {return h->description;}
    std::string setDescription(const std::string& d) {return h->description=d;}
    std::string reductionDescription() const {return h->reductionDescription();}  
    size_t getSliceIndex() const {return h->sliceIndex;}
    size_t setSliceIndex(size_t x) {return h->sliceIndex=x;}
    size_t getSliceMin() const {return h->sliceMin;}
    size_t setSliceMin(size_t x) {return h->sliceMin=x;}
    size_t getSliceMax() const {return h->sliceMax;}
    size_t setSliceMax(size_t x) {return h->sliceMax=x;}
    const std::string& sliceLabel() const {return h->sliceLabel();}
    const std::string& sliceLabelAt(size_t i) const {return h->sliceLabels[i];}
    const std::string& minSliceLabel() const {return h->minSliceLabel();}
    const std::string& maxSliceLabel() const {return h->maxSliceLabel();}
    bool collapsed() const {return h->collapsed();}
    void moveTo(double x, double y, bool dontCollapse) {h->moveTo(x,y,dontCollapse);}
    void toggleCollapsed() {h->toggleCollapsed();}
    bool getDisplayFilterCaliper() const {return h->displayFilterCaliper;}
    void setDisplayFilterCaliper(bool x) {h->displayFilterCaliper=x;}
  };
  
  struct JRavelCairo: public RavelCairo<val*>
  {
    unique_ptr<val> canvasContext;

    JRavelCairo() {
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

    void setCanvas(const val& x) { canvasContext.reset(new val(x)); setG(canvasContext.get());}
    JSDataCube dc;
    void setDataCallback(val f) {dc.dataCallback=f;}

    void loadData(const std::string& jsonData) {dc.loadData(jsonData);}
    JSRawData hyperSlice() {return JSRawData(dc.hyperSlice(*this));}
    void populateData() {dc.populateArray(*this);}
    /// dimension the datacube according to info in Ravel
    void dimension(const val& arg) {
      map<string,vector<string>> labels;
      for (auto& h: Ravel::handles)
        {
          labels[h.description]=vector<string>(h.sliceLabels.begin(),h.sliceLabels.end());
          // forward sort axis labels always for now.
          h.sliceLabels.order(HandleSort::forward);
        }

      LabelsVector lv;
      vector<string> axes;
      for (size_t i=0; i<arg["length"].as<unsigned>(); ++i)
        axes.push_back(arg[i].as<string>());
      for (auto& axis: axes)
          lv.emplace_back(axis, labels[axis]);
      dc.dimension(lv);
      redistributeHandles();
    }
    void setRank(int r) {
      Ravel::handleIds.resize(r);
      for (size_t i=0; i<r; ++i)
        Ravel::handleIds[i]=i;
    }
    void setSlicer(size_t handle, const std::string& label)
    {
      auto& h=Ravel::handles[handle];
      for (size_t i=0; i<h.sliceLabels.size(); ++i)
        if (h.sliceLabels[i]==label)
          {
            h.sliceIndex=i;
            break;
          }
    }
    void handleLeftKey() {
      if (auto h=selectedHandle()) h->moveSliceIdx(1);
    }
    void handleRightKey() {
      if (auto h=selectedHandle()) h->moveSliceIdx(-1);
    }
    size_t addHandle(const std::string& description, const val& sliceLabels) {
      return Ravel::addHandle(description, vecFromJSArray<std::string>(sliceLabels));
    }

    JSHandle handle{Ravel::handles};
    val getHandleIds() const {return arrayFromContainer(handleIds.begin(), handleIds.end());}
    void setHandleIds(const val& x) {handleIds=vecFromJSArray<size_t>(x);}
    size_t handleId(size_t i) const {return handleIds[i];}
  };

  struct RavelCairoWrapper: public wrapper<JRavelCairo> {
    EMSCRIPTEN_WRAPPER(RavelCairoWrapper);
  };

  struct DataCubeWrapper: public wrapper<JSDataCube> {
    EMSCRIPTEN_WRAPPER(DataCubeWrapper);
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
    .value("numReverse",HandleSort::numReverse);

  class_<JSSortedVector>("SortedVector")
    .function("resize",&JSSortedVector::resize)
    .function("clear",&JSSortedVector::clear)
    .function("push_back", &JSSortedVector::push_back)
    .function("idx",&JSSortedVector::idx)
    .function("size",&JSSortedVector::size)
    .function("empty",&JSSortedVector::empty)
    .function("getOrder",&JSSortedVector::getOrder)
    .function("setOrder",&JSSortedVector::setOrder)
    .function("orderReversed",&JSSortedVector::orderReversed)
    .function("customPermutation",&JSSortedVector::customPermutation)
    .function("isPermValid",&JSSortedVector::isPermValid)
    .function("set",&JSSortedVector::set)
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
    .function("setSliceMin",&JSHandle::setSliceMin)
    .function("getSliceMax",&JSHandle::getSliceMax)
    .function("setSliceMax",&JSHandle::setSliceMax)
    .function("sliceLabel",&JSHandle::sliceLabel)
    .function("sliceLabelAt",&JSHandle::sliceLabelAt)
    .function("minSliceLabel",&JSHandle::minSliceLabel)
    .function("maxSliceLabel",&JSHandle::maxSliceLabel)
    .function("collapsed",&JSHandle::collapsed)
    .function("moveTo",&JSHandle::moveTo)
    .function("toggleCollapsed",&JSHandle::toggleCollapsed)
    .function("getDisplayFilterCaliper",&JSHandle::getDisplayFilterCaliper)
    .function("setDisplayFilterCaliper",&JSHandle::setDisplayFilterCaliper)
    ;
  
  class_<Ravel>("Ravel")
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
    .function("handleX",&Ravel::handleX)
    .function("handleY",&Ravel::handleY)
    .function("description",&Ravel::description)
    .function("redistributeHandles",&Ravel::redistributeHandles)
    .function("setSliceCoordinates",&Ravel::setSliceCoordinates)
    ;
  
  class_<RavelCairo<val*>,base<Ravel>>("RavelCairoval*")
    .function("onMouseDown",&RavelCairo<val*>::onMouseDown)
    .function("onMouseOver",&RavelCairo<val*>::onMouseOver)
    .function("handleIfMouseOverAxisLabel",&RavelCairo<val*>::handleIfMouseOverAxisLabel)
    .function("handleIfMouseOverOpLabel",&RavelCairo<val*>::handleIfMouseOverOpLabel)
    .function("handleIfMouseOverCaliperLabel",
              &RavelCairo<val*>::handleIfMouseOverCaliperLabel)
    .function("render",&RavelCairo<val*>::render)
    ;

  class_<JSDataCube>("DataCube")
    .property("dataCallback",&JSDataCube::dataCallback)
    .function("loadData",&JSDataCube::loadData)
    .function("dimension",&JSDataCube::dimension)
    .constructor<>()
    ;

  class_<JRavelCairo,base<RavelCairo<val*>>>("RavelDataCube")
    .allow_subclass<RavelCairoWrapper>("RavelCairoWrapper")
    .property("dc",&JRavelCairo::dc)
    .property("handle",&JRavelCairo::handle)
    .function("setCanvas",&JRavelCairo::setCanvas)
    .function("hyperSlice",&JRavelCairo::hyperSlice)
    .function("populateData",&JRavelCairo::populateData)
    .function("dimension",&JRavelCairo::dimension)
    .function("loadData",&JRavelCairo::loadData)
    .function("setDataCallback",&JRavelCairo::setDataCallback)
    .function("setRank",&JRavelCairo::setRank)
    .function("setSlicer",&JRavelCairo::setSlicer)
    .function("addHandle",&JRavelCairo::addHandle)
    .function("handleLeftKey",&JRavelCairo::handleLeftKey)
    .function("handleRightKey",&JRavelCairo::handleRightKey)
    .function("getHandleIds",&JRavelCairo::getHandleIds)
    .function("setHandleIds",&JRavelCairo::setHandleIds)
    .function("handleId",&JRavelCairo::handleId)
    .constructor<>()
    ;

  class_<RawDataIdx>("RawDataIdx")
    .function("size",&RawData::size)
    .function("rank",&RawData::rank)
    ;

  class_<RawData,base<RawDataIdx>>("RawDataBase");

  class_<JSRawData,base<RawData>>("RawData")
    .function("val",&JSRawData::val)
    ;
}


