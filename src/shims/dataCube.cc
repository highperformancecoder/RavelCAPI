#include <limits.h>
#include "dataCube.h"
#include "ravel.h"
#include "ravelError.h"
#ifdef CLASSDESC
#include <classdesc_epilogue.h>
#endif
#ifdef ECOLAB_LIB
#include <ecolab_epilogue.h>
#endif

#include <boost/type_traits.hpp>
#include <boost/tokenizer.hpp>
#include <boost/token_functions.hpp>
#include <fstream>

using namespace std;
using namespace ravel;
using boost::any;
using boost::any_cast;

string str(const any& x)
{
  if (const double *dx=any_cast<double>(&x))
    {
      ostringstream os;
      os<<*dx;
      return os.str();
    }
  else if (const string* sx = any_cast<string>(&x))
    return *sx;
  else
    return "";
}

// trim whitespace
static string trim(const string& x)
{
  size_t firstNonWhite=x.size(), lastNonWhite=0;
  for (size_t i=0; i<x.size(); ++i)
    if (!isspace(x[i]))
      {
        firstNonWhite=min(i,firstNonWhite);
        lastNonWhite=i;
      }
  return x.substr(0,lastNonWhite+1).substr(firstNonWhite);
}

vector<any> CSVFTokeniser::getLine()
{
  string buf;
  if (getline(input,buf))
    {
      // trim any extraneous carriage returns
      if (buf.back()=='\r')
        buf.resize(buf.size()-1);
      boost::escaped_list_separator<char> csvParser('\\',separator,'"');
      boost::tokenizer<boost::escaped_list_separator<char> >
        tok(buf.begin(), buf.end(),csvParser);
      // attempt to convert to doubles, if not then leave as strings
      vector<any> r;
      for (string t : tok)
        if (t.empty())
          r.push_back(any());
        else
          {
            try
              {
                size_t charsConverted;
                string tt=trim(t);
                r.push_back(any(stod(tt, &charsConverted)));
                if (charsConverted != tt.size())
                  { // didn't parse correctly as a number
                    r.back() = any(t);
                  }
              }
            catch (std::exception)
              {
                r.push_back(any(t));
              }
          }
      return r;
    }
  else
    return vector<any>();
}

void DataCube::loadFile(const std::string& fileName, char separator, const DataSpec& spec)
{
  ifstream input(fileName);
  CSVFTokeniser tok(input, separator);
  loadData(tok, spec);
}

namespace
{
  //used for storing unique labels in the order of their original appearance
  struct UniqueString
  {
    size_t appearanceOrder;
    string value;
    UniqueString(const string& value="", size_t appearanceOrder=0):
      appearanceOrder(appearanceOrder), value(value)  {}
    bool operator<(const UniqueString& x) const {
      return value<x.value;
    }
    operator const string&() const {return value;}
  };
}

void DataCube::loadData(Tokeniser& tok, const DataSpec& spec)
{
  rawData.clear();

  // compute number of actual column axes by subracting comment columns
  size_t nColAxes = spec.nColAxes;
  for (size_t cr: spec.commentCols) 
    nColAxes-=cr<spec.nColAxes;

  dimNames.resize(nColAxes);

  vector<set<UniqueString>> axisLabelSet(nColAxes);
  vector<vector<string>> colLabels;
  map<Key,double> tmpData;

  vector<any> line;
  for (unsigned long row=0; !(line=tok.getLine()).empty(); ++row)
    {
      // skip comment rows
      if (spec.commentRows.count(row))
        continue;

      if (row<spec.nRowAxes)
        {
          // compute number of value columns
          unsigned nvcol=0;
          for (unsigned i=spec.nColAxes; i<line.size(); ++i)
            if (!spec.commentCols.count(i))
              nvcol++;
          
          // parse column headers
          if (nvcol>1) 
            {
              axisLabelSet.push_back(set<UniqueString>());
              colLabels.push_back(vector<string>(line.size()));
              dimNames.push_back("?"); // create space for row axes names
            }
          string lastLabel;
          unsigned dim=0, labelCnt=0;
          for (unsigned long col=0; col<line.size(); ++col)
            if (!spec.commentCols.count(col))
              {
                if (col<spec.nColAxes)
                  {
                    string label=str(line[col]);
                    if (!label.empty())
                      {
                        labelCnt++;
                        dimNames[dim].swap(label);
                      }
                    dim++;
                  }
                else
                  {
                    // column slice label
                    string label=str(line[col]);
                    if (!label.empty()) 
                      lastLabel.swap(label);
                    if (!colLabels.empty())
                      {
                        axisLabelSet.back().insert(UniqueString(lastLabel, col));
                        if (col<colLabels.back().size()) colLabels.back()[col] = lastLabel;
                      }
                  }
              }
          if (spec.nColAxes>1)
            {
              // if only one dimension name provided, treat it as a row axis name
              if (labelCnt==1)
                dimNames.back().swap(dimNames[0]);
              else if (labelCnt>1 && line.size()>spec.nColAxes+1 && lastLabel.empty())
                {
                  dimNames.pop_back();
                  axisLabelSet.pop_back();
                  colLabels.pop_back();
                }
            }
              
        }
      else
        {
          // parse data rows
          Key key; //(axisLabelSet.size());
          LabelsVector labels;
          for (unsigned col=0, dim=0; col<line.size(); ++col)
            {
              if (!spec.commentCols.count(col))
                {
                  if (col<spec.nColAxes)
                    {
                      key.emplace_back(dimNames[dim],str(line[col]));
                      axisLabelSet[dim].insert(UniqueString(key.back().second,row));
                      dim++;
                    }
                  else // in data area
                    {
                      // build rest of keys from saved column labels
                      size_t i=nColAxes;
                      key.resize(nColAxes);
                      for (const vector<string>& cl:  colLabels)
                        key.emplace_back(dimNames[i++],cl[col]);
                      assert(key.size()==nColAxes+colLabels.size());
                      
                      if (!str(line[col]).empty())
                        {
                          // key should unique identify a row of the CSV table
                          if (tmpData.count(key))
                            {
                              string err="duplicate row detected:";
                              for (const any& t: line)
                                err+=" "+str(t);
                              throw RavelError(err);
                            }
                          if (const double*v=any_cast<double>(&line[col]))
                            {
                              m_maxCol=max(m_maxCol,size_t(col));
                              m_maxRow=row;
                              tmpData[key]=*v;
                            }
                        }
                    }
                }
            }
        }
    }

  m_dimLabels.clear();


  for (const set<UniqueString>& s: axisLabelSet)
    {
      vector<UniqueString> l(s.begin(), s.end());
      // resort axis labels in their appearance order
      sort(l.begin(), l.end(), [](const UniqueString& x, const UniqueString& y){
          return x.appearanceOrder<y.appearanceOrder;
        });
      m_dimLabels.push_back(vector<string>(l.begin(), l.end()));
    }

  LabelsVector labels;
  assert(dimNames.size()==m_dimLabels.size());
  for (size_t i=0; i<dimNames.size(); ++i)
    labels.emplace_back(dimNames[i],m_dimLabels[i]);
  rawData=RawData(labels);
  for (auto& i: tmpData)
    rawData[i.first]=i.second;

  m_sortBy.resize(m_dimLabels.size());
  leastRowAxis = spec.nColAxes - spec.commentCols.size();
}

//namespace 
//{
//  class HyperSlice: public Op, public map<DataCube::Key, double>
//  {
//    typedef DataCube::Key Key;
//    double m_min=numeric_limits<double>::max(), m_max=-m_min;
//    bool minMaxValid=false;
//  public:
//    using map<DataCube::Key, double>::find;
//    /// minimum value in hyperslice
//    double minVal() {
//      computeMinMax();
//      return m_min;
//    }
//    /// maximum value in hyperslice
//    double maxVal() {
//      computeMinMax();
//      return m_max;
//    }
//
//    void computeMinMax() {
//      if (!minMaxValid) {
//        m_min=numeric_limits<double>::max(); m_max=-m_min;
//        for (auto i=begin(); i!=end(); ++i) {
//          m_min=::min(m_min, i->second);
//          m_max=::max(m_max, i->second);
//        }
//        minMaxValid=true;
//      }
//    }
//
//    /// value to initialise an element if it doesn't exist
//    double init=0;
//    double& operator[](const Key& k) {
//      auto i=find(k);
//      if (i==end())
//        return insert(make_pair(k,init)).first->second;
//      else
//        return i->second;
//    }
//
//    HyperSlice() {}
//    /// initialise with a slice defined by sliceKey
//    HyperSlice(const DataCube::RawData& rawData, 
//               const map<unsigned,string>& sliceKey);
//     
//    void reduceAlongDim(unsigned dim, ReductionOp op);
//    // conditionally call f((*(this)[key]) if key exists
//    template <class F>
//    void doIfKeyExists(const Key& key, F f) {
//      auto it=find(key);
//      if (it!=end()) f(it->second);
//    }
//
//    // creates a sorting permutation give the sort data
//    void setupSortByValue(vector<size_t>& perm, const string& key, 
//                          bool reverse, bool swapAxes, 
//                          const SortedVector& labels) const;
//
//  };
//
//  HyperSlice::HyperSlice(const DataCube::RawData& rawData, 
//                         const map<unsigned,string>& sliceKey)
//  {
//    for (auto i=rawData.begin(); i!=rawData.end(); ++i)
//      {
//        DataCube::Key key;
//        for (unsigned j=0; j<i->first.size(); ++j)
//          {
//            auto it=sliceKey.find(j);
//            if (it!=sliceKey.end())
//              {
//                if (it->second!=i->first[j])
//                  goto dontAddItem;
//              }
//            else
//              key.push_back(i->first[j]);
//          }
//        if (const double *d=any_cast<double>(&i->second))
//          {
//            (*this)[key]=*d;
//            m_min=::min(m_min,*d);
//            m_max=::max(m_max,*d);
//          }
//      dontAddItem: continue;
//      }
//    minMaxValid=true;
//  }
//
//  void HyperSlice::reduceAlongDim(unsigned dim, ReductionOp op)
//  {
//    HyperSlice r, sumx /* for stddev */, n;
//    switch (op)
//      {
//        // here for completeness, not needed as r.init=0 by default
//      case sum: case av: case stddev: r.init=0; break;
//      case prod: r.init=1; break;
//      case min: r.init=numeric_limits<double>::max(); break;
//      case max: r.init=-numeric_limits<double>::max(); break;
//      }
//
//    for (auto i=begin(); i!=end(); ++i)
//      {
//        DataCube::Key key=i->first;
//        assert(dim < key.size());
//        key.erase(key.begin()+dim);
//        switch (op)
//          {
//          case sum: 
//            r[key]+=i->second;
//            break;
//          case prod:
//            r[key]*=i->second;
//            break;
//          case min:
//            r[key]=std::min(r[key],i->second);
//            break;
//          case max:
//            r[key]=std::max(r[key],i->second);
//            break;
//
//          case stddev:
//            r[key]+= i->second * i->second;
//          case av:
//            sumx[key]+=i->second;
//            n[key]+=1;
//            break;
//          }
//      } 
//    // postprocess
//    switch (op)
//      {
//      case av:
//        for (auto i=sumx.begin(); i!=sumx.end(); ++i)
//          r[i->first]=i->second/n[i->first];
//        break;
//      case stddev:
//        for (auto i=sumx.begin(); i!=sumx.end(); ++i)
//          {
//            double N=n[i->first];
//            double av=sumx[i->first]/N;
//            double& x=r[i->first];
//            x/=N;
//            x-=av*av;
//            x=sqrt(std::max(0.0,x)); // deal with roundoff error
//          }
//        break;
//      default:
//        break;
//      }
//
//    swap(r);
//    minMaxValid=false;
//  }
//
//  void HyperSlice::setupSortByValue
//  (vector<size_t>& perm, const string& key, bool reverse, 
//   bool swapAxes, const SortedVector& labels) const
//  {
//    assert(perm.size()==labels.size());
//    sort(perm.begin(), perm.end(), [&](unsigned i, unsigned j) {
//        Key keyi={key,labels[i]}, keyj={key,labels[j]};
//        if (swapAxes) {::swap(keyi[0],keyi[1]); ::swap(keyj[0],keyj[1]);} 
//        auto iti=find(keyi);
//	auto itj = find(keyj);
//	if (iti != end())
//          {
//            if (itj!=end())
//              return reverse? itj->second<iti->second: iti->second<itj->second;
//            return true; //non existing labels bigger than anything
//          }
//        return false;
//      });
//  }
//
//  void setupSortByPerm(DataCube::SortBy sortBy, 
//                       bool swapAxes, const HyperSlice& slice,
//                       SortedVector& labels, const string& key)
//  {
//    if (sortBy.type==DataCube::SortBy::byValue)
//      {
//        vector<size_t> perm(labels.size());
//        for (unsigned i=0; i<perm.size(); ++i) perm[i]=i;
//        // original label order needed for slice.setupSortByValue
//        labels.order(SortedVector::none); 
//        slice.setupSortByValue(perm,key,sortBy.reverse,swapAxes,labels);
//        labels.customPermutation(perm);
//      }
//
//  }
//
//  struct SliceIdx: public map<unsigned,string>
//  {
//    SliceIdx(const Ravel& ravel) {
//      for (unsigned h=0; h<ravel.handles.size(); ++h)
//        if (h!=ravel.xHandleId() && h!=ravel.yHandleId() &&
//            !ravel.handles[h].collapsed())
//          (*this)[h]=ravel.handles[h].sliceLabel();
//    }
//  };
//
//  // A Hyperslice that has been preprocessed by a Ravel
//  struct HyperSliceR: private SliceIdx, public HyperSlice
//  {    
//    using HyperSlice::begin;
//    using HyperSlice::end;
//    using HyperSlice::size;
//    HyperSliceR(const DataCube::RawData& rawData, const Ravel& ravel):
//      // note: initialisation order alert
//      SliceIdx(ravel), HyperSlice(rawData, *this) {
//      for (unsigned h = 0, dim = 0; h < ravel.handles.size(); ++h)
//        {
//          if (!SliceIdx::count(h)) dim++; // only collapsed or x/y handles are included in slice
//          if (ravel.handles[h].collapsed()) //every collapse undoes the previous dim increment
//            reduceAlongDim(--dim, ravel.handles[h].reductionOp);
//        }
//    }
//    size_t count(const DataCube::Key& k) const {return HyperSlice::count(k);}
//  };
//
//}


void DataCube::populateArray(ravel::Ravel& ravel)
{
  xh=ravel.xHandleId(); yh=ravel.yHandleId();
  Handle& xHandle = ravel.handles[xh];
  Handle& yHandle = ravel.handles[yh];

  m_minVal=numeric_limits<double>::max(); 
  m_maxVal=-m_minVal;

  //HyperSliceR slice(rawData, ravel);

  vector<string> axes{xHandle.description, yHandle.description};
  Key sliceLabels;
  for (auto& h: ravel.handles)
    if (&h!=&xHandle && &h!=&yHandle)
      if (h.collapsed())
        axes.push_back(h.description);
      else
        sliceLabels.emplace_back(h.description, h.sliceLabel());
  RawDataIdx slice=rawData.slice(axes, sliceLabels);

  RawData sliceData;
  if (slice.rank()>2)
    // perform reductions
    {
      bool firstReduction=true;
      for (auto& h: ravel.handles)
        if (&h!=&xHandle && &h!=&yHandle && h.collapsed())
          if (firstReduction)
            {
              // avoid copying data first time around
              sliceData=move
                (rawData.reduceAlong(slice.dim(h.description),slice,h.reductionOp));
              firstReduction=false;
            }
          else
            sliceData=move(sliceData.reduceAlong(sliceData.dim(h.description), sliceData,
                                                 h.reductionOp));
    }
  else
    sliceData=move(RawData(rawData,slice));
  
  // TODO reduction operations

  assert(m_sortBy[xh].rowCol<yHandle.sliceLabels.size() && 
         m_sortBy[yh].rowCol<xHandle.sliceLabels.size());

//  if (!xHandle.collapsed())
//    setupSortByPerm(m_sortBy[xh],yh>xh, slice, 
//                    xHandle.sliceLabels, 
//                    yHandle.sliceLabels[m_sortBy[xh].rowCol]);
//  if (!yHandle.collapsed())
//    setupSortByPerm(m_sortBy[yh],xh>yh, slice, 
//                    yHandle.sliceLabels, 
//                    xHandle.sliceLabels[m_sortBy[yh].rowCol]);

  // determine empty rows/columns
  xHandle.mask.clear(); yHandle.mask.clear();
//  if (filterOn || xHandle.displayFilterCaliper ||yHandle.displayFilterCaliper )
//    {
//      if (xHandle.collapsed()) 
//	{
//	  if (!yHandle.collapsed())
//	    for (unsigned j=0; j<yHandle.sliceLabels.size(); ++j)
//	      {
//		Key key{yHandle.sliceLabels[j]};
//		slice.doIfKeyExists(key, [&](double x) {
//		    m_minVal=min(m_minVal, x);
//		    m_maxVal=max(m_maxVal, x);
//		    if (((filterOn && filterMin<=x && filterMax>=x) || !filterOn) &&
//                        ((yHandle.displayFilterCaliper && j>=yHandle.sliceMin && j<=yHandle.sliceMax) ||
//                         !yHandle.displayFilterCaliper) && 
//                        (filterOn||yHandle.displayFilterCaliper))
//                      {
//                        yHandle.mask.insert(j);
//                      }
//		  });
//	      }
//	}
//      else if (yHandle.collapsed())
//	for (unsigned i=0; i<xHandle.sliceLabels.size(); ++i)
//	  {
//	    Key key{xHandle.sliceLabels[i]};
//	    slice.doIfKeyExists(key, [&](double x) {
//		m_minVal=min(m_minVal, x);
//		m_maxVal=max(m_maxVal, x);
//                if (((filterOn && filterMin<=x && filterMax>=x) || !filterOn) &&
//                    ((xHandle.displayFilterCaliper && i>=xHandle.sliceMin && i<=xHandle.sliceMax) ||
//                     !xHandle.displayFilterCaliper) && 
//                    (filterOn||xHandle.displayFilterCaliper))
//                  {
//                    xHandle.mask.insert(i);
//                  }
//	      });
//	  }
//      else
//	for (unsigned i=0; i<xHandle.sliceLabels.size(); ++i)
//	  for (unsigned j=0; j<yHandle.sliceLabels.size(); ++j)
//	    {
//	      Key key{xHandle.sliceLabels[i], yHandle.sliceLabels[j]};
//	      if (xh>yh) swap(key[0],key[1]);
//	      slice.doIfKeyExists(key, [&](double x) {
//		  m_minVal=min(m_minVal, x);
//		  m_maxVal=max(m_maxVal, x);
//                  if (((filterOn && filterMin<=x && filterMax>=x) || !filterOn) &&
//                      ((xHandle.displayFilterCaliper && i>=xHandle.sliceMin && i<=xHandle.sliceMax) ||
//                       !xHandle.displayFilterCaliper) && 
//                      ((yHandle.displayFilterCaliper && j>=yHandle.sliceMin && j<=yHandle.sliceMax) ||
//                       !yHandle.displayFilterCaliper))
//                    // note filterOn||xHandle.displayFilterCaliper||yHandle.displayFilterCaliper is true here
//                    {
//                      xHandle.mask.insert(i);
//                      yHandle.mask.insert(j);
//                    }
//		});
//	    }	    
//    }  
//  else
//    {
//      for (unsigned i=0; i<xHandle.sliceLabels.size(); ++i) xHandle.mask.insert(i);
//      for (unsigned i=0; i<yHandle.sliceLabels.size(); ++i) yHandle.mask.insert(i);
//    }
//
//  if (xHandle.collapsed()) 
//    if (yHandle.collapsed())
//      setDataElement(0,0,slice.begin()->second);
//    else
//      {
//	for (unsigned j=0,j1=0; j<yHandle.sliceLabels.size(); ++j)
//	  if (yHandle.mask.count(j))
//	    {
//	      Key key{yHandle.sliceLabels[j]};
//	      slice.doIfKeyExists(key, [=](double x) {
//		  filterDataElement(0,j1,x);});
//	      j1++;
//	    }
//      }
//  else if (yHandle.collapsed())
//    {
//      for (unsigned i=0, i1=0; i<xHandle.sliceLabels.size(); ++i)
//	if (xHandle.mask.count(i))
//	  {
//	    Key key{xHandle.sliceLabels[i]};
//	    slice.doIfKeyExists(key, [=](double x)
//				{filterDataElement(i1,0,x);});
//	    i1++;
//	  }
//    }
//  else
//    {
//      unsigned i=0; unsigned i1=0;
//      for (; i < xHandle.sliceLabels.size(); ++i)
//	if (xHandle.mask.count(i))
//	  {
//	    unsigned j = 0; unsigned j1 = 0;
//	    for (; j<yHandle.sliceLabels.size(); ++j)
//	      if (yHandle.mask.count(j))
//		{
//		  Key key{xHandle.sliceLabels[i], yHandle.sliceLabels[j]};
//		  if (xh>yh) swap(key[0],key[1]);
//		  slice.doIfKeyExists(key, [&](double x) {
//		      filterDataElement(i1, j1, x); });
//		  j1++;
//		}
//	    i1++;
//	  }
//    }

  if (xHandle.collapsed())
    {
      RawData rd=sliceData.reduceAlong(0,sliceData,xHandle.reductionOp);
      if (yHandle.collapsed())
        setDataElement(0,0,rd.reduce(yHandle.reductionOp, 0, rd.stride(0), rd.size()));
      else
        for (size_t i=0; i<rd.dim(0); ++i)
          {
            double v=rd[i*rd.stride(0)];
            if (!isnan(v))
              setDataElement(0,i,v);
          }
    }
  else if (yHandle.collapsed())
    {
      RawData rd=sliceData.reduceAlong(1,sliceData,yHandle.reductionOp);
      for (size_t i=0; i<rd.dim(0); ++i)
        {
          double v=rd[i*rd.stride(0)];
          if (!isnan(v))
            setDataElement(i,0,v);
        }
    }
  else
    for (size_t i=0; i<sliceData.dim(0); ++i)
      for (size_t j=0; j<sliceData.dim(1); ++j)
        {
          double v=sliceData[i*sliceData.stride(0) + j*sliceData.stride(1)];
          if (!std::isnan(v))
            setDataElement(i,j,v);
        }

  // populate the histogram
//  for (unsigned& x: histogram) x=0;
//  for (auto it=slice.begin(); it!=slice.end(); ++it)
//    {
//      size_t idx=size_t((histogram.size()*(it->second-slice.minVal()))/
//                        (slice.maxVal()-slice.minVal()));
//      if (idx>=histogram.size()) idx=histogram.size()-1;
//      histogram[idx]++;
//    }
}

// returns first position of v such that all elements in that or later
// positions are numerical or null
static size_t firstNumerical(const vector<any>& v)
{
  size_t r=0;
  for (size_t i=0; i<v.size(); ++i)
    if (const string* s = any_cast<string>(&v[i]))
      if (!s->empty())
        r=i+1;
  return r;
}

// returns true if all elements of v after start are empty
static bool emptyTail(const vector<any>& v, size_t start)
{
  for (size_t i=start; i<v.size(); ++i)
    {
      if (v[i].type()==typeid(double)) return false;
      if (const string* s = any_cast<string>(&v[i]))
        if (!s->empty()) return false;
    }
  return true;
}

// counts number of non empty entries on a line
static size_t numEntries(const vector<any>& v)
{
  size_t c=0;
  for (const any& x: v)
    if (x.type()==typeid(double) || !str(x).empty())
      c++;
  return c;
}

DataSpec DataCube::initDataSpec(Tokeniser& tok)
{
  DataSpec spec;
  vector<any> line;
  vector<size_t> starts;
  size_t nCols=0;
  size_t firstEmpty=numeric_limits<size_t>::max();
  //  vector<unsigned> possibleComments;
  while (!(line=tok.getLine()).empty())
    {
      starts.push_back(firstNumerical(line));
      nCols=max(nCols, line.size());
      // this is to detect if an empty header line is used to carry colAxes labels

      // treat a single item on its own as a comment
      if (numEntries(line)==1)
        spec.commentRows.insert(unsigned(starts.size()-1));
      else
        if (starts.size()-1 < firstEmpty && starts.back()<nCols && emptyTail(line, starts.back()))
          firstEmpty=starts.size()-1;
    }
  // compute average of starts, then look for first row that drops below average
  double sum=0;
  for (unsigned long i=0; i<starts.size(); ++i) 
    if (spec.commentRows.count(i)==0)
      sum+=starts[i];
  double av=sum/(starts.size()-spec.commentRows.size());
  for (spec.nRowAxes=0; 
       spec.commentRows.count(spec.nRowAxes) ||
         (starts.size()>spec.nRowAxes && starts[spec.nRowAxes]>av); 
       ++spec.nRowAxes);
  for (size_t i=spec.nRowAxes; i<starts.size(); ++i)
    spec.nColAxes=max(spec.nColAxes,starts[i]);
  // if more than 1 data column, treat the first row as an axis row
  if (spec.nRowAxes==0 && nCols-spec.nColAxes>1)
    spec.nRowAxes=1;

  // treat single entry rows as comments
  if (!spec.commentRows.empty() && 
      spec.nColAxes == 1 && starts[*spec.commentRows.rbegin()] == 1)
    {
      firstEmpty = *spec.commentRows.rbegin();
      spec.commentRows.erase(firstEmpty);
    }
  if (firstEmpty==spec.nRowAxes) ++spec.nRowAxes; // allow for possible colAxes header line
  return spec;
}



void DataCube::initRavel(ravel::Ravel& ravel) const
{
  ravel.clear();
  for (size_t i=0; i<dimNames.size(); ++i)
    ravel.addHandle(dimNames[i], dimLabels()[i]);
  // if some axes are read in as rows, then set the xHandle to the
  // first of these, otherwise take the second
  if (dimNames.size()>1)
    ravel.setXYHandles( leastRowAxis<dimNames.size()? leastRowAxis: 1, 0);
}

void DataCube::filterDataElement(size_t col, size_t row, double v)
{
  if (!filterOn)
    {
      m_minVal=min(m_minVal, v);
      m_maxVal=max(m_maxVal, v);
    }
  if (v>=filterMin && v<=filterMax)
    setDataElement(col,row,v);
}

