#include "ravelCairoImpl.h"
using namespace ravel;

#ifdef USE_GDI
#include <windows.h>
template class RavelCairo<HDC>;

#else
#define CAIRO_WIN32_STATIC_BUILD
#include <cairo.h>
#undef CAIRO_WIN32_STATIC_BUILD

template class RavelCairo<cairo_t*>;
#endif
