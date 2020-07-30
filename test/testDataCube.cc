#include <dataCube.h>
#include <UnitTest++/UnitTest++.h>
#include <ecolab_epilogue.h>
using namespace std;
using namespace ravel;

struct DC: public DataCube
{
  void setDataElement(size_t col, size_t row, double v) override {}
};

SUITE(DataCube)
{
  TEST_FIXTURE(DC, calipers)
    {
      ifstream f("input.csv");
      CSVFTokeniser tok(f,',');
      auto spec=initDataSpec(tok);
      loadFile("input.csv",',',spec);

      Ravel ravel;
      initRavel(ravel);
      ravel.handleIds=vector<size_t>{2,1};
      auto initRD=hyperSlice(ravel);

      CHECK_EQUAL("ax1",ravel.handles[0].description);
      CHECK_EQUAL("ax2",ravel.handles[1].description);
      CHECK_EQUAL("?",ravel.handles[2].description);

      ravel.handles[2].sliceLabels.min(1);
      ravel.handles[2].sliceLabels.max(2);
      ravel.handles[2].displayFilterCaliper(true);

      auto rd=hyperSlice(ravel);
      CHECK_EQUAL(2,rd.dim(0));
      for (size_t i=0; i<rd.dim(0); ++i)
        for (size_t j=0; j<rd.dim(1); ++j)
          CHECK_EQUAL(initRD[(i+1)*initRD.stride(0) + j*initRD.stride(1)],
                        rd[i*rd.stride(0)+j*rd.stride(1)]);
    }

#if 0  
      // bin on handle
      ravel.handles[0].addPartialReduction(new Bin(Bin::add,2));
      auto rd=hyperSlice(ravel);

      CHECK_EQUAL(ravel.rank(),rd.rank());
      for (size_t axis=0; axis<rd.rank(); ++axis)
        if (axis==rd.axis(ravel.handles[0].description))
          CHECK_EQUAL(1,rd.dim(axis));
        else
          CHECK_EQUAL(initRD.dim(axis),rd.dim(axis));
      CHECK(rd.size()==initRD.size()/2);

      for (size_t i=0; i<rd.size(); ++i)
        CHECK(rd[i]==initRD[2*i]+initRD[2*i+1]);

      dynamic_cast<Bin&>(*ravel.handles[0].partialReductions()[0]).op=Bin::multiply;
      rd=hyperSlice(ravel);
      
      CHECK_EQUAL(ravel.rank(),rd.rank());
      for (size_t axis=0; axis<rd.rank(); ++axis)
        if (axis==rd.axis(ravel.handles[0].description))
          CHECK_EQUAL(1,rd.dim(axis));
        else
          CHECK_EQUAL(initRD.dim(axis),rd.dim(axis));
      CHECK(rd.size()==initRD.size()/2);

      for (size_t i=0; i<rd.size(); ++i)
        CHECK(rd[i]==initRD[2*i]*initRD[2*i+1]);

      ravel.handles[0].clearPartialReductions();
      ravel.handles[2].sliceIndex++;
      auto rdSucc=hyperSlice(ravel);

      // now bin on non-output handle
      ravel.handles[2].addPartialReduction(new Bin(Bin::add,2));
      CHECK_EQUAL(0,ravel.handles[2].sliceIndex);
      rd=hyperSlice(ravel);
      CHECK_EQUAL(ravel.rank(),rd.rank());
      for (size_t axis=0; axis<rd.rank(); ++axis)
        CHECK_EQUAL(initRD.dim(axis),rd.dim(axis));
      CHECK(rd.size()==initRD.size());
      for (size_t i=0; i<rd.size(); ++i)
        CHECK(rd[i]==initRD[i]+rdSucc[i]);

      // remove a data item, and check result is sensible
      rawData[0]=nan("");
      rd=hyperSlice(ravel);
      CHECK_EQUAL(rdSucc[0],rd[0]);
      dynamic_cast<Bin&>(*ravel.handles[2].partialReductions()[0]).op=Bin::multiply;
      rd=hyperSlice(ravel);
      CHECK_EQUAL(rdSucc[0],rd[0]);

   }
#endif


  TEST_FIXTURE(DC, orderedCollapse)
    {
      ifstream f("input.csv");
      CSVFTokeniser tok(f,',');
      auto spec=initDataSpec(tok);
      loadFile("input.csv",',',spec);

      Ravel ravel;
      initRavel(ravel);

      ravel.handleIds={0};
      ravel.handles[1].toggleCollapsed();
      ravel.handles[2].setSlicer("a");
      auto hs=hyperSlice(ravel);
      CHECK_EQUAL(1,hs.rank());
      CHECK_EQUAL(2,hs.dim(0));
      CHECK_EQUAL(3,hs[0]);
      CHECK_EQUAL(7,hs[1]);
      ravel.handles[2].sliceLabels.customPermutation({1,0,2});
      ravel.handles[2].setSlicer("a");
      CHECK_EQUAL("a",ravel.handles[2].sliceLabel());
      hs=hyperSlice(ravel);
      CHECK_EQUAL(3,hs[0]);
      CHECK_EQUAL(7,hs[1]);
      ravel.handles[2].sliceLabels.order(HandleSort::forward);
      ravel.handles[2].setSlicer("a");
      hs=hyperSlice(ravel);
      CHECK_EQUAL(3,hs[0]);
      CHECK_EQUAL(7,hs[1]);
      ravel.handles[2].sliceLabels.order(HandleSort::reverse);
      ravel.handles[2].setSlicer("a");
      hs=hyperSlice(ravel);
      CHECK_EQUAL(3,hs[0]);
      CHECK_EQUAL(7,hs[1]);
    }
}
