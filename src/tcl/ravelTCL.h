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
    TCL_obj(targ,desc+".setCalipers",arg,&ravel::SortedVector::setCalipers);
  }
  template <>
  void TCL_obj(TCL_obj_t& targ, const string& desc, const ravel::SortedVector& arg)
  {TCL_obj_const_vector(targ,desc,arg);}

  template <>
  void TCL_obj(TCL_obj_t& targ, const string& desc, ravel::Handle& arg)
  {
    TCL_obj_register(targ,desc,arg);
    classdesc_access::access_TCL_obj<ravel::Handle>()(targ,desc,arg);
    // select the mutating version
    bool (ravel::Handle::*dfc)(bool)=&ravel::Handle::displayFilterCaliper;
    TCL_obj(targ,desc+".displayFilterCaliper",arg,dfc);
    TCL_obj_custom_register(desc,arg);
  }
}

using ecolab::TCL_obj;

#endif
