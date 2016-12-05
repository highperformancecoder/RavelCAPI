#include "ravelCairo.h"
#include "ravelCairoImpl.h"
//#include "HTMLCanvas.h"
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

  typedef val* HTMLCanvas; //TODO remove later
  
  struct JRavelCairo: public RavelCairo<val*>
  {
    unique_ptr<val> canvasContext;
    void setCanvas(const val& x) { canvasContext.reset(new val(x)); setG(canvasContext.get());}
    void render() {
      canvasContext->call<void>("clearRect",-radius(),-radius(),2*radius(),2*radius());
      RavelCairo<val*>::render();
    }
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
    .function("labelAnchor",&Handle::labelAnchor);
  
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
    .function("handleX",&Ravel::handleX)
    .function("handleY",&Ravel::handleY);

  class_<RavelCairo<HTMLCanvas>,base<Ravel>>("RavelCairoHTMLCanvas")
    .function("onMouseDown",&RavelCairo<HTMLCanvas>::onMouseDown)
    .function("onMouseOver",&RavelCairo<HTMLCanvas>::onMouseOver)
    .function("handleIfMouseOverAxisLabel",&RavelCairo<HTMLCanvas>::handleIfMouseOverAxisLabel)
    .function("handleIfMouseOverOpLabel",&RavelCairo<HTMLCanvas>::handleIfMouseOverOpLabel)
    .function("handleIfMouseOverCaliperLabel",
              &RavelCairo<HTMLCanvas>::handleIfMouseOverCaliperLabel)
    .function("render",&RavelCairo<HTMLCanvas>::render)
    ;

    class_<JRavelCairo,base<RavelCairo<val*>>>("JRavelCairo")
      .allow_subclass<RavelCairoWrapper>("RavelCairoWrapper")
      .function("setCanvas",&JRavelCairo::setCanvas)
      .function("render",&JRavelCairo::render)
      .constructor<>()
      ;
    
 register_vector<string>("VectorString");
}


