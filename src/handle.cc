#include "handle.h"
#ifdef CLASSDESC
#include <classdesc_epilogue.h>
#endif
#ifdef ECOLAB_LIB
#include <ecolab_epilogue.h>
#endif
#ifndef M_PI
#define M_PI		3.14159265358979323846
#endif
using namespace ravel;
using namespace std;

#include <math.h>
#include <assert.h>

constexpr const double Handle::caliperLength,  Handle::hubRadius, Handle::slicerRadius;


namespace
{
  inline double sqr(double x) {return x*x;}
  const string emptyString;
}

double Handle::opX() const
{
  double r=sqrt(sqr(x())+sqr(y()));
  // from 0 .. 135 degrees, we need to offset the x coordinate to
  // prevent a clash with the handle label
  if (y()<=0 && (x()>0 || fabs(x())<fabs(y())))
    return 1.2*x()-0.5*r;
  else
    return 1.2*x();
}

double Handle::opY() const
{
  double r=sqrt(sqr(x())+sqr(y()));
  if (y()>0 && fabs(x()) < fabs(y()))
    return 1.2*y() + 0.5*r;
  else
    return 1.2*y();
}

AnchorPoint Handle::labelAnchor() const
{
  double off=0.03*sqrt(sqr(x())+sqr(y()));
  if ((y()>0 && fabs(x()) < fabs(y())) || (!collapsed() && x() > fabs(y())))
    // in bottom quadrant, or near x axis anchor at south east
    return AnchorPoint{x()-off,y()-off,descLabelWidth,descLabelHeight,AnchorPoint::se};
  else 
    return AnchorPoint{x()+off,y()-off,descLabelWidth,descLabelHeight,AnchorPoint::sw};
}

AnchorPoint Handle::opLabelAnchor() const
{
  return AnchorPoint{opX(),opY(), opLabelWidth, opLabelHeight, AnchorPoint::se};
}

AnchorPoint Handle::minCaliperLabelAnchor() const
{
  return AnchorPoint{minSliceX(),minSliceY(), minSliceWidth, minSliceHeight,
      AnchorPoint::sw};
}

AnchorPoint Handle::maxCaliperLabelAnchor() const
{
  return AnchorPoint{maxSliceX(),maxSliceY(), maxSliceWidth, maxSliceHeight,
      AnchorPoint::sw};
}

void Handle::setSliceCoordinates(size_t& a_sliceIndex,double x, double y)
{
  // compute x coordinate along handle closest to (x,y), as fraction
  // of handle length times number of slicelabels
  double x_i=(sliceLabels.currentPermutation().size()+1)*
    (m_x*x+m_y*y)/(m_x*m_x+m_y*m_y);
  if (sliceLabels.empty()) sliceLabels.resize(1);
  if (x_i<1)
    a_sliceIndex=0;
  else if (x_i>=sliceLabels.currentPermutation().size())
    a_sliceIndex=sliceLabels.currentPermutation().size()-1;
  else
    a_sliceIndex=size_t(x_i-0.5);
}

const std::string& Handle::sliceLabel() const 
{
  return sliceIndex<sliceLabels.size() ? sliceLabels[sliceIndex]: emptyString;
}

const std::string& Handle::minSliceLabel() const 
{
  return sliceLabels.empty() ? emptyString : sliceLabels[0];
} 

const std::string& Handle::maxSliceLabel() const 
{
  return sliceLabels.empty() ? emptyString : sliceLabels[sliceLabels.size()-1];
} 

void Handle::moveTo(double x, double y, bool dontCollapse)
{
  double xySq=x*x+y*y, homeSq=home_x*home_x + home_y*home_y;
  if (!dontCollapse && xySq < 0.25 * homeSq)
    {
      // handle collapsed
      m_collapsed=true;
      snap();
    }
  else
    {
      // scale back to original length
      double r=sqrt(homeSq/xySq);
      m_x=x*r;
      m_y=y*r;
      m_collapsed=false;
    }
}

void Handle::snap()
{
  if (m_collapsed)
    {
      m_x=0.2*home_x;
      m_y=0.2*home_y;
    }
  else
    {
      // Ravel class will call setHome if r=snapx, or snapy
      m_x=home_x;
      m_y=home_y;
    }
}

void Handle::toggleCollapsed()
{
  m_collapsed=!m_collapsed;
  snap();
}

void Handle::setSlicer(const std::string& label)
{
  for (size_t i=0; i<sliceLabels.size(); ++i)
    if (sliceLabels[i]==label)
      {
        sliceIndex=i;
        break;
      }
}

namespace
{
  class PresortData: public classdesc::Poly<PresortData, PartialReduction>
  {
    PartialReductionType type() const override
    {return PartialReductionType::presort;}
    vector<size_t> m_indices;

  public:
    PresortData(const SortedVector& sv) {
      assert(sv.isPermValid());
      for (size_t i=0; i<sv.size(); ++i)
        m_indices.push_back(sv.idx(i));
    }

    void operator()(double* dest, const double* src, size_t stride, size_t num)
      const override
    {
      assert(num==m_indices.size());
      for (size_t i=0; i<num; ++i)
        dest[i*stride]=src[m_indices[i]*stride];
    }
    std::vector<size_t> indices(size_t N) const override {return m_indices;}
  };
}

void Handle::addPartialReduction(const std::shared_ptr<PartialReduction>& red)
{
  if (m_partialReductions.empty())
    {
      unreducedSliceLabels=sliceLabels;
      // we cannot rely on sliceLabels.order()==none, as custom sort order may be applied
      size_t i=0;
      for (; i<sliceLabels.size(); ++i)
        if (i!=sliceLabels.idx(i)) break;

      if (i<sliceLabels.size())
        // add a dummy p reduction that simply sorts the data
        m_partialReductions.emplace_back(new PresortData(sliceLabels));
      sliceLabels.order(SortedVector::none);
    }
  m_partialReductions.push_back(red);

  // build new slicelabel vector
  auto origOrder=sliceLabels.order();
  for (auto& i: m_partialReductions)
    {
      SortedVector labels;
      bool sliceIndexUnset=true;
      for (auto j: i->indices(sliceLabels.size()))
        {
          if (sliceIndexUnset && j>=sliceIndex)
            {
              sliceIndex=labels.size();
              sliceIndexUnset=false;
            }
          labels.push_back(sliceLabels[j]);
        }
      if (sliceIndexUnset)
        sliceIndex=labels.size()-1;
      sliceLabels=move(labels);
    }
  // restore original order
  //  sliceLabels.order(origOrder);
}

void Handle::clearPartialReductions()
{
  if (!m_partialReductions.empty())
    {
      m_partialReductions.clear();
      unreducedSliceLabels.order(sliceLabels.order());
      sliceLabels=unreducedSliceLabels;
    }
}

string Handle::reductionDescription() const
{
  string r;
  if (collapsed())
    switch (reductionOp)
      {
      case Op::sum:
        r="sum of ";
        break;
      case Op::prod:
        r="product of ";
        break;
      case Op::av:
        r="average of ";
        break;
      case Op::stddev:
        r="standard deviation of ";
        break;
      case Op::min:
        r="minimum of ";
        break;
      case Op::max:
        r="maximum of ";
        break;
      }

  r+=description;
  return r;
}

double Handle::sliceCoordInterp(size_t idx, double x) const
{
  return (1-2.5*hubRadius)*(x*(idx+1))/(sliceLabels.currentPermutation().size()+1) +
    1.5*hubRadius*x;
}

bool Handle::displayFilterCaliper(bool d)
{
  m_displayFilterCaliper=d;
  if (!d)
    {
      sliceLabels.min(0);
      sliceLabels.max(std::numeric_limits<size_t>::max()-1);
    }
  return d;
}

HandleState Handle::getHandleState() const
{
  HandleState hs;
  hs.x=x();
  hs.y=y();
  hs.description=description;
  hs.sliceLabel=sliceLabel();
  hs.minLabel=minSliceLabel();
  hs.maxLabel=maxSliceLabel();
  hs.collapsed=collapsed();
  hs.displayFilterCaliper=displayFilterCaliper();
  hs.reductionOp=reductionOp;
  hs.order=sliceLabels.order();
  if (hs.order==HandleSort::custom)
    {
      auto& lv=sliceLabels.labelsVector();
      for (auto i: sliceLabels.currentPermutation())
        hs.customOrder.push_back(lv[i]);
    }
  return hs;
}

void Handle::setHandleState(const HandleState& hs)
{
  description=hs.description;
  if (hs.x*hs.x+hs.y*hs.y>0)
    moveTo(hs.x, hs.y, false);
  assert(isfinite(x()) && isfinite(y()));
  if (hs.collapsed!=collapsed())
    toggleCollapsed();
  displayFilterCaliper(hs.displayFilterCaliper);
  reductionOp=Op::ReductionOp(hs.reductionOp);

  if (hs.order==HandleSort::custom)
    sliceLabels.customPermutation(hs.customOrder);
  else
    sliceLabels.order(hs.order);

  sliceLabels.setCalipers(hs.minLabel, hs.maxLabel);
  setSlicer(hs.sliceLabel);
}

