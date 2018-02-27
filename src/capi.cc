#include "capi.h"
#include "ravelCairo.h"
using namespace ravel;

extern "C" void* ravel_new(size_t rank)
{
  auto r=new RavelCairo<cairo_t*>;
  for (size_t i=0; i<rank; ++i)
    r->handleIds.push_back(i);
  return r;
}

extern "C" void ravel_delete(void* ravel)
{
  delete static_cast<RavelCairo<cairo_t*>*>(ravel);
}

