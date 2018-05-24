#include "rawData.h"
#ifdef CLASSDESC
#include <classdesc_epilogue.h>
#endif
#ifdef ECOLAB_LIB
#include <ecolab_epilogue.h>
#endif

#include <iostream>
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
      if (k==x.indicesByName.end()) throw InvalidKey(i);
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

LabelsVector RawDataIdx::labelsVector() const
{
  LabelsVector r;
  for (auto& i: indicesByName)
    {
      r.emplace_back(i.first,vector<string>{});
      for (auto& j: indices[i.second].idx)
        r.back().second.push_back(j.first);
      sort(r.back().second.begin(),r.back().second.end(),
           [&](const string& x,const string& y)
           {
             auto& m=indices[i.second].idx;
             auto i=m.find(x);
             auto j=m.find(y);
             return i!=m.end() && j!=m.end() && i->second<j->second;
           });
    }
  sort(r.begin(), r.end(),
       [&](const LabelsVector::value_type& x,const LabelsVector::value_type& y)
       {
         auto i=indicesByName.find(x.first);
         auto j=indicesByName.find(y.first);
         return i!=indicesByName.end() && j!=indicesByName.end() &&
           i->second<j->second;
       });
  return r;
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
        throw InvalidKey(key);
      auto k=indices[j->second].idx.find(i.slice);
      if (k==indices[j->second].idx.end())
        throw InvalidKey(key);
      idx+=k->second;
    }
  return idx;
}

const RawDataIdx::Idx& RawDataIdx::index(const string& axis) const
{
  auto i=indicesByName.find(axis);
  if (i==indicesByName.end())
    throw InvalidKey(axis);
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
      if (i==indicesByName.end()) throw InvalidKey(axis);
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
  for (size_t i=offset; i<offset+stride*dim && i<x.size(); i+=stride)
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
        for (size_t i=offset; i<offset+stride*dim && i<size(); i+=stride)
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
        for (size_t i=offset; i<offset+stride*dim && i<size(); i+=stride)
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
      apply(slice.offset(),sizeStride,[&](size_t j) {
          if (j<x.size())
            data[i++]=x[j];
        });
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

RawData RawData::partialReduce(size_t axis, PartialReduction& p) const
{
  auto lv=labelsVector();
  assert(axis<lv.size());
  vector<string> labels;
  for (auto i: p.indices(dim(axis))) labels.push_back(lv[axis].second[i]);
  lv[axis].second.swap(labels);
  RawData r{RawDataIdx(lv)};
  for (size_t i=0,j=0; i<size();
       i+=dim(axis)*stride(axis), j+=r.dim(axis)*stride(axis))
    for (size_t k=0; k<stride(axis) && j+k<r.size() && i+k<size(); ++k)
      {
        assert(j+k<r.size());
        assert(i+k<data.size());
        p(&r[j+k],&data[i+k],stride(axis),dim(axis));
      }
  return r;
}

template <class F>
void RawData::orderedApply(size_t offset, std::vector<const SortedVector*> order, F f)
{
  if (order.size()==1)
    for (size_t i=0; i<order[0]->size(); ++i)
      f(offset+order[0]->idx(i)*stride(0));
  else
    {
      auto& o=*order.back();
      order.pop_back();
      for (size_t i=0; i<o.size(); ++i)
        orderedApply(offset + o.idx(i)*stride(order.size()), order, f);
    }
}

void RawData::reorder(RawData& r, const std::vector<const SortedVector*>& o)
{
#ifndef NDEBUG
  assert(rank()==o.size());
  for (size_t i=0; i<rank(); ++i)
    {
      assert(o[i]);
      assert(dim(i)==o[i]->filteredSize());
    }
#endif
  r=static_cast<RawDataIdx&>(*this);
  r.normalise();
  size_t idx=0;
  orderedApply(0,o,[&](size_t i){r[idx++]=data[i];});
}
