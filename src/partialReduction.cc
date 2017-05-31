#include "partialReduction.h"
#include <factory.h>
#ifdef CLASSDESC
#include <classdesc_epilogue.h>
#endif
#ifdef ECOLAB_LIB
#include <ecolab_epilogue.h>
#endif

using namespace std;
using namespace classdesc;

namespace classdesc
{
  using namespace ravel;
  template <> Factory<PartialReduction,PartialReductionType>::Factory()
  {
    registerType<Bin>();
    registerType<Scan>();
    registerType<Change>();
  }
}

namespace ravel
{

  namespace
  {
    Factory<PartialReduction,PartialReductionType> factory;
  }

  PartialReduction* PartialReduction::create(PartialReductionType t)
  {
    return factory.create(t);
  }


  vector<PartialReduction::IndexVal> Bin::operator()(const vector<double>& x) const
  {
    vector<IndexVal> r;
    for (size_t i=0; i<x.size(); i+=binSize)
      {
        r.emplace_back(i+binSize/2,0);
        for (size_t j=0; j<binSize && i+j<x.size(); ++j)
          switch (op)
            {
            case add:
              r.back().value+=x[i+j];
              break;
            case multiply:
              r.back().value*=x[i+j];
              break;
            }
      }
    return r;
  }

  vector<PartialReduction::IndexVal> Scan::operator()(const vector<double>& x) const
  {
    vector<IndexVal> r;
    r.emplace_back(0,x[0]);
    for (size_t i=1; i<x.size(); ++i)
      {
        r.emplace_back(i,x[i]);
        switch (op)
          {
          case add:
            r[i].value+=r[i-1].value;
            if (i>=window)
              r[i].value-=r[i-window].value;
            break;
          case multiply:
            r[i].value*=r[i-1].value;
            if (i>=window && r[i-window].value)
              r[i].value/=r[i-window].value;
            break;
          }
      }
    return r;
  }

  vector<PartialReduction::IndexVal> Change::operator()(const vector<double>& x) const
  {
    vector<IndexVal> r;
    for (size_t i=offset; i<x.size(); ++i)
      switch (op)
        {
        case subtract:
          r.emplace_back(i,x[i]-x[i-offset]);
          break;
        case divide:
          r.emplace_back(i,x[i]/x[i-offset]);
          break;
        case percent:
          r.emplace_back(i,100*(x[i]-x[i-offset])/x[i-offset]);
          break;
        case relative:
          r.emplace_back(i,(x[i]-x[i-offset])/x[i-offset]);
          break;
        }
    return r;
  }
}
