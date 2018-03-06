#include "cairoShim.h"
#include "cairoRenderer.h"
#include "ravelCairoImpl.h"

using namespace std;

namespace ravel
{
  template <> class CairoShimImpl<CAPIRenderer*>: public CAPIRenderer {};

  template <> CairoShim<CAPIRenderer*>::CairoShim(CAPIRenderer* r): 
    impl(static_cast<CairoShimImpl<CAPIRenderer*>*>(r)) {}
  template <> CairoShim<CAPIRenderer*>::~CairoShim() {}

  template <> void CairoShim<CAPIRenderer*>::moveTo(double x, double y) 
  {impl->moveTo(impl,x,y);}

  template <> void CairoShim<CAPIRenderer*>::lineTo(double x, double y)
  {impl->lineTo(impl,x,y);}

  template <> void CairoShim<CAPIRenderer*>::relMoveTo(double x, double y)
  {impl->relMoveTo(impl,x,y);}

  template <> void CairoShim<CAPIRenderer*>::relLineTo(double x, double y)
  {impl->relLineTo(impl,x,y);}

  template <> void CairoShim<CAPIRenderer*>::arc
  (double x, double y, double radius, double start, double end)
  {impl->arc(impl,x,y,radius,start,end);}

  template <> void CairoShim<CAPIRenderer*>::newPath()
  {impl->newPath(impl);}

  template <> void CairoShim<CAPIRenderer*>::closePath()
  {impl->closePath(impl);}

  template <> void CairoShim<CAPIRenderer*>::fill()
  {impl->fill(impl);}

  template <> void CairoShim<CAPIRenderer*>::stroke()
  {impl->stroke(impl);}
  template <> void CairoShim<CAPIRenderer*>::strokePreserve()
  {impl->strokePreserve(impl);}

  template <> void CairoShim<CAPIRenderer*>::setLineWidth(double w)
  {impl->setLineWidth(impl, w);}

  template <> void CairoShim<CAPIRenderer*>::setSourceRGB
  (double r, double g, double b)
  {impl->setSourceRGB(impl,r,g,b);}

  template <> void CairoShim<CAPIRenderer*>::showText(const std::string& text)
  {impl->showText(impl,text.c_str());}

  template <> void CairoShim<CAPIRenderer*>::setTextExtents(const std::string& text)
  {impl->setTextExtents(impl,text.c_str());}

  template <> double CairoShim<CAPIRenderer*>::textWidth() const
  {return impl->textWidth(impl);}

  template <> double CairoShim<CAPIRenderer*>::textHeight() const
  {return impl->textHeight(impl);}

  // matrix transformation
  template <> void CairoShim<CAPIRenderer*>::identityMatrix()
  {impl->identityMatrix(impl);}
  
  template <> void CairoShim<CAPIRenderer*>::translate(double x, double y)
  {impl->translate(impl,x,y);}

  template <> void CairoShim<CAPIRenderer*>::scale(double sx, double sy)
  {impl->scale(impl,sx,sy);}

  template <> void CairoShim<CAPIRenderer*>::rotate(double angle)
  {impl->rotate(impl,angle);}

    // context manipulation
  template <> void CairoShim<CAPIRenderer*>::save()
  {impl->save(impl);}

  template <> void CairoShim<CAPIRenderer*>::restore()
  {impl->restore(impl);}

  template class RavelCairo<CAPIRenderer*>;
}
