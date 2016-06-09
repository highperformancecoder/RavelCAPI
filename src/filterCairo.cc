#include "filterCairo.h"
#include "cairoShim.h"
#ifdef CLASSDESC
#include <classdesc_epilogue.h>
#endif
#ifdef ECOLAB_LIB
#include <ecolab_epilogue.h>
#endif
#include <algorithm>
using namespace ravel;
using namespace std;

namespace
{
  string str(double x) {
    ostringstream os;
    os<<x;
    return os.str();
  }
}

template <class G>
void FilterCairo<G>::render() const
{
  const unsigned numBins=20;
  double maxH=*max_element(histogram.begin(),histogram.end());

  double fmin=max(filterMin, minVal());
  double fmax=min(filterMax, maxVal());
  
  CairoShim<G> g(this->g);
  g.save();
  g.scale(width/maxH, height/numBins);
  for (size_t i=0; i<numBins; ++i)
    {
      g.moveTo(0,double(i));
      g.lineTo(histogram[i],double(i));
      g.lineTo(histogram[i],double(i+1));
      g.lineTo(0,double(i+1));
      g.closePath();
      double v=(i*(maxVal()-minVal()))/numBins+minVal();
      if (v>=fmin && v<=fmax)
        g.setSourceRGB(0,1,0);
      else
        g.setSourceRGB(1,0,0);
      g.fill();
    }
  g.restore();
  g.save();
  g.identityMatrix();
  g.setLineWidth(2);

  double sy=height/(maxVal()-minVal());
  //  g.scale(sx,sy);

  // draw box and whisker indicating filter bounds
  const double bwWidth=0.2*width;
  const double bwOffs=0.1*width;
  const double bwCentre=0.5*bwWidth+bwOffs;
  g.setSourceRGB(0,0,0);

  g.moveTo(bwOffs,0);
  g.relLineTo(bwWidth,0);
  g.stroke(); // for some reason GDI+ fills in skipped segments!
  g.moveTo(bwOffs,(maxVal()-minVal())*sy);
  g.relLineTo(bwWidth,0);
  g.stroke();
  g.moveTo(bwCentre,0);
  if (fmin-minVal()!=0) //without guards, subsequent stroke fails with cairo
    {                   // without stroke, drawing fails on GDI+ - sigh!
      g.relLineTo(0,(fmin-minVal())*sy);
      g.stroke();
    }
  if (maxVal()-fmax!=0)
    {
      g.moveTo(bwCentre,(fmax-minVal())*sy);
      g.relLineTo(0,(maxVal()-fmax)*sy);
      g.stroke();
    }

  g.moveTo(bwOffs, (fmin-minVal())*sy);
  g.relLineTo(bwWidth,0);
  g.relLineTo(0,(fmax-fmin)*sy);
  g.relLineTo(-bwWidth,0);
  g.relLineTo(0,(fmin-fmax)*sy);
  g.stroke();

  // value labels
  double sf=0.01*width;
  g.scale(sf,sf); // scale factor required to get GDI to scale fonts
  g.moveTo((bwOffs+bwWidth)/sf,(fmin-minVal())*sy/sf+10);
  g.showText(str(fmin));
  g.moveTo((bwOffs + bwWidth)/sf, (fmax - minVal())*sy/sf);
  g.showText(str(fmax));

  g.restore();
}

template <class G>
void FilterCairo<G>::clamp(double& v)
{
  if (v<minVal()) v=minVal();
  if (v>maxVal()) v=maxVal();
}

template <class G>
void FilterCairo<G>::onMouse(double, double y)
{
  if (mouseTracking!=none)
    {
      double v=((maxVal()-minVal())*y)/height+minVal();
      if ((mouseTracking==lowerLimit && v<filterMax) || v<filterMin)
        clamp(filterMin=min(v,filterMax));
      else
        clamp(filterMax=max(v,filterMin));
    }
}

template <class G>
void FilterCairo<G>::onMouseDown(double, double y)
{
  // determine which limit we're closer to
  double v=((maxVal()-minVal())*y)/height+minVal();
  if (fabs(v-max(minVal(),filterMin))<fabs(v-min(maxVal(),filterMax)))
    mouseTracking=lowerLimit;
  else
    mouseTracking=upperLimit;
}

#ifdef USE_GDI
#include <windows.h>
template class FilterCairo<HDC>;

#else
#define CAIRO_WIN32_STATIC_BUILD
#include <cairo.h>
#undef CAIRO_WIN32_STATIC_BUILD

template class FilterCairo<cairo_t*>;
#endif
