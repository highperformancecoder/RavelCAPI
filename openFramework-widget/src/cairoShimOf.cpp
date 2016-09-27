#include "../../src/cairoShim.h"
#include "../../src/ravelCairoImpl.h"
#include "ofApp.h"

namespace ravel
{
  typedef ofApp* G;

  template <> class CairoShimImpl<G> 
  {
  public:
    double penx=0, peny=0;
  };

  template <> CairoShim<G>::CairoShim(G) 
  {
    impl=new CairoShimImpl<G>;
  }
  template <> CairoShim<G>::~CairoShim() {delete impl;}

  template <> void CairoShim<G>::moveTo(double x, double y) 
  {impl->penx=x; impl->peny=y;}
  template <> void CairoShim<G>::lineTo(double x, double y) {
    ofDrawLine(impl->penx,impl->peny,x,y);
    moveTo(x,y);
  }
  template <> void CairoShim<G>::relMoveTo(double x, double y) {
    moveTo(impl->penx+x, impl->peny+y);
  }
  template <> void CairoShim<G>::relLineTo(double x, double y) {
    lineTo(impl->penx+x, impl->peny+y);
  }
  template <> void CairoShim<G>::arc(double x, double y, double radius, double start, double end) {
    if (end==2*M_PI)
      ofDrawCircle(x,y,radius);
    else
      {
        ofBeginShape();
        // implement as 100 splines
        if (start>end) swap(start,end);
        double incr=0.01*(end-start);
        for (; start<end; start+=incr)
          {
            double xx=x+radius*cos(start), yy=y+radius*sin(start);
            ofCurveVertex(xx,yy);
          }
        ofEndShape();
      }
  }
  template <> void CairoShim<G>::setLineWidth(double) {}

    // paths
  template <> void CairoShim<G>::newPath() {}
  template <> void CairoShim<G>::closePath() {}
  template <> void CairoShim<G>::fill() {}
  template <> void CairoShim<G>::stroke() {}
  template <> void CairoShim<G>::strokePreserve() {}

    // sources
  template <> void CairoShim<G>::setSourceRGB(double r, double g, double b) 
  {ofSetColor(255*r,255*g,255*b);}

    
    // text. Argument is in UTF8 encoding
  template <> void CairoShim<G>::showText(const std::string& s) {
    ofDrawBitmapString(s,impl->penx,impl->peny);
  }
  template <> void CairoShim<G>::setTextExtents(const std::string&) {}
  template <> double CairoShim<G>::textWidth() const {return 0;}
  template <> double CairoShim<G>::textHeight() const {return 0;}

    // matrix transformation
  template <> void CairoShim<G>::identityMatrix() {ofLoadIdentityMatrix();}
  template <> void CairoShim<G>::translate(double x, double y) 
  {ofTranslate(x,y);}
  template <> void CairoShim<G>::scale(double sx, double sy) 
  {ofScale(sx,sy);}
  template <> void CairoShim<G>::rotate(double angle) 
  {
    constexpr double deg=180/M_PI;
    ofRotate(deg*angle);
  }

    // context manipulation
  template <> void CairoShim<G>::save() {
    ofPushMatrix();
    ofPushStyle();
  }
  template <> void CairoShim<G>::restore() {
    ofPopStyle();
    ofPopMatrix();
  }

  template class RavelCairo<G>;
 
}

