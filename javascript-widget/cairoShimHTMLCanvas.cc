#include "cairoShim.h"
#include <emscripten/bind.h>
using namespace emscripten;
using namespace std;

namespace ravel
{
  typedef val* HTMLCanvas;
  template <> class CairoShimImpl<HTMLCanvas>
  {
    bool pathOpen=false;
  public:
    val& canvas;
    double currX, currY;
    double textWidth, textHeight;
    CairoShimImpl(val* canvas): canvas(*canvas) {}
  };

  template <> CairoShim<HTMLCanvas>::CairoShim(HTMLCanvas canvas): 
    impl(new CairoShimImpl<HTMLCanvas>(canvas)) {}
  template <> CairoShim<HTMLCanvas>::~CairoShim()
  {delete impl;}

  template <> void CairoShim<HTMLCanvas>::moveTo(double x, double y) 
  {
    impl->canvas.call<void>("moveTo",x,y);
    impl->currX=x; impl->currY=y;
  }
  
  template <> void CairoShim<HTMLCanvas>::lineTo(double x, double y)
  {
    impl->canvas.call<void>("lineTo",x,y);
    impl->currX=x; impl->currY=y;
  }

  template <> void CairoShim<HTMLCanvas>::relMoveTo(double x, double y)
  {
    impl->canvas.call<void>("moveTo",x+impl->currX, y+impl->currY);
    impl->currX=x; impl->currY=y;
  }

  template <> void CairoShim<HTMLCanvas>::relLineTo(double x, double y)
  {
    impl->canvas.call<void>("lineTo",x+impl->currX, y+impl->currY);
    impl->currX=x; impl->currY=y;
  }

  template <> void CairoShim<HTMLCanvas>::arc
  (double x, double y, double radius, double start, double end)
  {
    impl->canvas.call<void>("arc",x,y,radius,start,end,false);
  }

    // paths
  template <> void CairoShim<HTMLCanvas>::newPath()
  {impl->canvas.call<void>("beginPath");}

  template <> void CairoShim<HTMLCanvas>::closePath()
  {impl->canvas.call<void>("closePath");}

  template <> void CairoShim<HTMLCanvas>::fill()
  {impl->canvas.call<void>("fill");}

  template <> void CairoShim<HTMLCanvas>::stroke()
  {impl->canvas.call<void>("stroke");}
  
  template <> void CairoShim<HTMLCanvas>::strokePreserve()
  {}

  template <> void CairoShim<HTMLCanvas>::setLineWidth(double w)
  {}

  // sources
  template <> void CairoShim<HTMLCanvas>::setSourceRGB
  (double r, double g, double b)
  {
    char rgbSpec[50];
    snprintf(rgbSpec,sizeof(rgbSpec),"rgb(%d,%d,%d)",int(255*r),int(255*g),int(255*b));
    impl->canvas.set("fillStyle",rgbSpec);
  }

  // text. Argument is in UTF8 encoding
  template <> void CairoShim<HTMLCanvas>::showText(const std::string& text)
  {
    impl->canvas.call<void>("fillText",text,impl->currX,impl->currY);
  }

  template <> void CairoShim<HTMLCanvas>::setTextExtents(const std::string& text)
  {
    val textExtents=impl->canvas.call<val>("measureText",text);
    val document(val::global("document"));
    impl->textWidth=textExtents["width"].as<double>();
    impl->textHeight=10; // height not available in current browsers
  }

  template <> double CairoShim<HTMLCanvas>::textWidth() const
  {return impl->textWidth;}

  template <> double CairoShim<HTMLCanvas>::textHeight() const
  {return impl->textHeight;}

  // matrix transformation
  template <> void CairoShim<HTMLCanvas>::identityMatrix()
  {impl->canvas.call<void>("resetTransform");}
  
  template <> void CairoShim<HTMLCanvas>::translate(double x, double y)
  {impl->canvas.call<void>("translate",x,y);}

  template <> void CairoShim<HTMLCanvas>::scale(double sx, double sy)
  {impl->canvas.call<void>("scale",sx,sy);}


  template <> void CairoShim<HTMLCanvas>::rotate(double angle)
  {impl->canvas.call<void>("rotate",angle);}

    // context manipulation
  template <> void CairoShim<HTMLCanvas>::save()
  {impl->canvas.call<void>("save");}

  template <> void CairoShim<HTMLCanvas>::restore()
  {impl->canvas.call<void>("restore");}


}
