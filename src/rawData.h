/**
   The RawData class represents the data indexed by string axes and labels. The actual data is stored in contiguous memory

*/

#ifndef RAWDATA_H
#define RAWDATA_H

#include <classdesc_access.h>
#include "ravel.h" // for Op
#include <string>
#include <map>
#include <vector>
#include <exception>
#include <assert.h>
#include <math.h>

namespace ravel
{
  struct InvalidKey: public std::exception
  {
    const char* what() const noexcept {return "invalid key";}
  };

  typedef std::vector<std::pair<std::string,std::vector<std::string>>> 
    LabelsVector;
  typedef std::vector<std::pair<std::string, std::string>> Key;

  class RawDataIdx
  {
  public:
    struct Idx
    {
      std::map<std::string,size_t> idx;
      size_t stride;
    };
  private:

    std::map<std::string, size_t> indicesByName;
    std::vector<Idx> indices;
    size_t m_size, m_offset=0;

    const Idx& index(const std::string& axis) const;
    CLASSDESC_ACCESS(RawDataIdx);

  public:
    RawDataIdx() {}
    RawDataIdx(const LabelsVector& labels);
    RawDataIdx(const RawDataIdx& x, const std::vector<std::string>& axes);

    /// reorder indices so that this slice refers to contiguous data
    void normalise();
    
    void clear() {indicesByName.clear(); indices.clear(); m_size=0;}

    /// returns number of elements in this slice
    size_t size() const {return m_size;}
    /// returns rank of slice (number of axes)
    size_t rank() const {return indices.size();}
    /// returns the index into contiguous data of \a key
    size_t idx(const Key& key) const;
    /// return stride of \a axis
    size_t stride(const std::string& axis) const {
      return index(axis).stride;
    }
    /// return stride of \a axis
    size_t stride(size_t axis) const {
      return indices[axis].stride;
    }
    /// return dimension (size) along an \a axis
    size_t dim(size_t axis) const {
      return indices[axis].idx.size();
    }
    size_t dim(const std::string& axis) const {
      auto i=indicesByName.find(axis);
      if (i==indicesByName.end()) throw InvalidKey();
      return i->second;
    }

    /// return offset into rawdata of first element of this slice
    size_t offset() const {return m_offset;}
    
    struct SizeStride
    {
      size_t size, stride;
      SizeStride(size_t sz=0, size_t st=0): size(sz), stride(st) {} 
    };
  
    SizeStride sizeStride(const std::string& axis) const {
      auto& i=index(axis);
      SizeStride r(i.idx.size(),i.stride);
      return r;
    }

    /// returns an idx describing a slice with fixed labels \a
    /// fixedLabels, with dimensions in order given by axes
    RawDataIdx slice(const std::vector<std::string>& axes,
                     const Key& fixedLabels) const;

    RawDataIdx removeDimension(size_t) const;
  };

  /** apply functional \a f to a range of elements in a contiguous data range
  
   this peculiar way of doing things is to emulate a nested for
   loop with an arbitrary number of nestings

   For example a rank 3 apply call can be interpreted as
     auto& s=sizeAndStrides;
     for (size_t i=0; i<s[0].size; i++)
      for (size_t j=0; j<s[1].size; j++)
       for (size_t k=0; k<s[2].size; k++)
         f(offset + i*s[0].stride + j*s[1].stride + k*s[2].stride]);
  */

  template <typename F>
  void apply(size_t offset, std::vector<RawDataIdx::SizeStride> sizeAndStrides,
             F f)
  {
    assert(!sizeAndStrides.empty());
    size_t size=sizeAndStrides.back().size;
    size_t stride=sizeAndStrides.back().stride; 
    sizeAndStrides.pop_back();
    for (size_t i=offset; i<offset+size*stride; i+=stride)
      if (sizeAndStrides.empty())
        f(i);
      else
        apply(i, sizeAndStrides, f);
  }

  class RawData: public RawDataIdx
  {
    std::vector<double> data;
    CLASSDESC_ACCESS(RawData);

  public:
    double& operator[](const Key& key) {return data[idx(key)];}
    double operator[](const Key& key) const {return data[idx(key)];}
    double& operator[](size_t i) {return data[i];}
    double operator[](size_t i) const {return data[i];}

    RawData() {}

    /** @{
        Create an empty RawData with structure given by \a x
    */
    RawData(const RawDataIdx& x): RawDataIdx(x) {data.resize(size(),nan(""));}
    RawData(const RawDataIdx& x, const std::vector<std::string>& a): 
      RawDataIdx(x,a) {data.resize(size(),nan(""));}
    RawData(RawDataIdx&& x): RawDataIdx(x) {data.resize(size(),nan(""));}
    ///@}
 
    /// produces a concrete RawData given a slice through some other data
    RawData(const RawData& x, const RawDataIdx& slice);
    
    void clear() {RawDataIdx::clear(); data.clear();}

    /// Return the reduction of \a n elements starting at \a offset,
    /// and incrementing by \a stride
    double reduce(Op::ReductionOp op, size_t offset, size_t stride, size_t n) const;

    /// Produce a new slice by reducing along dimension axis of the slice given by \a slice
    RawData reduceAlong(size_t axis, const RawDataIdx& slice,
                                 Op::ReductionOp op) const;
  };

}

#if defined(CLASSDESC) || defined(ECOLAB_LIB)
#include "rawData.cd"
#endif
#endif
