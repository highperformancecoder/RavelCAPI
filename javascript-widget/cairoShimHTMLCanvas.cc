#include "cairoShim.h"
#include <emscripten/bind.h>
using namespace emscripten;
using namespace std;

namespace
{
  //snaffled from some random guy in StackOverflow. TODO - treatment of exceptions?
  std::wstring utf8_to_utf16(const std::string& utf8)
  {
    std::vector<unsigned long> unicode;
    size_t i = 0;
    while (i < utf8.size())
      {
        unsigned long uni;
        size_t todo;
        bool error = false;
        unsigned char ch = utf8[i++];
        if (ch <= 0x7F)
          {
            uni = ch;
            todo = 0;
          }
        else if (ch <= 0xBF)
          {
            throw std::logic_error("not a UTF-8 string");
          }
        else if (ch <= 0xDF)
          {
            uni = ch&0x1F;
            todo = 1;
          }
        else if (ch <= 0xEF)
          {
            uni = ch&0x0F;
            todo = 2;
          }
        else if (ch <= 0xF7)
          {
            uni = ch&0x07;
            todo = 3;
          }
        else
          {
            throw std::logic_error("not a UTF-8 string");
          }
        for (size_t j = 0; j < todo; ++j)
          {
            if (i == utf8.size())
              throw std::logic_error("not a UTF-8 string");
            unsigned char ch = utf8[i++];
            if (ch < 0x80 || ch > 0xBF)
              throw std::logic_error("not a UTF-8 string");
            uni <<= 6;
            uni += ch & 0x3F;
          }
        if (uni >= 0xD800 && uni <= 0xDFFF)
          throw std::logic_error("not a UTF-8 string");
        if (uni > 0x10FFFF)
          throw std::logic_error("not a UTF-8 string");
        unicode.push_back(uni);
      }
    std::wstring utf16;
    for (size_t i = 0; i < unicode.size(); ++i)
      {
        unsigned long uni = unicode[i];
        if (uni <= 0xFFFF)
          {
            utf16 += (wchar_t)uni;
          }
        else
          {
            uni -= 0x10000;
            utf16 += (wchar_t)((uni >> 10) + 0xD800);
            utf16 += (wchar_t)((uni & 0x3FF) + 0xDC00);
          }
      }
    return utf16;
  }
}

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
  {
    // clipping helps with chrome rendering bugs, but only in v59 and higher 
    //    impl->canvas.call<void>("save");
    //    impl->canvas.call<void>("clip");
    impl->canvas.call<void>("fill");
    //    impl->canvas.call<void>("restore");
    newPath();
  }

  template <> void CairoShim<val*>::clip()
  {
    impl->canvas.call<void>("clip");
    newPath();
  }

  template <> void CairoShim<val*>::stroke()
  {impl->canvas.call<void>("stroke");newPath();}
  
  template <> void CairoShim<val*>::strokePreserve()
  {impl->canvas.call<void>("stroke");}

  template <> void CairoShim<val*>::setLineWidth(double w)
  {impl->canvas.set("lineWidth",w);}

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
    impl->canvas.call<void>("fillText",utf8_to_utf16(text),impl->currX,impl->currY);
  }

  template <> void CairoShim<val*>::setTextExtents(const std::string& text)
  {
    val textExtents=impl->canvas.call<val>("measureText",text);
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
