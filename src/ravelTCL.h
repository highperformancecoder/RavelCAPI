#ifndef RAVELTCL_H
#define RAVELTCL_H
#include <TCL_obj_base.h>
#include <TCL_obj_stl.h>
#include "ravel.h"

namespace classdesc
{
  template <>
  struct is_sequence<ravel::Ravel::Handles>: public true_type {};
}

namespace ecolab
{

  template <>
  void TCL_obj(TCL_obj_t& targ, const string& desc, ravel::SortedVector& arg)
  {
    TCL_obj_const_vector(targ,desc,arg);
    // select the mutating version
    ravel::SortedVector::Order (ravel::SortedVector::*m)
      (ravel::SortedVector::Order)=&ravel::SortedVector::order;
    TCL_obj(targ,desc+".order",arg,m);
  }
  template <>
  void TCL_obj(TCL_obj_t& targ, const string& desc, const ravel::SortedVector& arg)
  {TCL_obj_const_vector(targ,desc,arg);}

}

using ecolab::TCL_obj;

#endif
