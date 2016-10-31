#include "ravel.h"
#include <emscripten/bind.h>

using namespace ravel;
using namespace emscripten;

namespace {

  struct RavelWrapper: public wrapper<Ravel> {
    EMSCRIPTEN_WRAPPER(RavelWrapper);
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
    .allow_subclass<RavelWrapper>("RavelWrapper")
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

  register_vector<std::string>("VectorString");
}


