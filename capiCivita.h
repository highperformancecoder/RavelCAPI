/*
  Ravel C API. © Ravelation Pty Ltd 2023
*/

#ifndef CAPICIVITA_H
#define CAPICIVITA_H
#include <stdlib.h>
/// First argument of these method pointers is effectively the this pointer
struct CAPITensor
{
  /// return a json representation of a hypercube
  const char* (*hypercube)(const struct CAPITensor*);
  /// return number of non-zero elements in the tensor
  size_t (*size)(const struct CAPITensor*);
  /// return number of elements in the index vector: 0 implies dense data 
  size_t (*indexSize)(const struct CAPITensor*);
  /// return \a index[i]
  size_t (*index)(const struct CAPITensor*, size_t i);
  /// return element \a i. If sparse, this is equivalent to T[index[i]].
  double (*at)(const struct CAPITensor*,size_t i);
};
typedef struct CAPITensor CAPITensor;
#endif
