#include "ravelCairo.h"
#include "dataCube.h"
#include "ravelCairoImpl.h"
#include <string>
#include <emscripten/emscripten.h>
#include <emscripten/bind.h>


using namespace std;
using namespace ravel;
using namespace emscripten;

namespace ravel
{
  template class RavelCairo<val*>;
}

namespace {

  struct JSDataCube: public DataCube
  {
    /// assign a Javascript function to this value
    val dataCallback=val::null();
    void setDataElement(size_t col, size_t row, double v) override
    {dataCallback(col,row,v);}
    
    void loadData(const std::string& jsonData) {
      //      emscripten_run_script(("alert("+jsonData+")").c_str());;
      if (jsonData.empty()) return;
      size_t i=0;
      const char* p=jsonData.c_str();
      if (p) p++; // skip leading '['
      //EM_ASM_ARGS(alert($0),rawData.size());
      while (*p && i<rawData.size())
        {
          char* p1;
          double v=strtod(p,&p1);
          if (!isnan(v))
            EM_ASM_ARGS(alert('i='+$0+'v='+$1),i,v);
          rawData[i++] = p1>p? v: nan("");
          p=p1;
          while (*p && *p!=',') p++;
          if (*p) p++; //skip comma
        }
      // fill any tail with NaNs
      for (; i<rawData.size(); ++i) rawData[i++] = nan("");
    }

    // set the dimensions of this datacube
    void dimension(const LabelsVector& lv) {
      rawData=RawDataIdx(lv);
    }
  };
  
  struct JRavelCairo: public RavelCairo<val*>
  {
    unique_ptr<val> canvasContext;
    void setCanvas(const val& x) { canvasContext.reset(new val(x)); setG(canvasContext.get());}
    JSDataCube dc;
    void populateData() {dc.populateArray(*this);}
    /// dimension the datacube according to info in Ravel
    void dimension() {
      LabelsVector lv;
      for (auto& h: handles)
        {
          vector<string> labels;
          for (auto& j: h.sliceLabels) labels.push_back(j);
          lv.emplace_back(h.description, labels);
        }
      dc.dimension(lv);
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
    .function("handles",optional_override([](const Ravel& self, size_t i){return self.handles[i];}))
    .function("numHandles",optional_override([](const Ravel& self){return self.handles.size();}))
    .function("addHandle",&Ravel::addHandle)
    .function("clear",&Ravel::clear)
    .function("moveHandleTo",&Ravel::moveHandleTo)
    .function("rescale",&Ravel::rescale)
    .function("onMouseMotion",&Ravel::onMouseMotion)
    .function("onMouseDown",&Ravel::onMouseDown)
    .function("onMouseUp",&Ravel::onMouseUp)
    .function("handleIfMouseOver",&Ravel::handleIfMouseOver)
    .function("handleX",&Ravel::handleX)
    .function("handleY",&Ravel::handleY)
    .function("xHandleId",&Ravel::xHandleId)
    .function("yHandleId",&Ravel::yHandleId)
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
    .function("populateData",&JRavelCairo::populateData)
    .function("dimension",&JRavelCairo::dimension)
    .constructor<>()
    ;

  register_vector<string>("VectorString");
  
  register_vector<Labels>("LabelsVector");
  register_vector<double>("DoubleVector");
}


