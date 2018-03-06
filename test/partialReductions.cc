#include <dataCube.h>
#include <UnitTest++/UnitTest++.h>
#include <ecolab_epilogue.h>
using namespace std;
using namespace ravel;

struct DC: public DataCube
{
  void setDataElement(size_t col, size_t row, double v) override {}
};

SUITE(PartialReductions)
{
  TEST_FIXTURE(DC, binning)
    {
      ifstream f("input.csv");
      CSVFTokeniser tok(f,',');
      auto spec=initDataSpec(tok);
      loadFile("input.csv",',',spec);

      Ravel ravel;
      initRavel(ravel);
      ravel.handleIds=vector<size_t>{0,1};
      auto initRD=hyperSlice(ravel);
      
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
  
  TEST_FIXTURE(DC, scanning)
    {
      ifstream f("input.csv");
      CSVFTokeniser tok(f,',');
      auto spec=initDataSpec(tok);
      loadFile("input.csv",',',spec);

      Ravel ravel;
      initRavel(ravel);
      ravel.handleIds=vector<size_t>{0,1};
      auto initRD=hyperSlice(ravel);
      
      // bin on handle
      const unsigned window=2;
      ravel.handles[0].addPartialReduction(new Scan(Scan::add,window));
      auto rd=hyperSlice(ravel);

      CHECK_EQUAL(ravel.rank(),rd.rank());
      for (size_t axis=0; axis<rd.rank(); ++axis)
        CHECK_EQUAL(initRD.dim(axis),rd.dim(axis));
      CHECK(rd.size()==initRD.size());
      CHECK(rd.stride(0)==initRD.stride(0));
      
      for (size_t i=0; i<rd.size(); i+=rd.dim(0)*rd.stride(0))
        for (size_t j=0; j<rd.dim(0); ++j)
          for (size_t k=0; k<rd.stride(0); ++k)
            {
              double sum=0;
              for (size_t l=j<window-1? 0: j-window+1; l<=j; ++l)
                {
                  double val=initRD[i+l*initRD.stride(0)+k];
                  if (isfinite(val))
                    sum+=val;
                }
              CHECK_EQUAL(sum, rd[i+j*rd.stride(0)+k]);
            }

      ravel.handles[0].clearPartialReductions();
      ravel.handles[0].addPartialReduction(new Scan(Scan::multiply,window));
      rd=hyperSlice(ravel);

      CHECK_EQUAL(ravel.rank(),rd.rank());
      for (size_t axis=0; axis<rd.rank(); ++axis)
        CHECK_EQUAL(initRD.dim(axis),rd.dim(axis));
      CHECK(rd.size()==initRD.size());
      CHECK(rd.stride(0)==initRD.stride(0));
      
      for (size_t i=0; i<rd.size(); i+=rd.dim(0)*rd.stride(0))
        for (size_t j=0; j<rd.dim(0); ++j)
          for (size_t k=0; k<rd.stride(0); ++k)
            {
              double prod=1;
              for (size_t l=j<window-1? 0: j-window+1; l<=j; ++l)
                {
                  double val=initRD[i+l*initRD.stride(0)+k];
                  if (isfinite(val))
                    prod*=val;
                }
              CHECK_EQUAL(prod, rd[i+j*rd.stride(0)+k]);
            }

      // remove a data item, and check result is sensible
      rawData[0]=nan("");
      rd=hyperSlice(ravel);
      CHECK(isnan(rd[0]));
      CHECK_EQUAL(initRD[initRD.stride(0)],rd[rd.stride(0)]);
      dynamic_cast<Scan&>(*ravel.handles[0].partialReductions()[0]).op=Scan::add;
      rd=hyperSlice(ravel);
      CHECK(isnan(rd[0]));
      CHECK_EQUAL(initRD[initRD.stride(0)],rd[rd.stride(0)]);
     
    }


  TEST_FIXTURE(DC, diffing)
    {
      ifstream f("input.csv");
      CSVFTokeniser tok(f,',');
      auto spec=initDataSpec(tok);
      loadFile("input.csv",',',spec);

      Ravel ravel;
      initRavel(ravel);
      ravel.handleIds=vector<size_t>{0,1};
      auto initRD=hyperSlice(ravel);
      Ravel originalRavel=ravel;
      
      // bin on handle
      const unsigned offset=1;
      ravel.handles[0].addPartialReduction(new Change(Change::subtract,offset));
      auto rd=hyperSlice(ravel);

      CHECK_EQUAL(ravel.rank(),rd.rank());
      for (size_t axis=0; axis<rd.rank(); ++axis)
        if (axis==rd.axis(ravel.handles[0].description))
          CHECK_EQUAL(initRD.dim(axis)-offset,rd.dim(axis));
        else
          CHECK_EQUAL(initRD.dim(axis),rd.dim(axis));

      CHECK(rd.stride(0)==initRD.stride(0));

      CHECK_EQUAL(2, rd.rank());
      for (size_t i=0; i<rd.dim(0); ++i)
        for (size_t j=0; j<rd.dim(1); ++j)
          {
            CHECK(i+offset<initRD.dim(0));
            size_t irdOffs=i*initRD.stride(0) +j*initRD.stride(1);
            size_t rdOffs=i*rd.stride(0) +j*rd.stride(1);
            
            CHECK_EQUAL(initRD[irdOffs+offset*initRD.stride(0)]-initRD[irdOffs],
                        rd[rdOffs]);
          }
      
      ravel.handles[0].clearPartialReductions();
      ravel.handles[0].addPartialReduction(new Change(Change::divide,offset));
      rd=hyperSlice(ravel);

      for (size_t i=0; i<rd.dim(0); ++i)
        for (size_t j=0; j<rd.dim(1); ++j)
          {
            CHECK(i+offset<initRD.dim(0));
            size_t irdOffs=i*initRD.stride(0) +j*initRD.stride(1);
            size_t rdOffs=i*rd.stride(0) +j*rd.stride(1);
            
            CHECK_EQUAL(initRD[irdOffs+offset*initRD.stride(0)]/initRD[irdOffs],
                        rd[rdOffs]);
          }

      ravel.handles[0].clearPartialReductions();
      ravel.handles[0].addPartialReduction(new Change(Change::percent,offset));
      rd=hyperSlice(ravel);

      for (size_t i=0; i<rd.dim(0); ++i)
        for (size_t j=0; j<rd.dim(1); ++j)
          {
            CHECK(i+offset<initRD.dim(0));
            size_t irdOffs=i*initRD.stride(0) +j*initRD.stride(1);
            size_t rdOffs=i*rd.stride(0) +j*rd.stride(1);
            
            CHECK_EQUAL(100*(initRD[irdOffs+offset*initRD.stride(0)]-initRD[irdOffs])/initRD[irdOffs],
                        rd[rdOffs]);
          }
      
      ravel.handles[0].clearPartialReductions();
      ravel.handles[0].addPartialReduction(new Change(Change::relative,offset));
      rd=hyperSlice(ravel);

      for (size_t i=0; i<rd.dim(0); ++i)
        for (size_t j=0; j<rd.dim(1); ++j)
          {
            CHECK(i+offset<initRD.dim(0));
            size_t irdOffs=i*initRD.stride(0) +j*initRD.stride(1);
            size_t rdOffs=i*rd.stride(0) +j*rd.stride(1);
            
            CHECK_EQUAL((initRD[irdOffs+offset*initRD.stride(0)]-initRD[irdOffs])/initRD[irdOffs],
                        rd[rdOffs]);
          }

      // check that clear reverts back to the original
      ravel.handles[0].clearPartialReductions();
      rd=hyperSlice(ravel);
      CHECK(rd.size()==initRD.size());
      for (size_t i=0; i<rd.size(); ++i)
        CHECK(rd[i]==initRD[i]);

      CHECK_EQUAL(originalRavel.handles.size(), ravel.handles.size());
      for (size_t i=0; i<ravel.handles.size(); ++i)
        CHECK(originalRavel.handles[i].sliceLabels==ravel.handles[i].sliceLabels);
      
    }
}
