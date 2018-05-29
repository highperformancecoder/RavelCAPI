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

        RawDataIdx hs=rd.slice({"foo","fbar"},{{"bar","b2"}});
        CHECK_EQUAL(2,hs.rank());
        CHECK_EQUAL(6,hs.size());
        
        CHECK(hs.size()<=rd.size());
        CHECK_EQUAL(2,(rd[hs.idx({{"foo","f2"},{"fbar","fb1"}})]));
        RawData rd2(rd,hs);
        CHECK(std::isnan(rd2[0]));
    }

  TEST_FIXTURE(RawDataIdxFixture,reorder)
  {
    RawData rd(*this);
    CHECK_EQUAL(3,rank());
    for (size_t i=0; i<dim(0); ++i)
      for (size_t j=0; j<dim(1); ++j)
        for (size_t k=0; k<dim(2); ++k)
          rd[i*stride(0)+j*stride(1)+k*stride(2)]=i+10*j+100*k;

    vector<Handle> handles(3);
    handles[0].description="foo";
    handles[0].sliceLabels={"f1","f2","f3"};
    handles[1].description="bar";
    handles[1].sliceLabels={"b1","b2"};
    handles[2].description="fbar";
    handles[2].sliceLabels={"fb0","fb1"};
    vector<SortedVector> order;
    for (auto& h: handles)
      {
        h.sliceLabels.order(HandleSort::reverse);
        order.push_back(h.sliceLabels);
      }
    RawData r2=rd.reorder(order);

    for (size_t i=0; i<dim(0); ++i)
      for (size_t j=0; j<dim(1); ++j)
        for (size_t k=0; k<dim(2); ++k)
          CHECK_EQUAL
            (rd[(dim(0)-i-1)*rd.stride(0)+
                (dim(1)-j-1)*rd.stride(1)+(dim(2)-k-1)*rd.stride(2)],
             r2[i*r2.stride(0)+j*r2.stride(1)+k*r2.stride(2)]);

  }
  
}
