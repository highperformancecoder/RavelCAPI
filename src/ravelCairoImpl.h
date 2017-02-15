#ifndef RAVELCAIROIMPL_H
#define RAVELCAIROIMPL_H
#include "ravelCairo.h"
#include "cairoShim.h"
#include <math.h>
#include <assert.h>
#ifdef CLASSDESC
#include <classdesc_epilogue.h>
#endif
#ifdef ECOLAB_LIB
#include <ecolab_epilogue.h>
#endif

const double pi = 3.1415927;

namespace ravel
{
  namespace
  {
    inline double sqr(double x) {return x*x;}
    // distance squared between two points
    inline double dsq(double x0, double y0, double x1, double y1)
    {return sqr(x1-x0)+sqr(y1-y0);}

    double palette[][3] = 
      {
        {0,0,0},
        {1,0,0},
        {0,0.5,0},
        {0,0,1},
        {1,0,1},
        {0,1,1},
        {1,0.64,0}, //orange
        {0.62,0.12,0.93} //purple
      };

    template <class G>
    void drawArrow(G& gc)
    {
      gc.moveTo(0,0.577);
      gc.relLineTo(-0.5,-0.866);
      gc.relLineTo(1,0);
      gc.closePath();
      gc.fill();
    }

    // draw line with arrow from current position to x,y
    template <class G>
    void drawLine(G& gc, double x, double y)
    {
      gc.save();
      gc.rotate(atan2(y,x));
      double r=sqrt(x*x+y*y);
      double f=20; //smaller means more curved
      gc.newPath();
      gc.arc(0,-f*r,f*r,.5*pi-atan(1/f),.5*pi);
      gc.arc(0,f*r,f*r,1.5*pi,1.5*pi+atan(1/f));
      gc.closePath();
      gc.fill();
      gc.rotate(-atan2(y,x));
      gc.translate(x,y);
      gc.rotate(atan2(-x,y));
      gc.scale(8,8);
      drawArrow(gc);
      gc.restore();
    }

    // a rotator mark is a curved arrow indicating handles can be rotated
    template <class G>
    void drawRotatorMark(G& gc,double x,double y, double dir)
    {
      dir/=fabs(dir); //ensure dir = +/-1
      gc.save();
      gc.setLineWidth(0.5);
      gc.rotate(atan2(y,x));
      double r=sqrt(x*x+y*y);
      double theta=0.05; // amount of arc to show
      if (dir>0)
        gc.arc(0,0,r,theta,2*theta);
      else
        gc.arc(0,0,r,-2*theta,-theta);
      gc.stroke();
      gc.translate(r*cos(2*theta),dir*r*sin(2*theta));
      gc.rotate(0.5*(1-dir)*pi + 2*dir*theta);
      gc.scale(3,3);
      drawArrow(gc);
      gc.fill();
      gc.restore();
    }

    template <class G>
    void drawCaliper(G& gc,double sf,double hx,double hy, double sx, double sy,
                     const std::string& label)
    {
      gc.save();
      gc.translate(sx,sy);
      gc.rotate(atan2(-hx,hy));
      gc.moveTo(0,0);
      gc.lineTo(Handle::caliperLength,0);
      gc.setLineWidth(1/sf);
      gc.stroke();
      gc.scale(0.5,0.5);
      gc.showText(label);
      gc.restore();
    }
  }

  template <class G>
  void RavelCairo<G>::render() const
  {
    if (!g) return;
    CairoShim<G> gc(g);
    gc.save();
    double sf=0.01*radius();
    gc.scale(sf,sf);
    for (unsigned i=0; i<handles.size(); ++i)
      {
        const Handle& h=handles[i];
        double hx=h.x()/sf, hy=h.y()/sf;

        gc.setSourceRGB(palette[i][0],palette[i][1],palette[i][2]);
                                           
        drawLine(gc, hx,hy);

        // draw indicators of sorting order, and filter calipers
        if (isOutputHandle(i) && !h.collapsed())
          {
            if (h.sliceLabels.order()!=HandleSort::none)
              {
                gc.save();
                gc.translate(0.1*hx,0.1*hy);
                double angle=atan2(-hx,hy);
                if (h.sliceLabels.orderReversed())
                  angle+=pi;
                gc.rotate(angle);
                gc.scale(5,5);
                drawArrow(gc);
                gc.restore();
              }
            if (h.displayFilterCaliper)
              {
                drawCaliper(gc,sf,hx,hy,h.minSliceX()/sf,h.minSliceY()/sf,
                            h.minSliceLabel());
                drawCaliper(gc,sf,hx,hy,h.maxSliceX()/sf,h.maxSliceY()/sf,
                            h.maxSliceLabel());
              }
          }
        

        AnchorPoint ap=h.labelAnchor();
        if (ap.anchor==AnchorPoint::se)
          {
            // in bottom quadrant, anchor at south east
            gc.setTextExtents(h.description);
            gc.moveTo(ap.x/sf-gc.textWidth(), ap.y/sf);
          }
        else
          gc.moveTo(ap.x/sf, ap.y/sf);
        gc.showText(h.description);

        if (toolTipHandle==int(i))
          {
            // draw motion indicators on the handle
            gc.save();
            gc.setSourceRGB(1,1,1);
            gc.setLineWidth(0.5);
            gc.moveTo(hx,hy);
            gc.lineTo(0.95*hx,0.95*hy);
            gc.stroke();
            gc.translate(0.95*hx,0.95*hy);
            gc.rotate(atan2(hx,-hy));
            gc.scale(2,2);
            drawArrow(gc);
            gc.restore();

            drawRotatorMark(gc,hx,hy,1);
            drawRotatorMark(gc,hx,hy,-1);
          
          }

        if (h.collapsed())
          {
            // show reduction operations
            assert(h.reductionOp < sizeof(opLabels)/sizeof(opLabels[0]));
            gc.setTextExtents(opLabels[h.reductionOp]);
        
            gc.moveTo(h.opX()/sf-gc.textWidth(), h.opY()/sf);
          
            gc.showText(opLabels[h.reductionOp]);
          }

        // slice control
        if (!h.collapsed() && !isOutputHandle(i))
          {
            gc.newPath();
            gc.arc(h.sliceX()/sf, h.sliceY()/sf, 4, 0, 2*pi);
            gc.fill();
          
            gc.moveTo(h.sliceX()/sf+2, h.sliceY()/sf-2);
            gc.showText(h.sliceLabel().c_str());
          }
      }
    gc.restore();
  }

  template <class G>
  void RavelCairo<G>::onMouseDown(double xx, double yy)
  {
    lastHandle=handleIfMouseOverCaliperLabel(xx,yy);
    if (lastHandle==-1)
      Ravel::onMouseDown(xx,yy);
    else 
      {
        Handle& h=handles[lastHandle];
        // select which caliper we're moving
        elementMoving = 
          dsq(xx-x,yy-y,h.minSliceX(),h.minSliceY())<dsq(xx-x,yy-y,h.maxSliceX(),h.maxSliceY())?
                                                     filterMin: filterMax;
      }
  }

  template <class G>
  bool RavelCairo<G>::onMouseOver(double xx, double yy)
  {
    int oldTTHandle = toolTipHandle;
    toolTipHandle=handleIfMouseOver(xx-x,yy-y);
    return toolTipHandle != oldTTHandle;
  }

  template <class G>
  static bool ptInText(G& gc, const std::string& text, const AnchorPoint& a, double x, double y, double sf)
  {
    gc.setTextExtents(text);
    switch (a.anchor)
      {
      case AnchorPoint::se:
        if (x<=a.x && x>=a.x-sf*gc.textWidth() &&
            y<=a.y && y>=a.y-sf*gc.textHeight())
          return true;
        break;
      case AnchorPoint::sw:
        if (x>=a.x && x<=a.x+sf*gc.textWidth() &&
            y<=a.y && y>=a.y-sf*gc.textHeight())
          return true;
        break;
      case AnchorPoint::ne:
        if (x>=a.x && x<=a.x+sf*gc.textWidth() &&
            y>=a.y && y<=a.y+sf*gc.textHeight())
          return true;
        break;
      case AnchorPoint::nw:
        if (x<=a.x && x>=a.x-sf*gc.textWidth() &&
            y>=a.y && y<=a.y+sf*gc.textHeight())
          return true;
        break;
      }
    return false;
  }

  template<class G>
  int  RavelCairo<G>::handleIfMouseOverAxisLabel(double x, double y) const
  {
    CairoShim<G> gc(g);
    x-=this->x; y-=this->y;
    double sf=0.01*radius();
    for (unsigned h=0; h<handles.size(); ++h)
      {
        const Handle& hnd=handles[h];
        if (ptInText(gc,hnd.description,hnd.labelAnchor(),x,y,sf))
          return h;
      }
    return -1;
  }

  template<class G>
  int RavelCairo<G>::handleIfMouseOverOpLabel(double x, double y) const
  {
    CairoShim<G> gc(g);
    x-=this->x; y-=this->y;
    double sf=0.01*radius();
    for (unsigned h=0; h<handles.size(); ++h)
      {
        const Handle& hnd=handles[h];
        if (ptInText(gc,opLabels[hnd.reductionOp],
                     AnchorPoint{hnd.opX(),hnd.opY(), AnchorPoint::se},
                     x,y,sf))
          return h;
      }
    return -1;
  }

  template<class G>
  int RavelCairo<G>::handleIfMouseOverCaliperLabel(double x, double y) const
  {
    CairoShim<G> gc(g);
    x-=this->x; y-=this->y;
    double sf=0.01*radius();
    for (unsigned h=0; h<handles.size(); ++h)
      {
        const Handle& hnd=handles[h];
        AnchorPoint minap{hnd.minSliceX(),hnd.minSliceY(), AnchorPoint::sw},
          maxap{hnd.maxSliceX(),hnd.maxSliceY(), AnchorPoint::sw};
          // if its the x axis, we need to rotate (x,y) by 90 degrees
          if (h==handleIds[0])
            {
              std::swap(x,y);
              x*=-1;
              std::swap(minap.x,minap.y);
              minap.x*=-1;
              std::swap(maxap.x,maxap.y);
              maxap.x*=-1;
            }
          if (hnd.displayFilterCaliper)
            {
              if (ptInText(gc,hnd.minSliceLabel(), minap, x,y,sf) ||
                  ptInText(gc,hnd.maxSliceLabel(), maxap, x,y,sf)              )
                return h;
            }
      }
    return -1;
  }
}
#endif
