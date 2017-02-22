#ifndef CONSOLELOG_H
#define CONSOLELOG_H

#include <emscripten/emscripten.h>
#include <sstream>

namespace ravel
{
  inline void log(const std::string& x)
  {
    emscripten_run_script(("console.log('"+x+"')").c_str());
  }

  // convenient iostream interface to the console log 
  struct ConsoleLog: public std::ostringstream {};
  extern ConsoleLog console;
  struct ConsoleLogEndl {};
  extern ConsoleLogEndl endl;
  inline ConsoleLog& operator<<(ConsoleLog& x, ConsoleLogEndl)
  {log(x.str()); x.str(""); return x;}
  template <class T>
  inline ConsoleLog& operator<<(ConsoleLog& x, const T& y)
  {static_cast<std::ostringstream&>(x)<<y; return x;}

}
#endif
