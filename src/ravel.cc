#include "ravel.h"
#include "ravelError.h"
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

namespace
{
  inline double sqr(double x) {return x*x;}
  // distance squared between two points
  inline double dsq(double x0, double y0, double x1, double y1)
  {return sqr(x1-x0)+sqr(y1-y0);}

  const string emptyString;
}

const unsigned Handle::caliperLength;

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
    return AnchorPoint{x()-off,y()-off,AnchorPoint::se};
  else 
    return AnchorPoint{x()+off,y()-off,AnchorPoint::sw};
}


void Handle::setSliceCoordinates(size_t& a_sliceIndex,double x, double y)
{
  // compute x coordinate along handle closest to (x,y), as fraction
  // of handle length times number of slicelabels
  double x_i=(sliceLabels.size()+1)*(m_x*x+m_y*y)/(m_x*m_x+m_y*m_y);
  if (sliceLabels.empty()) sliceLabels.resize(1);
  if (x_i<1)
    a_sliceIndex=0;
  else if (x_i>=sliceLabels.size())
    a_sliceIndex=sliceLabels.size()-1;
  else
    a_sliceIndex=size_t(x_i-0.5);
}

const std::string& Handle::sliceLabel() const 
{
  return sliceLabels.empty() ? emptyString : sliceLabels[sliceIndex];
}

const std::string& Handle::minSliceLabel() const 
{
  return sliceLabels.empty() ? emptyString :
    sliceLabels[std::min(sliceMin, sliceLabels.size()-1)];
} 

const std::string& Handle::maxSliceLabel() const 
{
  return sliceLabels.empty() ? emptyString :
    sliceLabels[std::min(sliceMax, sliceLabels.size()-1)];
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

void Ravel::redistributeHandles()
{
  double delta=1.5*M_PI/(handles.size()-1);
  double angle=0.5*M_PI+delta;
  for (unsigned i=0; i<handles.size(); ++i)
    if (handleIds.size()>0 && i==handleIds[0])
      handles[i].setHome(radius(),0);
    else if (handleIds.size()>1 && i==handleIds[1])
      handles[i].setHome(0, radius());
  // TODO handle higher rank (eg 3D) ravels
    else
      {
        handles[i].setHome(radius()*cos(angle), radius()*sin(angle));
        angle+=delta;
      }
}


//void Ravel::setOutputHandle(size_t dimension, size_t handle) 
//{
//  for (auto j: handleIds)
//    if (handle==j) return; // handle already an output handle
//  if (dimension>=handleIds.size()) handleIds.resize(dimension+1);
//  handleIds[dimension]=handle;
//  redistributeHandles();
//}

void Ravel::Handles::addHandle(const string& description, 
                     const vector<string>& sliceLabels)
{
  Super::resize(size()+1);
  back().description=description;
  if (!sliceLabels.empty()) // sliceLabels need at least one label
    back().sliceLabels = sliceLabels;
}

size_t Ravel::addHandle(const string& description, 
                     const vector<string>& sliceLabels)
{
  handles.addHandle(description,sliceLabels);
  if (handles.size()>=2)
    redistributeHandles();
  return handles.size()-1;    
}

void Ravel::moveHandleTo(unsigned handle, double xx, double yy) 
{
  if (handle<handles.size())
    {
      Handle& h=handles[handle];
      if (!h.collapsed())
        h.reductionOp=nextRedOp;
      h.moveTo(xx-x,yy-y, false /* always collapse */);
    }
}

void Ravel::snapHandle(unsigned handle)
{
  if (handle<handles.size())
    {
      Handle& thisHandle=handles[handle];
      int target=handleIfMouseOver(thisHandle.x(),thisHandle.y(),handle);
      if (target>-1)
        {
          thisHandle.swapHome(handles[target]);
          handles[target].snap();
          auto handleIt=find(handleIds.begin(), handleIds.end(), handle);
          auto targetIt=find(handleIds.begin(), handleIds.end(), target);
          if (handleIt!=handleIds.end() && targetIt!=handleIds.end())
            swap(*handleIt, *targetIt);
          else
            if (handleIt!=handleIds.end())
              *handleIt=target;
            else if (targetIt!=handleIds.end())
              *targetIt=handle;
        }
      thisHandle.snap();
    }
}

void Ravel::rescale(double r)
{
  if (r<=0)
    throw RavelError("Invalid radius provided");
  double scale=r/radius();
  m_radius=r;
  for (size_t i=0; i<handles.size(); ++i)
    handles[i].scaleHome(scale);
}

//int Ravel::handleIfMouseOver(double x, double y) const
//{
//  for (unsigned h=0; h<handles.size(); ++h)
//    {
//      double rsq=sqr(handles[h].x())+sqr(handles[h].y());
//      double dotp=x*handles[h].x() + y*handles[h].y();
//	  if (dotp>0 && dotp <= 1.1*rsq && /*1.1 allows for arrow */
//          rsq*(sqr(x)+sqr(y))-sqr(dotp) < 100*rsq)
//        return h;
//    }
//  return -1; // no handle found within tolerance
//}

int Ravel::handleIfMouseOver(double a_x, double a_y, int exclude) const
{
  // determine tolerances based on angles
  double cosTol=cos(0.75*M_PI/(handles.size()-1));
  double r=sqrt(sqr(a_x)+sqr(a_y));
  for (unsigned h=0; h<handles.size(); ++h)
    {
      if (int(h)==exclude) continue;
      if ((a_x*handles[h].x()+a_y*handles[h].y())> 
        (r*sqrt(sqr(handles[h].x())+sqr(handles[h].y()))) * cosTol)
        return h;
    }
  return -1; // no handle found within tolerance
}

Ravel::ElementMoving Ravel::sliceCtlHandle(int handle, double a_x, double a_y) const
{
  if (handle > -1)
    {
      const Handle& h=handles[handle];
      if (!h.collapsed())
        {
          if (!isOutputHandle(handle))
            {
              if (dsq(a_x, a_y, h.sliceX(), h.sliceY()) < 100)
                return slicer;
            }
          else if (h.displayFilterCaliper)
            {
              if (dsq(a_x, a_y, h.minSliceX(), h.minSliceY()) < 
                  sqr(0.01*Handle::caliperLength*radius()))
                return filterMin;
              else if (dsq(a_x, a_y, h.maxSliceX(), h.maxSliceY()) < 
                       sqr(0.01*Handle::caliperLength*radius()))
                return filterMax;
            }
        }
    }
  return Ravel::handle;
}


bool Ravel::onMouseMotion(double a_x, double a_y)
{
  if (lastHandle!=-1)
    {
      Handle& h=handles[lastHandle];
      a_x-=x; a_y-=y;
      switch (elementMoving)
        {
        case handle:
          moveHandleTo(lastHandle, a_x+x, a_y+y);
          break;
        case slicer:
          h.setSliceCoordinates(h.sliceIndex, a_x, a_y);
          break;
        case filterMin:
          h.setSliceCoordinates(h.sliceMin, a_x, a_y);
          break;
        case filterMax:
          h.setSliceCoordinates(h.sliceMax, a_x, a_y);
          break;
        }
    }
  return lastHandle != -1;
}

void Ravel::onMouseDown(double xx, double yy)
{
  lastHandle=handleIfMouseOver(xx-x,yy-y);
  elementMoving = sliceCtlHandle(lastHandle, xx-x,yy-y);
}

void Ravel::onMouseUp(double a_x, double a_y)
{
  onMouseMotion(a_x,a_y);
  if (lastHandle!=-1 && elementMoving==handle)
    snapHandle(lastHandle);
  lastHandle=-1;
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

string Ravel::description() const
{
  string r;
  if (handles.size()>1)
    {
      for (auto i: handleIds)
        {
          auto& h=handles[i];
          if (!r.empty()) r+=" by ";
          r+=h.collapsed()? h.reductionDescription(): h.description;
        }
      
      for (size_t i=0; i<handles.size(); ++i)
        if (!isOutputHandle(i))
          {
            const Handle& h=handles[i];
            if (h.collapsed())
              {
                switch (h.reductionOp)
                {
                case Op::sum:
                  r+=" summed over ";
                  break;
                case Op::prod:
                  r+=" product of ";
                  break;
                case Op::av:
                  r+=" averaged over ";
                  break;
                case Op::stddev:
                  r+=" standard deviation of ";
                  break;
                case Op::min:
                  r+=" minimum of ";
                  break;
                case Op::max:
                  r+=" maximum of ";
                  break;
                }
                r+= h.description;
              }
            else
              r+=" where "+h.description+"="+h.sliceLabel();
          }
    }
  return r;
}
