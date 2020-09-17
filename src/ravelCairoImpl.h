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
      double angle=atan2(-hx,hy);
      gc.rotate(angle);
      gc.moveTo(0,0);
      double cutoff=atan(0.5);
      double offset=-Handle::caliperLength;
      if (hx<-2*hy)
        offset*=-1;
      gc.lineTo(offset,0);
      gc.setLineWidth(1/sf);
      gc.stroke();
      gc.moveTo(offset,0);
      if (abs(hx)>2*abs(hy) || abs(hy)>2*abs(hx))
        gc.rotate(-0.25*pi-angle);
      else
        gc.rotate(-angle);
      gc.scale(0.5,0.5);
      gc.setTextExtents(label);
      gc.relMoveTo(-gc.textWidth(),0);
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

    // draw half moons on output handles to indicate inactive slicers
    for (auto i: handleIds)
      if (i<handles.size())
        {
          auto& h=handles[i];
          gc.save();
          gc.setSourceRGB(palette[i][0],palette[i][1],palette[i][2]);
          gc.newPath();
          gc.arc(0,0,Handle::hubRadius*radius()/sf, 2*pi, 0);
          gc.clip();
          // draw half-moons in hub to indicate slicer not active
          gc.newPath();
          gc.arc(Handle::hubRadius*h.x()/sf, Handle::hubRadius*h.y()/sf, Handle::slicerRadius, 0, 2*pi);
          gc.fill();
          gc.restore();
        }
 
      
    // draw central circle, and clip its interior out
    gc.newPath();
    gc.arc(0,0,Handle::hubRadius*radius()/sf, 0, 2*pi);
    gc.strokePreserve();
    gc.moveTo(-1.5*radius(),-1.5*radius());
    gc.lineTo(-1.5*radius(),1.5*radius());
    gc.lineTo(1.5*radius(),1.5*radius());
    gc.lineTo(1.5*radius(),-1.5*radius());
    gc.closePath();
    gc.clip();
    
    gc.moveTo(0.5*radius()/sf,0.5*radius()/sf);
    gc.showText(std::to_string(rank())+"D");
    for (unsigned i=0; i<handles.size(); ++i)
      {
        const Handle& h=handles[i];
        double hx=h.x()/sf, hy=h.y()/sf;

        gc.setSourceRGB(palette[i][0],palette[i][1],palette[i][2]);
                                           
        drawLine(gc, hx,hy);

        bool sliced=!isOutputHandle(i);
        // draw indicators of sorting order, and filter calipers
        if (!h.collapsed())
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
            if (h.displayFilterCaliper())
              {
                if (!sliced || h.sliceIndex!=0)
                  drawCaliper(gc,sf,hx,hy,h.minSliceX()/sf,h.minSliceY()/sf,
                              h.minSliceLabel());
                if (!sliced || h.sliceIndex!=h.sliceLabels.size()-1)
                  drawCaliper(gc,sf,hx,hy,h.maxSliceX()/sf,h.maxSliceY()/sf,
                              h.maxSliceLabel());
                h.setMinSliceLabelExtents(gc);
                h.setMaxSliceLabelExtents(gc);
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
        h.setDescLabelExtents(gc);

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

        if (h.collapsed() || !h.partialReductions().empty())
          {
            // show reduction operations
            assert(h.reductionOp < sizeof(opLabels)/sizeof(opLabels[0]));
            gc.setTextExtents(opLabels[h.reductionOp]);
        
            gc.moveTo(h.opX()/sf-gc.textWidth(), h.opY()/sf);
          
            gc.showText(opLabels[h.reductionOp]);
            h.setOpLabelExtents(gc);
          }

        // slice control
        if (!h.collapsed() && sliced)
          {
            gc.newPath();
            gc.arc(h.sliceX()/sf, h.sliceY()/sf, Handle::slicerRadius, 0, 2*pi);
            gc.fill();
          
            gc.save();
            double angle;
            if (abs(h.x())>2*abs(h.y()) || abs(h.y())>2*abs(h.x()))
              angle=-0.25*pi;
            else
              angle=0;
            double scale=.75;
            gc.rotate(angle);
            gc.scale(scale,scale);
            double x=(h.sliceX()/sf+2)/scale, y=(h.sliceY()/sf-2)/scale;
            // do moveTo in rotated frame to allow for HTML canvas behaviour
            gc.moveTo(x*cos(angle)+y*sin(angle), y*cos(angle)-x*sin(angle));
            gc.showText(h.sliceLabel());
            gc.restore();
          }
     }

    // display explanatory message
    gc.restore();
    if (!explain.empty())
      {
        gc.save();
        gc.setTextExtents(explain);
        double w=gc.textWidth(), h=gc.textHeight();
        gc.moveTo(explainX-0.05*w,explainY+0.1*h);
        gc.relLineTo(1.1*w,0);
        gc.relLineTo(0,-1.2*h);
        gc.relLineTo(-1.1*w,0);
        gc.closePath();
        gc.setSourceRGB(0,0,0);
        gc.strokePreserve();
        gc.setSourceRGB(1,1,1);
        gc.fill();
        gc.setSourceRGB(0,0,0);
        gc.moveTo(explainX,explainY);
        gc.showText(explain);
        gc.restore();
      }
  }

  template <class G>
  void RavelCairo<G>::onMouseDown(double xx, double yy)
  {
    Ravel::onMouseDown(xx,yy);
    if (elementMoving==none)
      {
        // check if caliper selected
        lastHandle=handleIfMouseOverCaliperLabel(xx,yy);
        if (lastHandle>-1)
          {
            Handle& h=handles[lastHandle];
            double dx=xx-x, dy=yy-y;
            // select which caliper we're moving
            if (h.sliceMin()==h.sliceMax()-1)
              // choose whichever handle is furthest from it's end, to avoid calipers sticking together
              elementMoving = dsq(dx,dy,h.x(),h.y())<dsq(dx,dy,0,0)? filterMin: filterMax;
            else
              elementMoving = dsq(dx,dy,h.minSliceX(),h.minSliceY())<
                dsq(dx,dy,h.maxSliceX(),h.maxSliceY())? filterMin: filterMax;
          }
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
  static bool ptInText(G& gc, const AnchorPoint& a, double x, double y, double sf)
  {
    switch (a.anchor)
      {
      case AnchorPoint::se:
        if (x<=a.x && x>=a.x-sf*a.width &&
            y<=a.y && y>=a.y-sf*a.height)
          return true;
        break;
      case AnchorPoint::sw:
        if (x>=a.x && x<=a.x+sf*a.width &&
            y<=a.y && y>=a.y-sf*a.height)
          return true;
        break;
      case AnchorPoint::ne:
        if (x>=a.x && x<=a.x+sf*a.width &&
            y>=a.y && y<=a.y+sf*a.height)
          return true;
        break;
      case AnchorPoint::nw:
        if (x<=a.x && x>=a.x-sf*a.width &&
            y>=a.y && y<=a.y+sf*a.height)
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
      if (ptInText(gc,handles[h].labelAnchor(),x,y,sf))
        return h;
    return -1;
  }

  template<class G>
  int RavelCairo<G>::handleIfMouseOverOpLabel(double x, double y) const
  {
    CairoShim<G> gc(g);
    x-=this->x; y-=this->y;
    double sf=0.01*radius();
    for (unsigned h=0; h<handles.size(); ++h)
      if (ptInText(gc,handles[h].opLabelAnchor(),x,y,sf))
        return h;
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
        if (hnd.displayFilterCaliper())
          {
            AnchorPoint minap=hnd.minCaliperLabelAnchor(),
              maxap=hnd.maxCaliperLabelAnchor();
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
            if (ptInText(gc,minap, x,y,sf) || ptInText(gc,maxap, x,y,sf))
              return h;
          }
      }
    return -1;
  }
}
#endif
