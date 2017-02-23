#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <cmath>
#include <boost/tokenizer.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace boost;

// remove initial code
string trim(const string& x)
{
  int pos=x.find(':');
  if (pos!=string::npos)
    return x.substr(pos+1);
  else
    return x;
}

int main()
{
  ifstream f("PopulationData.csv");
  escaped_list_separator<char> csvParser('\\','\t','"');
  typedef tokenizer<escaped_list_separator<char>> Tok;
  
  string buf;
  
  // skip header
  getline(f,buf);
  Tok tok(buf.begin(), buf.end(),csvParser);
  vector<string> col(tok.begin(),tok.end());

  ofstream pp("population.txt");
  format ppFormat(R"("%1%","%2%-Q%3%",%4%)""\n");
                   
  map<string, map<int, double>> pop;
  while (getline(f,buf))
    try
      {
        Tok tok(buf.begin(), buf.end(),csvParser);
        vector<string> row(tok.begin(),tok.end());
        if (row.size()<3) continue;
        auto country=trim(row[2]);
        for (size_t i=5; i<row.size(); ++i)
          if (i<col.size() && !row[i].empty())
            pop[country][stoi(col[i])]=stod(row[i]);
      }
    catch (...) {/*ignore bad data*/}

  // write out data, extrapolating harmonically for the quarters
  for (auto& i: pop)
    for (auto& j: i.second)
      {
        if (j.first!=i.second.begin()->first)
          {
            int pop=i.second[j.first-1];
            // interpolate
            double r=pow(j.second/pop,0.25);
            pop*=r;
            for (int quarter=1; quarter<4; ++quarter, pop*=r)
              pp<<ppFormat % i.first % j.first % quarter % pop;
          }
        pp<<ppFormat % i.first % j.first % 4 % j.second;
      }
}
