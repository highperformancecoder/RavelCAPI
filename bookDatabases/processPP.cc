#include <string>
#include <iostream>
#include <fstream>
#include <map>
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
  ifstream f("full_BIS_SELECTED_PP.csv");
  escaped_list_separator<char> csvParser('\\',',','"');
  typedef tokenizer<escaped_list_separator<char>> Tok;
  
  string buf;
  
  // skip header
  for (int i=0; i<6; ++i) getline(f,buf);
  Tok tok(buf.begin(), buf.end(),csvParser);
  vector<string> col(tok.begin(),tok.end());

  ofstream pp("property.txt");
  format ppFormat(R"("%1%","%2%","%3%","%4%",%5%)""\n");
                   
  map<string, map<string, double>> usdValue, gdpRatio;
  while (getline(f,buf))
    try
      {
        Tok tok(buf.begin(), buf.end(),csvParser);
        vector<string> row(tok.begin(),tok.end());
        if (row.size()<4) continue;
        auto country=trim(row[1]);
        auto value=trim(row[2]);
        auto unit=trim(row[3]);
        for (size_t i=5; i<row.size(); ++i)
          if (i<col.size() && !row[i].empty())
            pp << ppFormat % country % value % unit % col[i] % row[i];
      }
    catch (...) {/*ignore bad data*/}
}
