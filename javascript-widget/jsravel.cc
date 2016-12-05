#include "ravelCairo.h"
#include "ravelCairoImpl.h"
#include <string>
#include <emscripten/bind.h>


using namespace std;
using namespace ravel;
using namespace emscripten;

namespace ravel
{
  template class RavelCairo<val*>;
}

namespace {

  struct JRavelCairo: public RavelCairo<val*>
  {
    unique_ptr<val> canvasContext;
    void setCanvas(const val& x) { canvasContext.reset(new val(x)); setG(canvasContext.get());}
  };
  
  struct RavelCairoWrapper: public wrapper<JRavelCairo> {
    EMSCRIPTEN_WRAPPER(RavelCairoWrapper);
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
    .function("reductionDescription",&Handle::reductionDescription)
    //.function("sliceLabels",&Handle::sliceLabels)
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

    class_<JRavelCairo,base<RavelCairo<val*>>>("RavelCairo")
      .allow_subclass<RavelCairoWrapper>("RavelCairoWrapper")
      .function("setCanvas",&JRavelCairo::setCanvas)
      .constructor<>()
      ;
    
 register_vector<string>("VectorString");
}


