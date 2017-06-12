#include "partialReduction.h"
#ifdef CLASSDESC
#include <classdesc_epilogue.h>
#endif
#ifdef ECOLAB_LIB
#include <ecolab_epilogue.h>
#endif

using namespace std;
using namespace classdesc;

namespace ravel
{

  PartialReduction* PartialReduction::create(PartialReductionType t)
  {
    switch (t)
      {
      case PartialReductionType::bin: return new Bin();
      case PartialReductionType::scan: return new Scan();
      case PartialReductionType::change: return new Change();
      }
  }


  void Bin::operator()(double* dest, const double* src, size_t stride, size_t N) const
  {
    for (size_t i=0; i<N; i+=binSize, dest+=stride)
      {
        *dest=op==multiply? 1: 0;
        for (size_t j=0; j<binSize && i+j<N; ++j, src+=stride)
          switch (op)
            {
            case add:
              *dest+=*src;
              break;
            case multiply:
              *dest*=*src;
              break;
            }
      }
  }

  std::vector<size_t> Bin::indices(size_t N) const
  {
    std::vector<size_t> r(N? (N-1)/binSize+1: 0);
    for (size_t i=0; i<r.size(); ++i)
        r[i]=i*binSize+binSize/2;
    if (r.back()>=N) r.back()=N-1;
    return r;
  }


  void Scan::operator()(double* dest, const double* src, size_t stride, size_t N) const
  {
    *dest=*src;
    for (size_t i=1; i<N; ++i, src+=stride, dest+=stride)
      {
        *dest=*src;
        switch (op)
          {
          case add:
            *dest+=*(src-stride);
            if (i>=window)
              *dest-=*(src-window*stride);
            break;
          case multiply:
            *dest*=*(src-stride);
            if (i>=window && *(src-window*stride))
              *dest/=*(src-window*stride);
            break;
          }
      }
  }

  std::vector<size_t> Scan::indices(size_t N) const
  {
    std::vector<size_t> r(N);
    for (size_t i=0; i<N; ++i) r[i]=i;
    return r;
  }

  void Change::operator()(double* dest, const double* src, size_t stride, size_t N) const
  {
    for (size_t i=offset; i<N; ++i, dest+=stride, src+=stride)
      switch (op)
        {
        case subtract:
          *dest=src[offset*stride]-*src;
          break;
        case divide:
          *dest=src[offset*stride]/ *src;
          break;
        case percent:
          *dest=100*(src[offset*stride]-*src)/ *src;
          break;
        case relative:
          *dest=(src[offset*stride]-*src)/ *src;
         break;
        }
  }
  
  std::vector<size_t> Change::indices(size_t N) const
  {
    std::vector<size_t> r(N-offset);
    for (size_t i=0; i<r.size(); ++i) r[i]=i+offset;
    return r;
  }

}