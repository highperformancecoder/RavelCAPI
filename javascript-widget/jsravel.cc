#include "ravelCairo.h"
#include "dataCube.h"
#include "ravelCairoImpl.h"
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
  
  struct JRavelCairo: public RavelCairo<val*>
  {
    unique_ptr<val> canvasContext;
    void setCanvas(const val& x) { canvasContext.reset(new val(x)); setG(canvasContext.get());}
    JSDataCube dc;
    void setDataCallback(val f) {dc.dataCallback=f;}

    void loadData(const std::string& jsonData) {dc.loadData(jsonData);}
    JSRawData hyperSlice() {return JSRawData(dc.hyperSlice(*this));}
    void populateData() {dc.populateArray(*this);}
    /// dimension the datacube according to info in Ravel
    void dimension(const val& arg) {
      map<string,vector<string>> labels;
      for (auto& h: handles)
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
      handleIds.resize(r);
      for (size_t i=0; i<r; ++i)
        handleIds[i]=i;
    }
    void setSlicer(size_t handle, const std::string& label)
    {
      auto& h=handles[handle];
      for (size_t i=0; i<h.sliceLabels.size(); ++i)
        if (h.sliceLabels[i]==label)
          {
            h.sliceIndex=i;
            break;
          }
    }
    void toggleCollapsed(size_t h) {handles[h].toggleCollapsed();}
    void handleLeftKey() {
      if (auto h=selectedHandle()) h->moveSliceIdx(1);
    }
    void handleRightKey() {
      if (auto h=selectedHandle()) h->moveSliceIdx(-1);
    }
    void setReductionOp(size_t i, int op)
    {
      nextRedOp=Op::ReductionOp(op);
      handles[i].reductionOp=nextRedOp;
    }
    void setSort(size_t i, int o)
    {
      handles[i].sliceLabels.order(HandleSort::Order(o));
    }
    void setDisplayFilter(size_t i, bool filtering)
    {
      handles[i].displayFilterCaliper=filtering;
    }
  };

  struct RavelCairoWrapper: public wrapper<JRavelCairo> {
    EMSCRIPTEN_WRAPPER(RavelCairoWrapper);
  };

  typedef std::pair<std::string,std::vector<std::string>> Labels;

  struct DataCubeWrapper: public wrapper<JSDataCube> {
    EMSCRIPTEN_WRAPPER(DataCubeWrapper);
  };
}

EMSCRIPTEN_BINDINGS(Ravel) {
  enum_<AnchorPoint::Anchor>("Anchor")
    .value("ne",AnchorPoint::ne)
    .value("nw",AnchorPoint::nw)
    .value("se",AnchorPoint::se)
    .value("sw",AnchorPoint::sw);

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

  class_<AnchorPoint>("AnchorPoint")
    .constructor<>()
    .property("x",&AnchorPoint::x)
    .property("y",&AnchorPoint::y)
    .property("anchor",&AnchorPoint::anchor);
  
  class_<Handle>("Handle")
    .constructor<>()
    .function("x",&Handle::x)
    .function("y",&Handle::y)
    .property("description",&Handle::description)
    .function("sliceLabels",optional_override([](const Handle& self, size_t i){return self.sliceLabels[i];}))
    .function("sliceIdx",optional_override([](const Handle& self, size_t i){return self.sliceLabels.idx(i);}))
    .function("numSliceLabels",&Handle::numSliceLabels)
    .function("reductionDescription",&Handle::reductionDescription)
    .function("sliceX",&Handle::sliceX)
    .function("sliceY",&Handle::sliceY)
    .function("sliceLabel",&Handle::sliceLabel)
    .function("labelAnchor",&Handle::labelAnchor)
    .function("collapsed",&Handle::collapsed)
    .function("toggleCollapsed",&Handle::toggleCollapsed)
    ;
  
  class_<Ravel>("Ravel")
    .constructor<>()
    .property("x",&Ravel::x)
    .property("y",&Ravel::y)
    .function("handleIds",optional_override([](const Ravel& self, size_t i){return self.handleIds[i];}))
    .function("setHandleIds",optional_override([](Ravel& self, const val& ids){return self.handleIds=vecFromJSArray<size_t>(ids);}))
    .function("handles",optional_override([](const Ravel& self, size_t i){return self.handles[i];}))
    .function("numHandles",optional_override([](const Ravel& self){return self.handles.size();}))
    .function("addHandle",&Ravel::addHandle)
    .function("rank",&Ravel::rank)
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

  class_<Labels>("Labels")
    .property("axis",&Labels::first)
    .property("labels",&Labels::second);

  class_<JSDataCube>("JSDataCube")
    .property("dataCallback",&JSDataCube::dataCallback)
    .function("loadData",&JSDataCube::loadData)
    .function("dimension",&JSDataCube::dimension)
    .constructor<>()
    ;

  class_<JRavelCairo,base<RavelCairo<val*>>>("RavelCairo")
    .allow_subclass<RavelCairoWrapper>("RavelCairoWrapper")
    .property("dc",&JRavelCairo::dc)
    .function("setCanvas",&JRavelCairo::setCanvas)
    .function("hyperSlice",&JRavelCairo::hyperSlice)
    .function("populateData",&JRavelCairo::populateData)
    .function("dimension",&JRavelCairo::dimension)
    .function("loadData",&JRavelCairo::loadData)
    .function("setDataCallback",&JRavelCairo::setDataCallback)
    .function("setRank",&JRavelCairo::setRank)
    .function("setSlicer",&JRavelCairo::setSlicer)
    .function("toggleCollapsed",&JRavelCairo::toggleCollapsed)
    .function("handleLeftKey",&JRavelCairo::handleLeftKey)
    .function("handleRightKey",&JRavelCairo::handleRightKey)
    .function("setReductionOp",&JRavelCairo::setReductionOp)
    .function("setSort",&JRavelCairo::setSort)
    .function("setDisplayFilter",&JRavelCairo::setDisplayFilter)
    .constructor<>()
    ;

  class_<RawDataIdx>("RawDataIdx")
    .function("size",&RawData::size)
    .function("rank",&RawData::rank)
    ;

  class_<RawData,base<RawDataIdx>>("RawData");

  class_<JSRawData,base<RawData>>("JSRawData")
    .function("val",&JSRawData::val)
    ;

  register_vector<size_t>("VectorSizet");
  register_vector<string>("VectorString");
  
  register_vector<Labels>("LabelsVector");
  register_vector<double>("DoubleVector");
  register_vector<AxisSlice>("Key");
}


