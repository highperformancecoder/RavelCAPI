/**
   The RawData class represents the data indexed by string axes and labels. The actual data is stored in contiguous memory

*/

#ifndef RAWDATA_H
#define RAWDATA_H

#include <classdesc_access.h>

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
    size_t m_size;

    const Idx& index(const std::string& axis) const;
    CLASSDESC_ACCESS(RawDataIdx);

  public:
    RawDataIdx() {}
    RawDataIdx(const LabelsVector& labels);
    RawDataIdx(const RawDataIdx& x, const std::vector<std::string>& axes);

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

  };

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
    RawData(const RawDataIdx& x): RawDataIdx(x) {data.resize(size(),nan(""));}
    RawData(const RawDataIdx& x, const std::vector<std::string>& a): 
      RawDataIdx(x,a) {data.resize(size(),nan(""));}
    RawData(RawDataIdx&& x): RawDataIdx(x) {data.resize(size(),nan(""));}

    void clear() {RawDataIdx::clear(); data.clear();}

    RawData hyperSlice(const std::vector<std::string>& axes, const Key& fixedLabels) {
      std::vector<SizeStride> sizeAndStrides;

//      // need to push strides in reverse order, as apply pops from the back
//      for (auto axis=axes.rbegin(); axis!=axes.rend(); ++axis) 
//        sizeAndStrides.push_back(sizeStride(*axis));
      for (auto& axis: axes)
        sizeAndStrides.push_back(sizeStride(axis));

      RawData r(*this,axes);
      // assign sliced data to new slice
      size_t i=0;
      apply(idx(fixedLabels), sizeAndStrides, [&](double x){r.data[i++]=x;});
      return r;
    }

    /// apply functional \a f to all elements in slice defined by
    /// offset and \a sizeAndStrides

    // this peculiar way of doing things is to emulate a nested for
    // loop with an arbitrary number of nestings

    template <typename F>
    void apply(size_t offset, std::vector<RawDataIdx::SizeStride> sizeAndStrides, F f){
      assert(!sizeAndStrides.empty());
      size_t size=sizeAndStrides.back().size;
      size_t stride=sizeAndStrides.back().stride; 
      sizeAndStrides.pop_back();
      for (size_t i=offset; i<offset+size*stride; i+=stride)
        if (sizeAndStrides.empty())
          f(data[i]);
        else
          apply(i, sizeAndStrides, f);
    }
  };

}

#if defined(CLASSDESC) || defined(ECOLAB_LIB)
#include "rawData.cd"
#endif
#endif
