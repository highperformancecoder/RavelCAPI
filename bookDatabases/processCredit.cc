#include <string>
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
  ifstream f("full_BIS_TOTAL_CREDIT.csv");
  escaped_list_separator<char> csvParser('\\',',','"');
  typedef tokenizer<escaped_list_separator<char>> Tok;
  
  string buf;
  
  // skip header
  for (int i=0; i<6; ++i) getline(f,buf);
  Tok tok(buf.begin(), buf.end(),csvParser);
  vector<string> col(tok.begin(),tok.end());

  ofstream privCredit("priv_credit.txt"), govCredit("gov_credit.txt"), gdp("gdp.txt");
  format privFormat(R"("%1%","%2%","%3%","%4%")""\n");
  format govFormat(R"("%1%","%2%","%3%")""\n");
  format gdpFormat(R"("%1%","%2%",%3%)""\n");
                   
  map<string, map<string, double>> usdValue, gdpRatio;
  while (getline(f,buf))
    try
      {
        Tok tok(buf.begin(), buf.end(),csvParser);
        vector<string> row(tok.begin(),tok.end());
        if (row.size()<7) continue;
        auto country=trim(row[1]);
        auto sector=trim(row[2]);
        auto lendSector=trim(row[3]);
        auto valuation=trim(row[4]);
        auto unit_type=trim(row[5]);
        auto adjustment=trim(row[6]);
        if (valuation!="Market value" || adjustment!="Adjusted for breaks" || lendSector!="All sectors")
          continue;
        // grab the values needed for computing GDP
        if (/*sector=="Private non-financial sector" && */lendSector=="All sectors")
          for (size_t i=8; i<row.size(); ++i)
            if (i<col.size() && !row[i].empty())
              {
                if (unit_type=="Percentage of GDP")
                  gdpRatio[country][col[i]]+=stod(row[i]);
                else if (unit_type=="US Dollar")
                  usdValue[country][col[i]]+=stod(row[i]);
              }
        if  (unit_type=="US Dollar")
          for (size_t i=8; i<row.size(); ++i)
            if (i<col.size() && !row[i].empty())
              {
                if (sector=="General government")
                  govCredit<<govFormat % country % col[i] % row[i];
                else if (sector=="Households & NPISHs" ||
                         sector=="Non-financial corporations")
                  privCredit<<privFormat % country % sector % col[i] %
                    row[i];
              }
      }
    catch (...) {/*ignore bad data*/}

  // write out GDP data
  for (auto& i: usdValue)
    if (gdpRatio.count(i.first))
      for (auto& j: i.second)
        if (gdpRatio[i.first].count(j.first))
          gdp<<gdpFormat % i.first % j.first % (100*j.second/gdpRatio[i.first][j.first]);
          
}
