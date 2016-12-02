#include "cairoShim.h"
#include "HTMLCanvas.h"

using namespace std;

namespace ravel
{
  template <> class CairoShimImpl<HTMLCanvas>: public HTMLCanvas
  {
    bool pathOpen=false;
  public:
    void beginPath()
    {
      if (!pathOpen)
        EM_ASM_ARGS(HTMLcanvas[$0].beginPath(),id);
      pathOpen=true;
    }
    void endPath() {pathOpen=false;}
    CairoShimImpl(const HTMLCanvas& canvas): HTMLCanvas(canvas) {}
  };

  template <> CairoShim<HTMLCanvas>::CairoShim(HTMLCanvas canvas): 
    impl(new CairoShimImpl<HTMLCanvas>(canvas)) {}
  template <> CairoShim<HTMLCanvas>::~CairoShim()
  {delete impl;}

  template <> void CairoShim<HTMLCanvas>::moveTo(double x, double y) 
  {EM_ASM_ARGS(HTMLcanvas[$0].moveTo($1,$2),impl->id,x,y);}
  
  template <> void CairoShim<HTMLCanvas>::lineTo(double x, double y)
  {EM_ASM_ARGS(HTMLcanvas[$0].lineTo($1,$2),impl->id,x,y);}

  template <> void CairoShim<HTMLCanvas>::relMoveTo(double x, double y)
  {}

  template <> void CairoShim<HTMLCanvas>::relLineTo(double x, double y)
  {}

  template <> void CairoShim<HTMLCanvas>::arc
  (double x, double y, double radius, double start, double end)
  {}

    // paths
  template <> void CairoShim<HTMLCanvas>::newPath()
  {}

  template <> void CairoShim<HTMLCanvas>::closePath()
  {EM_ASM_ARGS(HTMLcanvas[$0].closePath(),impl->id);}

  template <> void CairoShim<HTMLCanvas>::fill()
  {}

  template <> void CairoShim<HTMLCanvas>::stroke()
  {EM_ASM_ARGS(HTMLcanvas[$0].stroke(),impl->id); impl->endPath();}
  template <> void CairoShim<HTMLCanvas>::strokePreserve()
  {}

  template <> void CairoShim<HTMLCanvas>::setLineWidth(double w)
  {}

  // sources
  template <> void CairoShim<HTMLCanvas>::setSourceRGB
  (double r, double g, double b)
  {}

  // text. Argument is in UTF8 encoding
  template <> void CairoShim<HTMLCanvas>::showText(const std::string& text)
  {}

  template <> void CairoShim<HTMLCanvas>::setTextExtents(const std::string& text)
  {}

  template <> double CairoShim<HTMLCanvas>::textWidth() const
  {}

  template <> double CairoShim<HTMLCanvas>::textHeight() const
  {}

  // matrix transformation
  template <> void CairoShim<HTMLCanvas>::identityMatrix()
  {}
  
  template <> void CairoShim<HTMLCanvas>::translate(double x, double y)
  {}

  template <> void CairoShim<HTMLCanvas>::scale(double sx, double sy)
  {}

  template <> void CairoShim<HTMLCanvas>::rotate(double angle)
  {}

    // context manipulation
  template <> void CairoShim<HTMLCanvas>::save()
  {}

  template <> void CairoShim<HTMLCanvas>::restore()
  {}


}
