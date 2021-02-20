#include "sortedVector.h"
#ifdef CLASSDESC
#include <classdesc_epilogue.h>
#endif
#ifdef ECOLAB_LIB
#include <ecolab_epilogue.h>
#endif
#include <algorithm>
#include <set>
#include <string>
#include <regex>
#include <chrono>
#include <iomanip>
#include <sstream>

#include <stdlib.h>
#include <time.h>


using namespace std;
using namespace std::chrono;

#include <boost/date_time.hpp>
using namespace boost;
using namespace boost::posix_time;
using namespace boost::gregorian;

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

  void extract(const string& fmt, const string& data, int pos1, const char* re1, int& var1,
               int pos2, const char* re2, int& var2)
  {
    string rePat=fmt.substr(0,pos1)+re1+
      fmt.substr(pos1+2,pos2-pos1-2)+re2+
      fmt.substr(pos2+2);
    regex pattern(rePat);
    smatch match;
    regex_match(data,match,pattern);
    var1=stoi(match[1]);
    var2=stoi(match[2]);
  }

  ptime sToPtime(const string& s)
  {
    const char* p=s.c_str();
    char *lp;
    unsigned long d[]={1,1,1,0,0,0}; // Y,M,D,h,m,s
    size_t i=0;
    for (; i<6 && *p; p=lp, ++i)
      {
        d[i]=strtoul(p,&lp,10);
        if (lp==p)
          break;
        while (*lp && !isdigit(*lp)) lp++;
      }
    if (i==0)
      throw runtime_error("invalid date/time: "+s);
    return ptime(date(d[0],d[1],d[2]), time_duration(d[3],d[4],d[5]));
  }

  // custom strptime that handle custom %Q marker and screwy dates with single digits
  ptime strptime(const string& format, const string& dateTime)
  {
    string::size_type pq;
    static regex screwyDates{R"(%([mdyY])[^%]%([mdyY])[^%]%([mdyY]))"};
    smatch m;
    if ((pq=format.find("%Q"))!=string::npos)
      {
        // year quarter format expected. Takes the first %Y (or
        // %y) and first %Q for year and quarter
        // respectively. Everything else is passed to regex, which
        // can be used to match complicated patterns.
        string pattern;
        int year, quarter;
        auto pY=format.find("%Y");
        if (pY>=0)
          if (pq<pY)
            extract(format,dateTime,pq,"(\\d)",quarter,pY,"(\\d{4})",year);
          else
            extract(format,dateTime,pY,"(\\d{4})",year,pq,"(\\d)",quarter);
        else
          throw runtime_error("year not specified in format string");
        if (quarter<1 || quarter>4)
          throw runtime_error("invalid quarter "+to_string(quarter));
        return ptime(date(year, 4*(quarter-1)+1, 1));
      }
    else if (regex_match(format, m, screwyDates)) // handle dates with 1 or 2 digits see Ravel ticket #35
      {
        static regex valParser{R"((\d+)\D(\d+)\D(\d+))"};
        smatch val;
        if (regex_match(dateTime, val, valParser))
          {
            int day, month, year;
            for (size_t i=1; i<val.size(); ++i)
              {
                
                int v;
                try
                  {v=stoi(val[i]);}
                catch (...)
                  {throw runtime_error(val.str(i)+" is not an integer");}
                switch (m.str(i)[0])
                  {
                  case 'd': day=v; break;
                  case 'm': month=v; break;
                  case 'y':
                    if (v>99) throw runtime_error(val.str(i)+" is out of range for %y");
                    year=v>68? v+1900: v+2000;
                    break;
                  case 'Y': year=v; break;
                  }
              }
            return ptime(date(year,month,day));
          }
        else
          throw runtime_error(dateTime+" doesn't match "+format);
      }
    else if (!format.empty())
      {
        istringstream is(dateTime);
        is.imbue(locale(is.getloc(), new time_input_facet(format.c_str())));
        ptime pt;
        is>>pt;
        if (pt.is_special())
          throw runtime_error("invalid date/time: "+dateTime);
        return pt;
      }
    else
      return sToPtime(dateTime);
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
  
  SortedVector::Order SortedVector::order(Order o, OrderType type, const std::string& format)
  {
    PreserveCalipers pc(*this);
    m_order=o;
    indices.resize(labels.size());
    for (size_t i=0; i<indices.size(); ++i) indices[i]=i;

    // reset calipers
    m_sliceMin=0;
    m_sliceMax=std::numeric_limits<size_t>::max()-1;
    switch (o)
      {
      case forward:
        switch (type)
          {
          case string: 
            sort(indices.begin(), indices.end(), [&](size_t i, size_t j) {
                                                   return labels[i]<labels[j];
                                                 });
            break;
          case time:
            sort(indices.begin(), indices.end(), [&](size_t i, size_t j) {
                               return strptime(format,labels[i])<strptime(format,labels[j]);
                                                 });
            m_order=timeForward;
            break;
          case value:
            sort(indices.begin(), indices.end(), [&](size_t i, size_t j) {
                                                   return numLess(labels[i],labels[j]);
                                                 });
            m_order=numForward;
            break;
          }
        break;
      case reverse:
        switch (type)
          {
          case string: 
            sort(indices.begin(), indices.end(), [&](size_t i, size_t j) {
                                                   return labels[i]>labels[j];
                                                 });
            break;
          case time:
            sort(indices.begin(), indices.end(), [&](size_t i, size_t j) {
                               return strptime(format,labels[i])>strptime(format,labels[j]);
                                                 });
            m_order=timeReverse;
            break;
          case value:
            sort(indices.begin(), indices.end(), [&](size_t i, size_t j) {
                                                   return numLess(labels[j],labels[i]);
                                                 });
            m_order=numReverse;
            break;
          }
        break;
      default:
        break;
      }
    return m_order;
  }

  SortedVector::Order SortedVector::order(Order o)
  {
    PreserveCalipers pc(*this);
    m_order=o;
    indices.resize(labels.size());
    for (size_t i=0; i<indices.size(); ++i) indices[i]=i;
    switch (o)
      {
      case numForward:
        order(forward, value);
        break;
      case numReverse:
        order(reverse, value);
        break;
      case timeForward:
        order(forward, time);
        break;
      case timeReverse:
        order(reverse, time);
        break;
      default:
        order(o, string);
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
    m_sliceMin=0;
    m_sliceMax=m_sliceMax=std::numeric_limits<size_t>::max()-1;
    assert(isPermValid());
  }

  void SortedVector::customPermutation(const std::vector<std::string>& v)
  {
    PreserveCalipers pc(*this);
    map<std::string, size_t> labelToIndex;
    for (size_t i=0; i<labels.size(); ++i)
      labelToIndex[labels[i]]=i;
    m_order=custom;
    indices.clear();
    for (auto i: v)
      {
        auto j=labelToIndex.find(i);
        if (j!=labelToIndex.end())
          indices.push_back(j->second);
      }
    m_sliceMin=0;
    m_sliceMax=m_sliceMax=std::numeric_limits<size_t>::max()-1;
  }
  
  bool SortedVector::isPermValid() const
  {
    set<size_t> s(indices.begin(), indices.end());
    return indices.size()==s.size() && (s.empty() || *s.rbegin()<labels.size());
  }

  void SortedVector::setCalipers(const std::string& l1, const std::string& l2)
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
