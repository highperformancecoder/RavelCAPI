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
#include <iostream>

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
      boost::escaped_list_separator<char> csvParser('\\',separator,'"');//
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
                      axisLabelSet[dim].insert(UniqueString(key.back().slice,row));
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

  if (tmpData.empty())
    throw RavelError("apparently empty data file loaded");

  // check for uniqueness of dimension names
  set<string> uniqDimNames(dimNames.begin(), dimNames.end());
  if (uniqDimNames.size()<dimNames.size())
    throw RavelError("non-unique dimension names provided - are you missing a header?");
  
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
  // if each data point lies on its own line, then we have overestimated leastRowAxis
  if (leastRowAxis==dimNames.size()) --leastRowAxis;
}

void setupSortByPerm(DataCube::SortBy sortBy, size_t axis, size_t otherAxis,
                     const RawData& slice, SortedVector& labels)
  {
    if (sortBy.type==DataCube::SortBy::byValue)
      {
        vector<size_t> perm(labels.size());
        for (unsigned i=0; i<perm.size(); ++i) perm[i]=i;
        size_t offs=slice.offset() + sortBy.rowCol*slice.stride(otherAxis);
        sort(perm.begin(), perm.end(), [&](size_t i, size_t j){
            if (sortBy.reverse) swap(i,j);
            return slice[offs + i*slice.stride(axis)] <
              slice[offs + j*slice.stride(axis)];
              });
        // invert permutation
        vector<size_t> perm1(perm.size());
        for (unsigned i=0; i<perm.size(); ++i)
          perm1[perm[i]]=i;
        labels.customPermutation(perm1);
      }

  }


void DataCube::hyperSlice(RawData& sliceData, Ravel& ravel) const
{
  // apply partial reductions, if any
  bool noReductions=true;
  RawData partReducedData;
  for (auto& h: ravel.handles)
    {
      // apply any caliper restrictions to data
      if (h.displayFilterCaliper && (h.sliceMin>0 || h.sliceMax<h.sliceLabels.size()))
        {
          ApplyCalipers ac(h.sliceMin, h.sliceMax);
          if (noReductions)
            {
              // avoid copying data first time around
              partReducedData=rawData.partialReduce(rawData.axis(h.description),ac);
              noReductions=false;
            }
          else
            partReducedData=partReducedData.partialReduce(partReducedData.axis(h.description),ac);
        }
      for (auto& pred: h.partialReductions())
        if (noReductions)
          {
            // avoid copying data first time around
            partReducedData=rawData.partialReduce(rawData.axis(h.description),*pred);
            noReductions=false;
          }
        else
          partReducedData=partReducedData.partialReduce(partReducedData.axis(h.description),*pred);
    }
  if (noReductions)
    hyperSliceAfterPartialReductions(sliceData, ravel, rawData);
  else
    hyperSliceAfterPartialReductions(sliceData, ravel, partReducedData);
}
  
void DataCube::hyperSliceAfterPartialReductions(RawData& sliceData, Ravel& ravel,const RawData& rawData) const
{
  vector<string> axes;
  Key sliceLabels;
  for (auto i: ravel.handleIds)
    if (i<ravel.handles.size())
      axes.push_back(ravel.handles[i].description);
  for (auto& h: ravel.handles)
    if (!ravel.isOutputHandle(h))
      {
        if (h.collapsed())
          axes.push_back(h.description);
        else
          sliceLabels.emplace_back(h.description, h.sliceLabel());
      }

  RawDataIdx slice=rawData.slice(axes, sliceLabels);

  // perform reductions
  bool noReductions=true;
  for (auto& h: ravel.handles)
    if (h.collapsed())
      {
        if (noReductions)
          {
            // avoid copying data first time around
            sliceData=move
              (rawData.reduceAlong(slice.axis(h.description),slice,h.reductionOp,ravel.isOutputHandle(h)));
            noReductions=false;
          }
        else
          sliceData=move(sliceData.reduceAlong(sliceData.axis(h.description), sliceData,
                                                   h.reductionOp,ravel.isOutputHandle(h)));
      }

  if (noReductions)
    sliceData=move(RawData(rawData,slice));
}

void DataCube::populateArray(Ravel& ravel)
{
  // TODO: handle ranks other than 2.
  if (ravel.handleIds.size()<2) return;
  xh=ravel.handleIds[0]; yh=ravel.handleIds[1];
  Handle& xHandle = ravel.handles[xh];
  Handle& yHandle = ravel.handles[yh];

  m_minVal=numeric_limits<double>::max(); 
  m_maxVal=-m_minVal;

  RawData sliceData(hyperSlice(ravel));
  if (sliceData.rank()!=2)
    return; 
  
  // TODO reduction operations

  assert(m_sortBy[xh].rowCol<yHandle.sliceLabels.size() && 
         m_sortBy[yh].rowCol<xHandle.sliceLabels.size());

  if (!xHandle.collapsed())
    setupSortByPerm(m_sortBy[xh],0,1,sliceData, xHandle.sliceLabels);
  if (!yHandle.collapsed())
    setupSortByPerm(m_sortBy[yh],1,0,sliceData, yHandle.sliceLabels);

  for (size_t i=0; i<sliceData.size(); ++i)
    if (isfinite(sliceData[i]))
      {
        m_minVal=min(m_minVal,sliceData[i]);
        m_maxVal=max(m_maxVal,sliceData[i]);
      }

  // prepare empty row/column masks
  xHandle.mask.clear(); yHandle.mask.clear();

  size_t xoffs=xHandle.displayFilterCaliper? xHandle.sliceMin: 0;
  size_t yoffs=yHandle.displayFilterCaliper? yHandle.sliceMin: 0;
  size_t xmax=sliceData.dim(0)+xoffs;
  size_t ymax=sliceData.dim(1)+yoffs;
  
  // set up masks to eliminate empty rows/cols
  set<size_t> validX, validY;
  for (size_t i=xoffs; i<xmax; ++i)
    for (size_t j=yoffs; j<ymax; ++j)
      if (!isnan(sliceData[(xHandle.sliceLabels.idx(i)-xoffs)*sliceData.stride(0)
                           + (yHandle.sliceLabels.idx(j)-yoffs)*sliceData.stride(1)]))
        {validX.insert(i); validY.insert(j);}

  for (size_t i=0; i<xHandle.sliceLabels.size(); ++i)
    if (!validX.count(i)) xHandle.mask.insert(i);
  for (size_t i=0; i<yHandle.sliceLabels.size(); ++i)
    if (!validY.count(i)) yHandle.mask.insert(i);

  for (size_t i=xoffs, i1=0; i<xmax; ++i)
    if (validX.count(i))
      {
        for (size_t j=yoffs, j1=0; j<ymax; ++j)
          if (validY.count(j))
            {
              double v=sliceData[(xHandle.sliceLabels.idx(i)-xoffs)*sliceData.stride(0)
                                 + (yHandle.sliceLabels.idx(j)-yoffs)*sliceData.stride(1)];
              if (!isnan(v))
                filterDataElement(i1,j1,v);
              j1++;
            }
        i1++;
      }

  // populate the histogram
  for (unsigned& x: histogram) x=0;
  for (size_t i=0; i<sliceData.size(); ++i)
    if (isfinite(sliceData[i]))
      {
        size_t idx=size_t((histogram.size()*(sliceData[i]-m_minVal))/
                          (m_maxVal-m_minVal));
        if (idx>=histogram.size()) idx=histogram.size()-1;
        histogram[idx]++;
      }
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
  if (dimNames.size()>1 && ravel.rank()>=1)
    {
      auto i=find(ravel.handleIds.begin(), ravel.handleIds.end(), leastRowAxis);
      if (i!=ravel.handleIds.end())
        swap(*i,ravel.handleIds.front());
      else
        ravel.handleIds.front()=leastRowAxis;
    }
  ravel.redistributeHandles();
}

void DataCube::filterDataElement(size_t col, size_t row, double v)
{
  if (v>=filterMin && v<=filterMax)
    setDataElement(col,row,v);
}

