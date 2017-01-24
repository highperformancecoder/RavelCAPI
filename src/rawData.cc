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

