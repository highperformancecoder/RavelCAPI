#include "cairoShim.h"
#include <emscripten/bind.h>
using namespace emscripten;
using namespace std;

namespace ravel
{
  template <> class CairoShimImpl<val*>
  {
    bool pathOpen=false;
  public:
    val& canvas;
    double currX, currY;
    double textWidth, textHeight;
    CairoShimImpl(val* canvas): canvas(*canvas) {}
  };

  template <> CairoShim<val*>::CairoShim(val* canvas): 
    impl(new CairoShimImpl<val*>(canvas)) {}
  template <> CairoShim<val*>::~CairoShim()
  {delete impl;}

  template <> void CairoShim<val*>::moveTo(double x, double y) 
  {
    impl->canvas.call<void>("moveTo",x,y);
    impl->currX=x; impl->currY=y;
  }
  
  template <> void CairoShim<val*>::lineTo(double x, double y)
  {
    impl->canvas.call<void>("lineTo",x,y);
    impl->currX=x; impl->currY=y;
  }

  template <> void CairoShim<val*>::relMoveTo(double x, double y)
  {
    x+=impl->currX; y+=impl->currY;
    impl->canvas.call<void>("moveTo",x, y);
    impl->currX=x; impl->currY=y;
  }

  template <> void CairoShim<val*>::relLineTo(double x, double y)
  {
    x+=impl->currX; y+=impl->currY;
    impl->canvas.call<void>("lineTo",x, y);
    impl->currX=x; impl->currY=y;
  }

  template <> void CairoShim<val*>::arc
  (double x, double y, double radius, double start, double end)
  {
    impl->canvas.call<void>("arc",x,y,radius,start,end,false);
  }

    // paths
  template <> void CairoShim<val*>::newPath()
  {impl->canvas.call<void>("beginPath");}

  template <> void CairoShim<val*>::closePath()
  {impl->canvas.call<void>("closePath");}

  template <> void CairoShim<val*>::fill()
  {impl->canvas.call<void>("fill");newPath();}

  template <> void CairoShim<val*>::stroke()
  {impl->canvas.call<void>("stroke");newPath();}
  
  template <> void CairoShim<val*>::strokePreserve()
  {impl->canvas.call<void>("stroke");}

  template <> void CairoShim<val*>::setLineWidth(double w)
  {}

  // sources
  template <> void CairoShim<val*>::setSourceRGB
  (double r, double g, double b)
  {
    char rgbSpec[50];
    snprintf(rgbSpec,sizeof(rgbSpec),"rgb(%d,%d,%d)",int(255*r),int(255*g),int(255*b));
    impl->canvas.set("fillStyle",rgbSpec);
    impl->canvas.set("strokeStyle",rgbSpec);
  }

  // text. Argument is in UTF8 encoding
  template <> void CairoShim<val*>::showText(const std::string& text)
  {
    impl->canvas.call<void>("fillText",text,impl->currX,impl->currY);
  }

  template <> void CairoShim<val*>::setTextExtents(const std::string& text)
  {
    val textExtents=impl->canvas.call<val>("measureText",text);
    val document(val::global("document"));
    impl->textWidth=textExtents["width"].as<double>();
    impl->textHeight=10; // height not available in current browsers
  }

  template <> double CairoShim<val*>::textWidth() const
  {return impl->textWidth;}

  template <> double CairoShim<val*>::textHeight() const
  {return impl->textHeight;}

  // matrix transformation
  template <> void CairoShim<val*>::identityMatrix()
  {impl->canvas.call<void>("resetTransform");}
  
  template <> void CairoShim<val*>::translate(double x, double y)
  {impl->canvas.call<void>("translate",x,y);}

  template <> void CairoShim<val*>::scale(double sx, double sy)
  {impl->canvas.call<void>("scale",sx,sy);}


  template <> void CairoShim<val*>::rotate(double angle)
  {impl->canvas.call<void>("rotate",angle);}

    // context manipulation
  template <> void CairoShim<val*>::save()
  {impl->canvas.call<void>("save");}

  template <> void CairoShim<val*>::restore()
  {impl->canvas.call<void>("restore");}


}
