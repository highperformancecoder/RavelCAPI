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
  class_<Ravel>("Ravel")
    .constructor<>()
    .allow_subclass<RavelWrapper>("RavelWrapper")
    .property("x",&Ravel::x)
    .property("y",&Ravel::y)
    .function("addHandle",&Ravel::addHandle)
    .function("moveHandleTo",&Ravel::moveHandleTo)
    .function("rescale",&Ravel::rescale)
    .function("onMouseMotion",&Ravel::onMouseMotion)
    .function("onMouseDown",&Ravel::onMouseDown)
    .function("onMouseUp",&Ravel::onMouseUp)
    .function("handleX",&Ravel::handleX)
    .function("handleY",&Ravel::handleY);
  register_vector<std::string>("VectorString");
}


