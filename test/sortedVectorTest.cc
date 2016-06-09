#include <sortedVector.h>
#include <UnitTest++/UnitTest++.h>
#include <ecolab_epilogue.h>
using namespace std;
using namespace ravel;

SUITE(SortedVector)
{
  /*
    Test rendering by outputting a PNG file.  This test is quite
    fragile, as cosmetic "look and feel" changes can break it.
   */
  TEST_FIXTURE(SortedVector, sortit)
    {
      SortedVector sv={"100a", "50b", "1000c", "100d"};
      SortedVector uv=sv; // unsorted copy
      CHECK(sv==uv);

      sv.order(SortedVector::forward);
      for (size_t i=0; i<sv.size()-1; ++i)
        CHECK(sv[i]<=sv[i+1]);

      sv.order(SortedVector::reverse);
      for (size_t i=0; i<sv.size()-1; ++i)
        CHECK(sv[i]>=sv[i+1]);

      sv.order(SortedVector::numForward);
      for (size_t i=0; i<sv.size()-1; ++i)
        CHECK(atof(sv[i].c_str())<=atof(sv[i+1].c_str()));

      sv.order(SortedVector::numReverse);
      for (size_t i=0; i<sv.size()-1; ++i)
        CHECK(atof(sv[i].c_str())>=atof(sv[i+1].c_str()));

      sv.order(SortedVector::none);
      CHECK(sv==uv);
      
      // check iterator operation
      for (SortedVector::iterator i=sv.begin(), j=uv.begin(); i!=sv.end(); ++i,++j)
        {
          CHECK(*i==*j);
          CHECK(i->size()==j->size());
        }

      // check increment/decrement ops
      SortedVector::iterator i=sv.begin();
      CHECK_EQUAL(sv[0],*i);
      CHECK_EQUAL(sv[0],*(i++));
      CHECK_EQUAL(sv[2],*(++i));
      CHECK_EQUAL(sv[2],*i);
      CHECK_EQUAL(sv[2],*(i--));
      CHECK_EQUAL(sv[0],*(--i));
      CHECK_EQUAL(sv[0],*i);

      sv.resize(2);
      CHECK_EQUAL(2,sv.size());
      CHECK_EQUAL(uv[0],sv[0]);
      CHECK_EQUAL(uv[1],sv[1]);

    }
}
