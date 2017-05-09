#include "rawData.h"
#ifdef CLASSDESC
#include <classdesc_epilogue.h>
#endif
#ifdef ECOLAB_LIB
#include <ecolab_epilogue.h>
#endif

using namespace ravel;
using namespace std;

RawDataIdx::RawDataIdx(const vector<pair<string,vector<string>>>& labels) 
{
  if (labels.empty()) return;
  m_size=1;
  for (auto& i: labels)
    {
      indicesByName[i.first]=indices.size();
      indices.emplace_back();
      auto& j=indices.back();
      j.stride=m_size;
      for (size_t k=0; k<i.second.size(); k++)
        j.idx[i.second[k]]=k*m_size;
      m_size*=i.second.size();
    }
}

RawDataIdx::RawDataIdx(const RawDataIdx& x, const std::vector<std::string>& axes)
{
  m_size=1;
  for (auto& i: axes)
    {
      auto k=x.indicesByName.find(i);
      if (k==x.indicesByName.end()) throw InvalidKey();
      auto& idx=x.indices[k->second];

      indicesByName[i]=indices.size();
      indices.emplace_back();
      auto& j=indices.back();
      j.stride=m_size;
      for (auto& ii: idx.idx)
        j.idx[ii.first]=(ii.second/idx.stride)*m_size;
      m_size*=idx.idx.size();
    }
}

void RawDataIdx::normalise()
{
  m_offset=0;
  m_size=1;
  for (auto& i: indices)
    {
      for (auto& j: i.idx)
        j.second = j.second/i.stride*m_size;
      i.stride=m_size;
      m_size*=i.idx.size();
    }
}


size_t RawDataIdx::idx(const Key& key) const
{
  size_t idx=m_offset;
  for (auto& i: key)
    {
      auto j=indicesByName.find(i.axis);
      if (j==indicesByName.end())
        throw InvalidKey();
      auto k=indices[j->second].idx.find(i.slice);
      if (k==indices[j->second].idx.end())
        throw InvalidKey();
      idx+=k->second;
    }
  return idx;
}

const RawDataIdx::Idx& RawDataIdx::index(const string& axis) const
{
  auto i=indicesByName.find(axis);
  if (i==indicesByName.end())
    throw InvalidKey();
  return indices[i->second];
}

RawDataIdx RawDataIdx::slice
(const vector<string>& axes, const Key& fixedLabels) const
{
  RawDataIdx r;
  r.m_offset=idx(fixedLabels);
  r.m_size=1;
  for (auto& axis: axes)
    {
      auto i=indicesByName.find(axis);
      if (i==indicesByName.end()) throw InvalidKey();
      r.indicesByName[axis]=r.indices.size();
      r.indices.push_back(indices[i->second]);
      r.m_size*=r.indices.back().idx.size();
    }
  return r;
}

RawDataIdx RawDataIdx::removeDimension(size_t axis) const
{
  vector<pair<string,size_t>> axesNamesOrdered(indicesByName.begin(), indicesByName.end());

  // sort into original axes order
  sort(axesNamesOrdered.begin(), axesNamesOrdered.end(),
       [](const pair<string,size_t>& x, const pair<string,size_t>& y)
       {return x.second<y.second;});

  vector<string> axes;
  for (size_t i=0; i<axesNamesOrdered.size(); ++i)
    if (i!=axis)
      axes.push_back(axesNamesOrdered[i].first);

  return RawDataIdx(*this,axes);
}

RawDataIdx RawDataIdx::collapseAxis(size_t i) const
{
  RawDataIdx r(*this);
  r.indices[i].idx.clear();
  r.indices[i].idx["result"]=0;
  r.normalise();
  return r;
}

template <class Op>
double reduceImpl(const RawData& x, Op op, double r, size_t offset,
                       size_t stride, size_t dim)
{
  bool valid=false;
  for (size_t i=offset; i<offset+stride*dim; i+=stride)
    {
      double v=x[i];
      if (isfinite(v))
        {
          valid=true;
          op(r,v);
        }
    }
  return valid? r: nan("");
}

double RawData::reduce(Op::ReductionOp op, size_t offset,
                       size_t stride, size_t dim) const
{
  double r;
  switch (op)
    {
    case Op::sum:
      return reduceImpl(*this, [](double& r,double v){r+=v;}, 0, offset,stride,dim);

    case Op::prod:
      return reduceImpl(*this, [](double& r,double v){r*=v;}, 1, offset,stride,dim);

    case Op::av:
      {
        r=0;
        size_t c=0;
        for (size_t i=offset; i<offset+stride*dim; i+=stride)
          {
            double v=(*this)[i];
            if (isfinite(v))
              {
                r+=v;
                c++;
              }
          }
        return c>0? r/c: nan("");
      }
    case Op::stddev:
      {
        double sum=0, sumsq=0;
        size_t c=0;
        for (size_t i=offset; i<offset+stride*dim; i+=stride)
          {
            double v=(*this)[i];
            if (isfinite(v))
              {
                sum+=v;
                sumsq+= v*v;
                c++;
              }
          }
        if (c==0) return nan("");
        sum/=c;
        return sqrt(max(0.0, sumsq/c-sum*sum));
      }

    case Op::min:
      return reduceImpl(*this, [](double& r,double v){r=min(r,v);},
                    numeric_limits<double>::max(), offset,stride,dim);
      
    case Op::max:
      return reduceImpl(*this, [](double& r,double v){r=max(r,v);},
                    -numeric_limits<double>::max(), offset,stride,dim);

    default:
      assert(false);
      return 0;
    }
}  

RawData::RawData(const RawData& x, const RawDataIdx& slice): RawDataIdx(slice)
{
  normalise();
  data.resize(size(),nan(""));
  vector<SizeStride> sizeStride;
  if (rank()==0)
    {
      if (x.size()>0)
        data[0]=x[0];
    }
  else
    {
      for (size_t i=0; i<slice.rank(); i++)
        sizeStride.emplace_back(slice.dim(i),slice.stride(i));
      size_t i=0;
      apply(slice.offset(),sizeStride,[&](size_t j) {data[i++]=x[j];});
    }
}
  
RawData RawData::reduceAlong(size_t axis, const RawDataIdx& slice,
                             Op::ReductionOp op, bool outputHandle) const
{
  RawData r(outputHandle? slice.collapseAxis(axis): slice.removeDimension(axis));
  assert(outputHandle && r.rank()==slice.rank() || !outputHandle && r.rank()==slice.rank()-1);
  vector<SizeStride> sizeStride;
  for (size_t i=0; i<slice.rank(); i++)
    if (i!=axis)
      sizeStride.emplace_back(slice.dim(i),slice.stride(i));

  size_t i=0;
  apply(slice.offset(), sizeStride, [&](size_t j) {
      r[i++]=reduce(op, j, slice.stride(axis), slice.dim(axis));
    });
  return r;
}

