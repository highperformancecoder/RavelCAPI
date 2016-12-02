#ifndef HTMLCANVAS
#define HTMLCANVAS
#include <string>
#include <emscripten/emscripten.h>

namespace ravel
{
  struct HTMLCanvas
  {
    int id;
    HTMLCanvas(const std::string& canvasId) {
      emscripten_run_script(("HTMLcanvas.newCanvas('"+canvasId+"')").c_str());
      id=EM_ASM_INT_V({return HTMLcanvas.length})-1;
   }
  };
}

#endif
