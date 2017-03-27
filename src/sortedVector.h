#ifndef SORTEDVECTOR_H
#define SORTEDVECTOR_H

#include <vector>
#include <string>
#include <assert.h>
#ifdef ECOLAB_LIB
#include <TCL_obj_stl.h>
#endif
#include "cda.h"

namespace ravel
{
  /// enum describing the sorting properties of handle
  struct HandleSort
  {
	  enum Order {none, forward, reverse, numForward, numReverse, custom };
  };

  /// a vector of string that can be placed in a sorted arrangement
  class SortedVector: public HandleSort
  {
  public:
    typedef std::string value_type;
   
    SortedVector(size_t sz=0, const std::string& s=""): 
      labels(sz,s) {order(none);}
    SortedVector(const std::vector<std::string>& x): labels(x) {order(none);}
    SortedVector(std::initializer_list<std::string> x): 
      labels(x.begin(), x.end()) {order(none);}

    void resize(size_t sz);

    void clear() {resize(0);}

    void push_back(const std::string& x) {
      labels.push_back(x);
      order(order());
    }
    void pop_back() {
      labels.pop_back();
    }
    
    const std::string& operator[](size_t i) const {
      assert(isPermValid());
      return labels[indices[i]];
    }
    size_t idx(size_t i) const {return indices[i];}
    
    class iterator: public std::iterator<std::bidirectional_iterator_tag, std::string>
    {
      const std::vector<std::string>& v;
      std::vector<size_t>::const_iterator i;
    public:
      iterator(const std::vector<std::string>& v, 
               std::vector<size_t>::const_iterator i): v(v), i(i) {}
      const std::string& operator*() const {return v[*i];}
      const std::string* operator->() const {return &v[*i];}
      const iterator& operator++() {++i; return *this;}
      iterator operator++(int) {iterator j(*this); ++i; return j;}
      const iterator& operator--() {--i; return *this;}
      iterator operator--(int) {iterator j(*this); --i; return j;}
      bool operator==(const iterator& x) const {return &v==&x.v && i==x.i;}
      bool operator!=(const iterator& x) const {return !operator==(x);}
    };

    typedef iterator const_iterator;
    iterator begin() const {return iterator(labels,indices.begin());}
    iterator end() const {return iterator(labels,indices.end());}
    typedef size_t size_type;
    size_t size() const {return labels.size();};
    bool empty() const {return size()==0;}
    std::string& back() {return labels.back();}
    
    /// elementwise equality.
    bool operator==(const SortedVector& x) {
      if (x.size()!=size()) return false;
      for (size_t i=0; i<size(); ++i)
        if (x[i]!=(*this)[i])
          return false;
      return true;
    }
    bool operator!=(const SortedVector& x) {return !operator==(x);}

    /// current ordering applied to the vector
    Order order() const {return m_order;}
    Order order(Order o);
    /// returns true if reverse ordering is in effect
    bool orderReversed() const {
      return order()==reverse || order()==numReverse;
    }

    /// apply a custom permuatation
    void customPermutation(std::vector<size_t>&);

    /// check permutation is valid
    bool isPermValid() const;

  private:
    CLASSDESC_ACCESS(SortedVector);
    Order m_order;
    std::vector<std::string> labels;
    std::vector<size_t> indices;
  };

#ifdef ECOLAB_LIB
  inline std::ostream& operator<<(std::ostream& i, const SortedVector& v)
  {return ecolab::ContainerOut(i,v);}

  inline std::istream& operator>>(std::istream& i, SortedVector& v)
  {return ecolab::ContainerIn(i,v);}
#endif

}


#if defined(CLASSDESC) || defined(ECOLAB_LIB)
#ifdef _CLASSDESC
// this is supplied in ravelTCL.h
#pragma omit TCL_obj ravel::SortedVector
#pragma omit random_init ravel::SortedVector
#endif


#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#endif

#include "sortedVector.cd"

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

namespace classdesc_access
{
  template <>
  struct access_random_init<ravel::SortedVector>
  {
    template <class T>
    void operator()(classdesc::random_init_t& r, const classdesc::string& d, T& a)
    {
      random_init(r,d,a.labels);
      ravel::SortedVector::Order o;
      random_init(r,d,o);
      a.order(o);
      assert(a.isPermValid());
    }
  };
}

#endif

#endif
