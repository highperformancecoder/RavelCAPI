/*
  Ravel C API. © Ravelation Pty Ltd 2023
*/

#ifndef RAVELCIVITA_H
#define RAVELCIVITA_H
#include "capiCivita.h"
#include "tensorInterface.h"

/// Wrapper around Civita classes, providing CAPICivita interface

namespace ravel
{
  // wrap a tensor, implement a CAPITensor
  class CAPITensor: public ::CAPITensor
  {
    const civita::ITensor& m_tensor;
    static const civita::ITensor& tensor(const ::CAPITensor* x) {return static_cast<const CAPITensor*>(x)->m_tensor;}
    std::string m_hypercube;
    static const char* s_hypercube(const struct ::CAPITensor* x) {
      auto& hc=const_cast<CAPITensor*>(static_cast<const CAPITensor*>(x))->m_hypercube;
      hc=tensor(x).hypercube().json();
      return hc.c_str();
    }
    static size_t s_size(const struct ::CAPITensor* x)
    {return tensor(x).size();}
    static size_t s_indexSize(const struct ::CAPITensor* x)
    {return tensor(x).index().size();}
    static size_t s_index(const struct ::CAPITensor* x, size_t i)
    {return tensor(x).index()[i];}
    static double s_at(const struct ::CAPITensor* x,size_t i)
    {return tensor(x)[i];}
  public:
    CAPITensor(const civita::ITensor& tensor): m_tensor(tensor)
    {
      hypercube=s_hypercube;
      size=s_size;
      indexSize=s_indexSize;
      index=s_index;
      at=s_at;
    }
  };

  // wrap a CAPITensor, implement an ITensor
  class TensorWrap: public civita::ITensor
  {
    const ::CAPITensor& tensor;
  public:
    TensorWrap(const ::CAPITensor& tensor): tensor(tensor) {
      std::set<size_t> idx;
      for (size_t i=0; i<tensor.indexSize(&tensor); ++i)
        idx.insert(tensor.index(&tensor,i));
      m_index=idx;
      hypercube(civita::Hypercube::fromJson(tensor.hypercube(&tensor)));
    }
    double operator[](std::size_t i) const override {return tensor.at(&tensor,i);}
    Timestamp timestamp() const override {return {};}
  };
  
}

#endif
