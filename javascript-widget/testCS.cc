#include "cairoShim.h"
#include <emscripten/bind.h>

using namespace ravel;
using namespace emscripten;

class TestCShim: public CairoShim<val*>
{
public:
  TestCShim(const std::string& x): CairoShim<val*>(HTMLCanvas(x)) {}
};

EMSCRIPTEN_BINDINGS(TestModule) {
  class_<CairoShim<HTMLCanvas>>("CairoShimHTMLCanvas")
    .function("moveTo",&TestCShim::moveTo)
    .function("lineTo",&CairoShim<val*>::lineTo)
    .function("stroke",&CairoShim<val*>::stroke)
    ;

  class_<TestCShim,base<CairoShim<HTMLCanvas>>>("CairoShim")
    .constructor<std::string>()
    ;
}
