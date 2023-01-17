/*
  Ravel C API. © Ravelation Pty Ltd 2023
*/

#ifndef CAPICIVITA_H
#define CAPICIVITA_H
#include <stdlib.h>
struct CAPITensor
{
  const char* (*hypercube)(struct  CAPITensor*);
  size_t (*size)(struct  CAPITensor*);
  size_t (*indexSize)(struct  CAPITensor*, size_t i);
  size_t* (*index)(struct  CAPITensor*, size_t i);
  double (*at)(struct  CAPITensor*,size_t i);
};
typedef struct CAPITensor CAPITensor;
#endif
