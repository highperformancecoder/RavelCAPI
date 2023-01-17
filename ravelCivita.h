/*
  Ravel C API. © Ravelation Pty Ltd 2023
*/

#ifndef CAPICIVITA_H
#define CAPICIVITA_H
#include "capiCivita.h"

/// Wrapper around Civita classes, providing CAPICivita interface

namespace ravel
{
  // wrap a tensor, implement a CAPITensor
  struct CAPITensor: public ::CAPITensor
  {
    const civita::ITensor& tensor;
    std::string hypercube;
    CAPITensor(const civita::ITensor& tensor): tensor(tensor)
    {
      hypercube=s_hypercube;
      size=s_size;
      index=s_index;
      at=s_at;
    }
    static const char* s_hypercube(struct CAPITensor* x) {
      x->hypercube=x->tensor.hypercube().json();
      return x->hypercube.c_str();
    }
    static size_t s_size(struct  CAPITensor* x)
    {return x->tensor.size();}
    size_t (*index)(struct  CAPITensor* x, size_t i)
    {return x->tensor.index()[i];}
    double (*at)(struct  CAPITensor*,size_t i)
    {return x->tensor[i];}
  };

  // wrap a CAPITensor, implement an ITensor
  class TensorWrap: public ITensor
  {
    const ::CAPITensor& tensor;
    TensorWrap(const ::CAPITensor& tensor): tensor(tensor) {
      std::set  idx;
      for (size_t i=0; i<tensor.indexSize(); ++i)
        idx.insert(tensor.index(i));
      m_index=idx;
      hypercube(civita::fromJson(tensor.hypercube()));
    }
    double operator[](std::size_t i) const override {return tensor->at(i);}
    Timestamp timestamp() const override {return {};}
  };
  
}

#endif
