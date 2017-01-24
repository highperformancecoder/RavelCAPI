#include <rawData.h>
#include <UnitTest++/UnitTest++.h>
#include <math.h>
#ifdef CLASSDESC
#include <classdesc_epilogue.h>
#endif
#ifdef ECOLAB_LIB
#include <ecolab_epilogue.h>
#endif


using namespace ravel;
using namespace std;

typedef vector<string> VS;
typedef LabelsVector::value_type VT;

struct RawDataIdxFixture: public RawDataIdx
{
  RawDataIdxFixture(): RawDataIdx(LabelsVector
                                  {
                                   VT{"foo",VS{"f1","f2","f3"}},
                                   VT{"bar",VS{"b1","b2"}},
                                   VT{"fbar",VS{"fb0","fb1"}}
                                  }) {}
};

SUITE(RawDataT)
{
  TEST_FIXTURE(RawDataIdxFixture,basic)
    {
      CHECK_EQUAL(3,rank());
      CHECK_EQUAL(12,size());
      CHECK_EQUAL(0,idx({{"foo","f1"},{"bar","b1"},{"fbar","fb0"}}));
      CHECK_EQUAL(6+3+1,idx({{"foo","f2"},{"bar","b2"},{"fbar","fb1"}}));
      CHECK_EQUAL(3,stride("bar"));
    }

  
  TEST_FIXTURE(RawDataIdxFixture,rawDataConstructor)
    {
        RawData rd(*this);
        rd[{{"foo","f1"},{"bar","b1"},{"fbar","fb0"}}]=1;
        rd[{{"foo","f2"},{"bar","b2"},{"fbar","fb1"}}]=2;
        CHECK_EQUAL(1,rd[0]);
        CHECK_EQUAL(2,rd[6+3+1]);

        RawData hs=rd.hyperSlice({"foo","fbar"},{{"bar","b2"}});
        CHECK_EQUAL(2,hs.rank());
        CHECK_EQUAL(6,hs.size());
        
        CHECK_EQUAL(2,(hs[{{"foo","f2"},{"fbar","fb1"}}]));
        CHECK(std::isnan(hs[0]));
    }
}
