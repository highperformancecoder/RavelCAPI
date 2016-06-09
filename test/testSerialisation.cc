#include <ravel.h>

// don't bother checking state of pointers, or the boost::any data in rawData
#include <cairo.h>
#include <boost/any.hpp>
#include <pack_base.h>
//namespace classdesc
//{
  void pack(classdesc::pack_t&,const classdesc::string&,cairo_t*) {}
  void unpack(classdesc::pack_t&,const classdesc::string&,cairo_t*) {}
//}
//using classdesc::pack; using classdesc::unpack;
namespace classdesc_access
{
  template <> struct access_pack<boost::any>:
    public classdesc::NullDescriptor<classdesc::pack_t> {};
  template <> struct access_unpack<boost::any>:
    public classdesc::NullDescriptor<classdesc::unpack_t> {};
}
#include <dataCubeTCL.h>
#include <pack_stl.h>
#include <ecolab_epilogue.h>
using namespace classdesc;
using namespace ravel;

#include <sstream>
using namespace std;

template <class T>
void serialiseDeserialise(T& r1, T& r2)
{
  random_init_t r;
  random_init(r,"",r1);
  random_init(r,"",r2);
  ostringstream o;
  xml_pack_t b1(o);
  b1<<r1;
  xml_unpack_t b2;
  //  cout << o.str() << endl;
  istringstream is(o.str());
  b2.parse(is);
  b2>>r2;
}

int main()
{
  Ravel r1, r2;
  serialiseDeserialise(r1,r2);
  // if any of r1's handles have an empty description, then r2's
  // description will be "?". This is by design
  assert(r1.handles.size()==r2.handles.size());
  for (size_t i=0; i<r1.handles.size(); ++i)
    if (r1.handles[i].description.empty())
      r2.handles[i].description.clear();
  
  assert(deepEq(r1,r2));

  DataCubeTCL d1, d2;
  serialiseDeserialise(d1,d2);
  // if any of d1.ravel's handles have an empty description, then d2's
  // description will be "?". This is by design
  assert(d1.ravel.handles.size()==d2.ravel.handles.size());
  for (size_t i=0; i<d1.ravel.handles.size(); ++i)
    if (d1.ravel.handles[i].description.empty())
      d2.ravel.handles[i].description.clear();
  assert(deepEq(d1,d2));
}
