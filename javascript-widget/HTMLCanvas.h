#ifndef HTMLCANVAS
#define HTMLCANVAS
#include <string>
#include <emscripten/emscripten.h>
#include <emscripten/val.h>

namespace ravel
{
  struct HTMLCanvas
  {
    int id=-1;
    HTMLCanvas(void* p=nullptr) {}
    HTMLCanvas(int id): id(id) {}
    operator bool() const {return id>=0;}
  };
}

#endif
