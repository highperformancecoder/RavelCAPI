#include "ravel.h"
#include <emscripten/bind.h>

using namespace ravel;
using namespace emscripten;

EMSCRIPTEN_BINDINGS(Ravel) {
  class_<Ravel>("Ravel")
    .constructor<>()
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


