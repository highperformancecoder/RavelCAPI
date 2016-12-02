#include "cairoShim.h"
#include "HTMLCanvas.h"
#include <emscripten/bind.h>

using namespace ravel;
using namespace emscripten;

class TestCShim: public CairoShim<HTMLCanvas>
{
public:
  TestCShim(const std::string& x): CairoShim<HTMLCanvas>(HTMLCanvas(x)) {}
};

EMSCRIPTEN_BINDINGS(TestModule) {
  class_<CairoShim<HTMLCanvas>>("CairoShimHTMLCanvas")
    .function("moveTo",&TestCShim::moveTo)
    .function("lineTo",&CairoShim<HTMLCanvas>::lineTo)
    .function("stroke",&CairoShim<HTMLCanvas>::stroke)
    ;

  class_<TestCShim,base<CairoShim<HTMLCanvas>>>("CairoShim")
    .constructor<std::string>()
    ;
}
