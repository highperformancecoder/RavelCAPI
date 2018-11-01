#include "sortedVector.h"
#ifdef CLASSDESC
#include <classdesc_epilogue.h>
#endif
#ifdef ECOLAB_LIB
#include <ecolab_epilogue.h>
#endif
#include <algorithm>
#include <set>
#include <stdlib.h>

using namespace std;

namespace
{
  // numerically compare two strings. Numerical comparison takes place
  // on the leading numerical portion of string - if these are equal,
  // the strings are sorted lexicographically
  bool numLess(const string& x, const string& y)
  {
    double xVal=atof(x.c_str()), yVal=atof(y.c_str());
    return xVal<yVal || (xVal==yVal && x<y);
  }
}

namespace ravel
{
  void SortedVector::resize(size_t sz) 
  {
    labels.resize(sz);
    order(order());
  }

  namespace {
    // retain min/max labels to reset the calipers to a sensible value
    struct PreserveCalipers
    {
      string minLabel, maxLabel;
      SortedVector& sv;
      PreserveCalipers(SortedVector& sv): sv(sv) {
        if (sv.size()>0)
          {
            minLabel=sv[0];
            maxLabel=sv[sv.size()-1];
          }
      }
      ~PreserveCalipers() {
        if (sv.min()>0 || sv.max()<sv.size()-1)
          {
            // reset calipers to previous labels
            sv.m_sliceMin=0, sv.m_sliceMax=std::numeric_limits<size_t>::max()-1;
            for (size_t i=0; i<sv.size(); ++i)
              {
                if (sv[i]==minLabel)
                  sv.m_sliceMin=i;
                if (sv[i]==maxLabel)
                  sv.m_sliceMax=i;
              }
            if (sv.m_sliceMax<sv.m_sliceMin) swap(sv.m_sliceMin,sv.m_sliceMax);
          }
      }
    };
  }
  
  SortedVector::Order SortedVector::order(Order o)
  {
    PreserveCalipers pc(*this);
    m_order=o;
    indices.resize(labels.size());
    for (size_t i=0; i<indices.size(); ++i) indices[i]=i;
    switch (o)
      {
      case forward:
        sort(indices.begin(), indices.end(), [&](size_t i, size_t j) {
            return labels[i]<labels[j];
          });
        break;
      case reverse:
        sort(indices.begin(), indices.end(), [&](size_t i, size_t j) {
            return labels[i]>labels[j];
          });
        break;
      case numForward:
        sort(indices.begin(), indices.end(), [&](size_t i, size_t j) {
            return numLess(labels[i],labels[j]);
          });
        break;
      case numReverse:
        sort(indices.begin(), indices.end(), [&](size_t i, size_t j) {
            return numLess(labels[j],labels[i]);
          });
        break;
      default:
        break;
      }
    assert(isPermValid());
    return m_order;
  }
  
  void SortedVector::customPermutation(const std::vector<size_t>& p)
  {
    PreserveCalipers pc(*this);
    m_order=custom;
    indices=p;
    assert(isPermValid());
  }

  bool SortedVector::isPermValid() const
  {
    set<size_t> s(indices.begin(), indices.end());
    return indices.size()==s.size() && (s.empty() || *s.rbegin()<labels.size());
  }
}
