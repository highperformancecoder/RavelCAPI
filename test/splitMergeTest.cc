#include <splitMerge.h>
#include <vector>
#include <sstream>
#include <assert.h>
#include <UnitTest++/UnitTest++.h>
#include <ecolab_epilogue.h>

using namespace ravel;
using namespace std;

namespace
{
  string str(int i)
  {
    ostringstream os;
    os<<i;
    return os.str();
  }
}

struct TestSM: public SplitMerge
{
  vector<vector<string> > d;
  size_t startR, startC;

  string get(size_t row, size_t col) const override
  {
    assert(row<d.size() && col<d[row].size());
    return d[row][col];
  }

  void set(size_t row, size_t col, const std::string& val) override
  {
    assert(row<d.size() && col<d[row].size());
    d[row][col]=val;
  }

  // for testing, just pick a value
  size_t startRow() const override {return startR;}
 
  size_t startCol() const override {return startC;}
  size_t numRows() const override {return d.size();}
  size_t numCols() const {return d.empty()? 0: d[0].size();}

  void insertRows(size_t r, size_t n) override 
  {
    if (r<d.size())
      {
        d.insert(d.begin()+r,n,vector<string>(numCols()));
        if (r<startR) startR+=n;
      }
  }

  virtual void insertCols(size_t c, size_t n) override 
  {
    for (auto i=d.begin(); i!=d.end(); ++i)
      if (c<i->size())
        {
          i->insert(i->begin()+c,n,"");
        }
    if (c<startC) startC+=n;
  }

  virtual void deleteRows(size_t r, size_t n) override 
  {
    r=min(d.size(), r);
    n=min(d.size()-r, n);
    d.erase(d.begin()+r,d.begin()+r+n);
    if (r<startR)
      startR -= min(startR-r, n);
  }
  
  virtual void deleteCols(size_t c, size_t n) override 
  {
    c=min(numCols(), c);
    n=min(numCols()-c, n);
    for (auto i=d.begin(); i!=d.end(); ++i)
      i->erase(i->begin()+c,i->begin()+c+n);
    if (c<startC)
      startC -= min(startC-c, n);
  }
};

SUITE(SplitMerge)
{
  TEST_FIXTURE(TestSM, split)
    {
      const vector<vector<string>> sample {{"","2000q1","2000q2","2000q3","2001q1","2001q2","2001q3"},
            {"2000q1","","","","","",""},
              {"2000q2","","","","","",""},
                {"2000q3","","","","","",""},
                  {"2001q1","","","","","",""},
                    {"2001q2","","","","","",""},
                      {"2001q3","","","","","",""}};
      d=sample;
      startC=1; startR=1;

      splitCol(0);
      CHECK_EQUAL(sample.size(), numRows());
      CHECK_EQUAL(sample[0].size()+1, numCols());
      CHECK_EQUAL(1, startRow());
      CHECK_EQUAL(2, startCol());
      for (size_t i=startRow(); i<numRows(); ++i)
        {
          CHECK_EQUAL(get(i,0),str(2000+(i-startRow())/3));
          CHECK_EQUAL(get(i,1),"q"+str((i-startRow())%3+1));
        }

      mergeCols(0,2); //checks inverse
      CHECK_EQUAL(sample.size(), numRows());
      CHECK_EQUAL(sample[0].size(), numCols());
      CHECK_EQUAL(1, startRow());
      CHECK_EQUAL(1, startCol());
      CHECK(d==sample);

      splitRow(0);
      CHECK_EQUAL(sample.size()+1, numRows());
      CHECK_EQUAL(sample[0].size(), numCols());
      CHECK_EQUAL(2, startRow());
      CHECK_EQUAL(1, startCol());
      for (size_t i=startCol(); i<numCols(); ++i)
        {
          CHECK_EQUAL(get(0,i),str(2000+(i-startCol())/3));
          CHECK_EQUAL(get(1,i),"q"+str((i-startCol())%3+1));
        }

      mergeRows(0,2); //checks inverse
      CHECK_EQUAL(sample.size(), numRows());
      CHECK_EQUAL(sample[0].size(), numCols());
      CHECK_EQUAL(1, startRow());
      CHECK_EQUAL(1, startCol());
      CHECK(d==sample);

      splitCol(1);
      CHECK(d==sample); // shouldn't do anything
      splitRow(1);
      CHECK(d==sample); // shouldn't do anything

    }

  TEST_FIXTURE(TestSM, merge)
    {
      const vector<vector<string>> sample {{"","","2000","2000","2000","2001","2001","2001"},
          {"","","1","2","3","1","2","3"},
            {"2000","1","","","","","",""},
              {"2000","2","","","","","",""},
                {"2000","3","","","","","",""},
                  {"2001","1","","","","","",""},
                    {"2001","2","","","","","",""},
                      {"2001","3","","","","","",""}};
      startC=startR=2;
      d=sample;

      mergeCols(0,2); 
      CHECK_EQUAL(2, startRow());
      CHECK_EQUAL(1, startCol());
      for (size_t i=startRow(); i<numRows(); ++i)
        CHECK_EQUAL(str(2000+(i-startRow())/3)+"."+str((i-startRow())%3+1), get(i,0));

      startC=startR=2;
      d=sample;
      mergeRows(0,2); 
      CHECK_EQUAL(1, startRow());
      CHECK_EQUAL(2, startCol());
      for (size_t i=startCol(); i<numCols(); ++i)
        CHECK_EQUAL(str(2000+(i-startCol())/3)+"."+str((i-startCol())%3+1), get(0,i));
      
    }

}
