#ifndef PARTIALREDUCTION_H
#define PARTIALREDUCTION_H

/*
  Supports the partial reduction concept, which is a vector-vector
  transformation of an axis.
 */

#include <vector>
#include <limits>
#if defined(CLASSDESC) || defined(ECOLAB_LIB)
#include <polyBase.h>
#include <polyXMLBase.h>
#else
#include "dummyPoly.h"
#endif

namespace ravel
{
  enum class PartialReductionType {bin,scan,change};
  struct PartialReduction:
    public classdesc::PolyBase<PartialReductionType>,
                           classdesc::PolyXML<PartialReduction>
    
  {
//    struct IndexVal
//    {
//      std::size_t index; ///< sliceindex of this value
//      double value=0;
//      IndexVal() {}
//      IndexVal(std::size_t index,double value): index(index), value(value) {}
//    };
    /// transform data along a slice
    virtual void operator()(double* dest, const double* src, size_t stride, size_t num) const=0;
    /// return indices corresponding to slice labels in the transformed axis
    /// @param N - dimension of axis
    virtual std::vector<size_t> indices(size_t N) const=0;
    static PartialReduction* create(PartialReductionType);
  };

  // group identity value
  template <class T> inline double identity(typename T::Op x) {
    switch (x)
      {
      case T::add: return 0;
      case T::multiply: return 1;
      default: return 0;
      }
  }
  
  struct Bin: public classdesc::Poly<Bin, PartialReduction>
  {
    PartialReductionType type() const override
    {return PartialReductionType::bin;}
    std::size_t binSize;
    enum Op {add, multiply};
    Op op;
    Bin(Op op=add, std::size_t binSize=1): binSize(binSize), op(op) {}
    void operator()(double*, const double*, size_t, size_t) const override;
    std::vector<size_t> indices(size_t) const override;
  };
    
  struct Scan: public classdesc::Poly<Scan,PartialReduction>
  {
    PartialReductionType type() const override
    {return PartialReductionType::scan;}
    std::size_t window;
    enum Op {add, multiply};
    Op op;
    Scan(Op op=add, std::size_t window=std::numeric_limits<std::size_t>::max()):
      op(op), window(window) {}
    void operator()(double*, const double*, size_t, size_t) const override;
    std::vector<size_t> indices(size_t) const override;
  };
    
  struct Change: public classdesc::Poly<Change,PartialReduction>
  {
    PartialReductionType type() const override
    {return PartialReductionType::change;}
    std::size_t offset;
    enum Op {subtract, divide, percent, relative};
    Op op;
    Change(Op op=subtract, std::size_t offset=1): op(op), offset(offset) {}
    void operator()(double*, const double*, size_t, size_t) const override;
    std::vector<size_t> indices(size_t) const override;
  };

}

#if defined(CLASSDESC) || defined(ECOLAB_LIB)
#include "partialReduction.cd"
#endif

#endif
