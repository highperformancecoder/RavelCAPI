#ifndef DUMMYPOLY_H
#define DUMMYPOLY_H

namespace classdesc
{
  // dummy versions of these when classdesc is not around
  template <class T> struct PolyBase
  {
    virtual T type() const=0;
  };
  template <class T> struct PolyXML {};
  template <class T, class B> struct Poly: public B {};

}
#endif
