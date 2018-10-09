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

void Ravel::redistributeHandles()
{
  double delta;
  switch (rank())
    {
    case 1: delta=2*M_PI/(handles.size()); break;
    default: delta=1.5*M_PI/(handles.size()-1); break;
    }
  vector<unsigned> handlesToSet;
  set<unsigned> angleAlreadySet;
  for (unsigned i=0; i<handles.size(); ++i)
    {
      auto& h=handles[i];
      if (handleIds.size()>0 && i==handleIds[0])
        {
          h.setHome(radius(),0); // first output handle is east facing
          assert(!angleAlreadySet.count(0));
          angleAlreadySet.insert(0);
        }
      else if (handleIds.size()>1 && i==handleIds[1])
        {
          assert(!angleAlreadySet.count(handles.size()-1));
          h.setHome(0, radius()); // second output handle is south facing
          angleAlreadySet.insert(handles.size()-1);
        }
      else 
        {
          double a=atan2(-h.y(),h.x());
          if (a<0) a+=2*M_PI;
          a/=delta;
          unsigned ia=a;
          if ((rank()==0 || ia>0) && (rank()<=1 || ia<handles.size()-1) &&
              !angleAlreadySet.count(ia) && fabs(a-ia)<0.1*delta)
            angleAlreadySet.insert(ia); // leave handle where it is
          else
            handlesToSet.push_back(i);
        }
    }

  // now move handles not in correct position
  double iangle=1;
  for (auto i: handlesToSet)
    {
      while (angleAlreadySet.count(iangle)) iangle++;
      double angle=delta * iangle;
      iangle++;
      // -ve y because y coordinates increase going down the page
      handles[i].setHome(radius()*cos(angle), -radius()*sin(angle));
    }
}


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
      h.moveTo(xx-x,yy-y, /*false*/ !moved /* only collapse if moving*/);
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

int Ravel::handleIfMouseOver(double a_x, double a_y, int exclude) const
{
  // determine tolerances based on angles
  double cosTol=cos(0.25*M_PI);
  if (handles.size()>2)
    cosTol=cos(0.75*M_PI/(handles.size()-1));
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
  if (sqr(a_x-x)+sqr(a_y-y)<sqr(Handle::hubRadius*radius()))
    return hub;
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
          else if (h.displayFilterCaliper())
            {
              if (dsq(a_x, a_y, h.minSliceX(), h.minSliceY()) < 
                  sqr(0.01*Handle::caliperLength*radius()))
                return filterMin;
              else if (dsq(a_x, a_y, h.maxSliceX(), h.maxSliceY()) < 
                       sqr(0.01*Handle::caliperLength*radius()))
                return filterMax;
            }
          if (sqr(a_x-h.x())+sqr(a_y-h.y())<
              sqr(Handle::hubRadius)*(sqr(h.x())+sqr(h.y())))
            return Ravel::handle;
        }
      else
        return Ravel::handle; // collapsed handle can be active all over
    }
  return none;
}


bool Ravel::onMouseMotion(double a_x, double a_y)
{
  if (elementMoving == hub &&
      sqr(a_x-x)+sqr(a_y-y)>=sqr(Handle::hubRadius*radius()))
    {
      lastHandle=handleIfMouseOver(a_x-x,a_y-y);
      if (lastHandle>=0)
        {
          auto i=find(handleIds.begin(), handleIds.end(), lastHandle);
          if (i!=handleIds.end())
            {
              // hub moved onto handle, initiate slicing, reducing rank
              handleIds.erase(i);
              elementMoving=slicer;
            }
        }
    }

  if (lastHandle!=-1 && elementMoving == slicer &&
      sqr(a_x-x)+sqr(a_y-y)<sqr(Handle::hubRadius*radius()))
    {
      // slicer moved onto hub, increasing rank
      handleIds.push_back(lastHandle);
      elementMoving=hub;
    }
  
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
        case filterMax:
          {
            size_t sm;
            h.setSliceCoordinates(sm, a_x, a_y);
            // do not move the calipers on top of each other, as this causes them to stick
            if (elementMoving==filterMin)
              h.sliceLabels.min(sm<h.sliceMax()? sm: h.sliceMax()-1);
            else
              h.sliceLabels.max(sm>h.sliceMin()? sm: h.sliceMin()+1);
            break;
          }
        }
    }
  moved=true;
  return lastHandle != -1;
}

void Ravel::onMouseDown(double xx, double yy)
{
  moved=false;
  if (sqr(xx-x)+sqr(yy-y)<sqr(Handle::hubRadius*radius()))
    {
      elementMoving = hub;
      return;
    }
  lastHandle=handleIfMouseOver(xx-x,yy-y);
  elementMoving = sliceCtlHandle(lastHandle, xx-x,yy-y);
}

void Ravel::onMouseUp(double a_x, double a_y)
{
  onMouseMotion(a_x,a_y);
  moved=false;
  if (lastHandle!=-1 && elementMoving==handle)
    snapHandle(lastHandle);
  lastHandle=-1;
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

const char* Ravel::explain(double xx, double yy) const
{
  int h=handleIfMouseOver(xx,yy);
  switch (sliceCtlHandle(h,xx,yy))
    {
    case hub:
      return "drag slicer to start slicing hypercube";
    case handle:
      assert(h>=0);
      if (handles[h].collapsed())
        return "drag outwards to drill down into data";
      else
        return "rotate to pivot hypercube or drag inwards to roll up data";
    case slicer:
      return "drag to change slice";
    case filterMin: case filterMax:
      return "adjust calipers to dice data";
    default:
      return "";
    }
}

