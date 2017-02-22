#include <ravel.h>
#include <dataCube.h>
#include <UnitTest++/UnitTest++.h>
#include <ecolab_epilogue.h>
using namespace ravel;

#include <fstream>
using namespace std;

struct DC: public DataCube
{
  void setDataElement(size_t col, size_t row, double v) override {}
};

SUITE(Datacube)
{
  TEST_FIXTURE(DC,hypeslice)
    {
      ifstream f("input.csv");
      CSVFTokeniser tok(f,',');
      auto spec=initDataSpec(tok);
      loadFile("input.csv",',',spec);

      Ravel ravel;
      initRavel(ravel);
      ravel.handleIds={0};
      auto rd=hyperSlice(ravel);
      
    }
}
