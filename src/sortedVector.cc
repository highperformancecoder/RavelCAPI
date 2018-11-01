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
        if (!sv.calipersUnrestricted())
          {
            minLabel=sv[0];
            maxLabel=sv[sv.size()-1];
          }
      }
      ~PreserveCalipers() {
        if (!minLabel.empty() || !maxLabel.empty())
          sv.setCalipers(minLabel,maxLabel);
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

  void SortedVector::setCalipers(const string& l1, const string& l2)
  {
    m_sliceMin=0;
    m_sliceMax=std::numeric_limits<size_t>::max()-1;
    for (size_t i=0; i<indices.size(); ++i)
      {
        if (labels[indices[i]]==l1)
          m_sliceMin=i;
        if (labels[indices[i]]==l2)
          m_sliceMax=i;
      }
    if (m_sliceMax<m_sliceMin) swap(m_sliceMin,m_sliceMax);
  }

}
