/*
  Ravel C API. © Ravelation Pty Ltd 2025
*/

#include "CSVTools.h"
#include <istream>
using namespace std;

namespace ravel
{
    // handle DOS files with '\r' '\n' line terminators
  void chomp(string& buf)
  {
    if (!buf.empty() && buf.back()=='\r')
      buf.erase(buf.size()-1);
  }
  
  // gets a line, accounting for quoted newlines
  bool getWholeLine(istream& input, string& line, const CSVSpec& spec)
  {
    line.clear();
    bool r=getline(input,line).good();
    chomp(line);
    while (r)
      {
        int quoteCount=0;
        for (auto i: line)
          if (i==spec.quote)
            ++quoteCount;
        if (quoteCount%2==0) break; // data line correctly terminated
        string buf;
        r=getline(input,buf).good(); // read next line and append
        chomp(buf);
        line+=buf;
      }
    escapeDoubledQuotes(line,spec);
    return r || !line.empty();
  }

  void escapeDoubledQuotes(string& line,const CSVSpec& spec)
  {
    // replace doubled quotes with escape quote
    for (size_t i=1; i<line.size(); ++i)
      if (line[i]==spec.quote && line[i-1]==spec.quote &&
          ((i==1 && (i==line.size()-1|| line[i+1]!=spec.quote)) ||                                       // deal with leading ""
           (i>1 &&
            ((line[i-2]!=spec.quote && line[i-2]!=spec.escape &&
              (line[i-2]!=spec.separator || i==line.size()-1|| line[i+1]!=spec.quote))  // deal with ,''
             ||            // deal with "" middle or end
             (line[i-2]==spec.quote && (i==2 || line[i-3]==spec.separator || line[i-3]==spec.escape)))))) // deal with leading """
        line[i-1]=spec.escape;
  }


}
