#include "rawData.h"

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
      auto& idx=indices[k->second];

      indicesByName[i]=indices.size();
      indices.emplace_back();
      auto& j=indices.back();
      j.stride=m_size;
      for (auto& i: idx.idx)
        j.idx[i.first]=(i.second/idx.stride)*m_size;
      m_size*=idx.idx.size();
    }
}

size_t RawDataIdx::idx(const Key& key) const
{
  size_t idx=0;
  for (auto& i: key)
    {
      auto j=indicesByName.find(i.first);
      if (j==indicesByName.end())
        throw InvalidKey();
      auto k=indices[j->second].idx.find(i.second);
      if (k==indices[j->second].idx.end())
        throw InvalidKey();
      idx+=indices[j->second].stride*k->second;
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


//HyperSlice::HyperSlice(const RawDataIdx& idx,const vector<string>& axes, const Key& fixedLabels): offset(0)
//{
//  offset=idx.idx(fixedLabels);
//    
//  vector<pair<string,vector<string>>> labels;
//  for (auto& axis: axes)
//    {
//      auto i=indices.find(axis);
//      if (i==indices.end()) throw InvalidKey();
//      auto k=labels.emplace(axis, vector<string>());
//      for (auto& j: i->second.idx)
//        k->second.push_back(j->first);
//      sizeAndStrides.emplace_back(i->second.idx.size(), i->second.stride);
//    }
//  r.idx=RawDataIdx(labels);
//}
