// Windows GDI+ implementation. G=HDC. GDI+ is used because it
// supports arbitrary rotations
#include "cairoShim.h"
#include <windows.h>
#include <gdiplus.h>
#include <vector>
using namespace std;
using namespace Gdiplus;
// generation of assignment operators
#pragma warning(disable:4512)
#include <boost/locale.hpp>
using boost::locale::conv::utf_to_utf;

// conversion of double to float
#pragma warning(disable:4244)

namespace ravel
{
  namespace 
  {
    const double degrees = 180 / 3.1415927;
    // stores state for save/restore functionality
    // MS's fucked up API - there's no reason why Pens and Fonts can't be copiable
    // implemented here as a type of unique_ptr smart ptr
    class Context
    {
      Pen* m_pen;
      Brush* m_brush;
      Font* m_font;
    public:
      GraphicsState g;
      double sx=1, sy=1;
      Context(HDC dc=0) {
        Color black(0,0,0);
        m_pen=new Pen(black,2);
        m_brush=new SolidBrush(black);
        m_font=dc? new Font(dc): nullptr;
      }
      ~Context() {
        delete m_pen;
        delete m_brush;
        delete m_font;
      }
      Context(const Context& x): 
        m_pen(x.m_pen->Clone()), m_brush(x.m_brush->Clone()), 
        m_font(x.m_font? x.m_font->Clone(): nullptr),
        g(x.g), sx(x.sx), sy(x.sy)
      {}
      Context& operator=(Context& x)
      {
        delete m_pen;
        m_pen=x.m_pen->Clone();
        delete m_brush;
        m_brush=x.m_brush->Clone();
        delete m_font;
        m_font=x.m_font? x.m_font->Clone(): nullptr;
        g=x.g; sx=x.sx; sy=x.sy;
      }
      Pen* pen() const {return m_pen;}
      Brush* brush() const {return m_brush;}
      void setBrush(Brush* b) {
        delete m_brush;
        m_brush=b;
        m_pen->SetBrush(b);
      }
      Font* font() const {return m_font;}
      void scale(double x, double y) {sx*=x; sy*=y;}
    };
  }

  template <> class CairoShimImpl<HDC>
  {
  public:
    Graphics gc;
    vector<Context> contextStack;
    GraphicsPath path; //current path
    PointF currPos; // current pen position
    RectF textExtents;
    CairoShimImpl(HDC dc): gc(dc), contextStack(1,Context(dc))
    {
      gc.SetSmoothingMode(SmoothingModeAntiAlias);
    }
  };

  template <> CairoShim<HDC>::CairoShim(HDC dc) :
    impl(new CairoShimImpl<HDC>(dc))
  {}

  template <> CairoShim<HDC>::~CairoShim() { delete impl; }

  template <> void CairoShim<HDC>::moveTo(double x, double y)
  {
    impl->currPos=PointF(x,y);
  }
  template <> void CairoShim<HDC>::lineTo(double x, double y)
  {
    impl->path.AddLine(impl->currPos, PointF(x,y));
    impl->currPos.X=x; impl->currPos.Y=y;
  }
  template <> void CairoShim<HDC>::relLineTo(double x, double y)
  {lineTo(impl->currPos.X+x, impl->currPos.Y+y);}
  template <> void CairoShim<HDC>::relMoveTo(double x, double y)
  {
    moveTo(impl->currPos.X + x, impl->currPos.Y + y);
  }
  template <> void CairoShim<HDC>::arc
  (double x, double y, double radius, double start, double end)
  {
    impl->path.AddArc((REAL)(x-radius), -radius + y, 2 * radius, 2 * radius, start*degrees, (end-start)*degrees);
  }
  
  template <> void CairoShim<HDC>::setLineWidth(double w)
  {
    // this doesn't map cairo's model exactly, as pens can have
    // different widths in the x & y directions
    impl->contextStack.back().pen()->SetWidth
      (w*max(impl->contextStack.back().sx,impl->contextStack.back().sy));
  }

  template <> void CairoShim<HDC>::newPath()
  {impl->path.Reset();}

  template <> void CairoShim<HDC>::closePath()
  {impl->path.CloseFigure();}

  template <> void CairoShim<HDC>::strokePreserve()
  {
    impl->gc.DrawPath(impl->contextStack.back().pen(), &impl->path);
  }
  template <> void CairoShim<HDC>::stroke()
  {
    strokePreserve();
    impl->path.Reset();
  }
  template <> void CairoShim<HDC>::fill()
  {
    impl->path.CloseAllFigures();
    //save();
    //setLineWidth(1);
    //strokePreserve();
    impl->gc.FillPath(impl->contextStack.back().brush(), &impl->path);
    //restore();
    impl->path.Reset();
  }

  namespace 
  {
    // clamp r,g,b values, otherwise 1.0 => 256 => 0!!
    void clamp(double& x)
    {
      x=min(x, 0.996);
    }
  }

  template <> void CairoShim<HDC>::setSourceRGB(double r, double g, double b)
  {
    clamp(r);
    clamp(g);
    clamp(b);
    impl->contextStack.back().setBrush(new SolidBrush(Color(255*r,255*g,255*b)));
  }

 template <> void CairoShim<HDC>::showText(const string& text)
  {
    // GDI anchors to top left, we need to emulate Cairo's bottom left
    PointF pos = impl->currPos;
    setTextExtents(text);
    pos.Y -= textHeight();
    impl->gc.DrawString(utf_to_utf<WCHAR>(text).c_str(),-1,
                        impl->contextStack.back().font(),pos, 
                        impl->contextStack.back().brush());
  }

  template <> void CairoShim<HDC>::setTextExtents(const std::string& text)
  {
    basic_string<WCHAR> t=utf_to_utf<WCHAR>(text);
    RectF layoutRect;
    impl->gc.GetClipBounds(&layoutRect);
    impl->gc.MeasureString(t.c_str(), -1, impl->contextStack.back().font(), layoutRect,
                           &impl->textExtents);
  }
  
  template <> double CairoShim<HDC>::textWidth() const 
  {return impl->textExtents.Width;}
  template <> double CairoShim<HDC>::textHeight() const 
  {return impl->textExtents.Height;}

  template <> void CairoShim<HDC>::identityMatrix()
  {
    Matrix identity;
    // MSDN does say whether ownership is passed here. I'm guessing not.
    impl->gc.SetTransform(&identity);
    impl->contextStack.back().sx=1;
    impl->contextStack.back().sy=1;
  }

  template <> void CairoShim<HDC>::translate(double x, double y)
  {impl->gc.TranslateTransform(x,y);}

  template <> void CairoShim<HDC>::scale(double sx, double sy)
  {
    impl->gc.ScaleTransform(sx, sy);
    impl->contextStack.back().scale(sx,sy);
  }

  template <> void CairoShim<HDC>::rotate(double angle)
  {
    impl->gc.RotateTransform(angle*degrees);
  }

  template <> void CairoShim<HDC>::save()
  {
    impl->contextStack.push_back(impl->contextStack.back());
    impl->contextStack.back().g=impl->gc.Save();
  }
  template <> void CairoShim<HDC>::restore()
  {
    impl->gc.Restore(impl->contextStack.back().g);
    impl->contextStack.pop_back();
  }

}


