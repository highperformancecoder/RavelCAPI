#include "ravel.h"
#include "ravelCAPITypes.h"
#include <UnitTest++/UnitTest++.h>
#include <ecolab_epilogue.h>
using namespace std;
using namespace ravel;
using namespace classdesc;

struct RavelFixture
{
  Ravel r1, r2;
  RavelFixture() {
    r1.addHandle("foo",vector<string>{"a","c","b","2"});
    r1.addHandle("bar",vector<string>{"a","c","b","2"});
    r1.addHandle("foobar",vector<string>{"a","c","b","2"});
    r1.handleIds={1,2};
    r1.redistributeHandles();
    r2=r1;
    r2.handleIds={0};
    r2.handles[0].setSlicer("c");
    r2.handles[0].sliceLabels.setCalipers("c","b");
    r2.handles[1].toggleCollapsed();
    r2.handles[1].reductionOp=Op::prod;
    r2.handles[2].sliceLabels.customPermutation(vector<size_t>{3,1,2});
 }
};

SUITE(RavelState)
{
  TEST_FIXTURE(RavelFixture, saveRestoreHandle1)
    {
      for (size_t i=0; i<r1.handles.size(); ++i)
        {
          CHECK(!deepEq(r1.handles[i], r2.handles[i]));
          auto state=r1.handles[i].getHandleState();
          r1.handles[i].setHandleState(state);
          r2.handles[i].setHandleState(state);
          CHECK(deepEq(r1.handles[i], r2.handles[i]));
          if (!deepEq(r1.handles[i], r2.handles[i]))
            cout << "handle: "<<i << endl;
        }
    }
  
  TEST_FIXTURE(RavelFixture, saveRestoreHandle2)
    {
      for (size_t i=0; i<r1.handles.size(); ++i)
        {
          CHECK(!deepEq(r1.handles[i], r2.handles[i]));
          auto state=r2.handles[i].getHandleState();
          r1.handles[i].setHandleState(state);
          r2.handles[i].setHandleState(state);
          CHECK(deepEq(r1.handles[i], r2.handles[i]));
          if (!deepEq(r1.handles[i], r2.handles[i]))
            cout << "handle: "<<i << endl;
        }
    }
  
  TEST_FIXTURE(RavelFixture, saveRestoreCAPIHandle1)
    {
      for (size_t i=0; i<r1.handles.size(); ++i)
        {
          CHECK(!deepEq(r1.handles[i], r2.handles[i]));
          RavelHandleStateX state=r1.handles[i].getHandleState();
          r1.handles[i].setHandleState(state);
          r2.handles[i].setHandleState(state);
          CHECK(deepEq(r1.handles[i], r2.handles[i]));
          if (!deepEq(r1.handles[i], r2.handles[i]))
            cout << "handle: "<<i << endl;
        }
    }
  
  TEST_FIXTURE(RavelFixture, saveRestoreCAPIHandle2)
    {
      for (size_t i=0; i<r1.handles.size(); ++i)
        {
          CHECK(!deepEq(r1.handles[i], r2.handles[i]));
          RavelHandleStateX state=r2.handles[i].getHandleState();
          r1.handles[i].setHandleState(state);
          r2.handles[i].setHandleState(state);
          CHECK(deepEq(r1.handles[i], r2.handles[i]));
          if (!deepEq(r1.handles[i], r2.handles[i]))
            cout << "handle: "<<i << endl;
        }
    }
  TEST_FIXTURE(RavelFixture, saveRestoreState1)
    {
      CHECK(!deepEq(r1, r2));
      auto state=r1.getState();
      r1.setState(state);
      r2.setState(state);
      CHECK(deepEq(r1, r2));
    }
  
   TEST_FIXTURE(RavelFixture, saveRestoreState2)
    {
      CHECK(!deepEq(r1, r2));
      auto state=r2.getState();
      r1.setState(state);
      r2.setState(state);
      CHECK(deepEq(r1,r2));
    }
   
   TEST_FIXTURE(RavelFixture, saveRestoreCAPIState1)
    {
      CHECK(!deepEq(r1,r2));
      RavelStateX cstate(r1.getState());
      r1.setState(cstate);
      r2.setState(cstate);
      CHECK(deepEq(r1,r2));
    }
   
   TEST_FIXTURE(RavelFixture, saveRestoreCAPIState2)
    {
      CHECK(!deepEq(r1,r2));
      RavelStateX cstate(r2.getState());
      r1.setState(cstate);
      r2.setState(cstate);
      CHECK(deepEq(r1,r2));
    }

}
