#include <UnitTest++/UnitTest++.h>
#include <UnitTest++/TestReporterStdout.h>
#include <boost/regex.hpp>
#include <iostream>
using namespace UnitTest;
using std::string;
using std::cout;
using std::endl;

namespace
{
  string name(const Test& t)
  {return string(t.m_details.suiteName)+"::"+t.m_details.testName;}

  struct RegExPredicate
  {
    boost::regex exp;
    RegExPredicate(): exp(".*") {} // equivalent to true
    RegExPredicate(const string& exp): exp(exp) {}
    bool operator()(const Test* const test) const
    {return boost::regex_match(name(*test), exp);}
  };
}

int main(int argc, const char** argv)
{
  if (argc>1)
    {
      bool listTests=false;
      // handle options
      if (argv[1][0]=='-')
        {
          switch (argv[1][1])
            {
            case 'h': case '?':
              cout << "Usage "<<argv[0]<<" [-h] [-l] [regex]\n";
              cout << " -h: print this help message\n";
              cout << " -l: list all tests matching given regex\n";
              cout << " no option: run all tests matching given pattern\n";
              cout << " no pattern: = all tests, as if .* was provided as the pattern\n";
              return 0;
            case 'l':
              listTests=true;
              break;
            }
          argv++;
          argc--;
        }
      RegExPredicate exp;
      if (argc>1)
        exp=RegExPredicate(argv[1]);
      if (listTests)
        {
          for (Test* t=Test::GetTestList().GetHead(); t; t=t->m_nextTest)
            if (exp(t))
              cout << name(*t)<<endl;
          return 0;
        }
      else
        {
          TestReporterStdout reporter;
          return TestRunner(reporter).RunTestsIf(Test::GetTestList(), NULL, exp, 0);
        }
    }  
      
  return RunAllTests();
}
