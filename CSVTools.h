/*
  Ravel C API. © Ravelation Pty Ltd 2025
*/

#ifndef RAVEL_CSVTOOLS_H
#define RAVEL_CSVTOOLS_H

#include <algorithm>
#include <string>

namespace ravel
{
  struct CSVSpec
  {
    char separator=',';       ///< field separator character
    char quote='"';           ///< quote character
    char escape='\0';          ///< escape character, might be backslash, technically shouldn't be used for CSV
  };
  
  /// get complete line from input, allowing for quoted linefeed
  bool getWholeLine(std::istream& input, std::string& line, const CSVSpec& spec);

  /// replace doubled quotes with escaped quotes
  void escapeDoubledQuotes(std::string&,const CSVSpec&);

  // pinched from boost::escape_list_separator, and modified to not throw
  template <class Char,
            class Traits = typename std::basic_string<Char>::traits_type >
  class EscapedListSeparator {

  private:
    typedef std::basic_string<Char,Traits> string_type;
    struct char_eq {
      Char e_;
      char_eq(Char e):e_(e) { }
      bool operator()(Char c) {
        return Traits::eq(e_,c);
      }
    };
    string_type  escape_;
    string_type  c_;
    string_type  quote_;
    bool last_;

    bool is_escape(Char e) {
      const char_eq f(e);
      return std::find_if(escape_.begin(),escape_.end(),f)!=escape_.end();
    }
    bool is_c(Char e) {
      const char_eq f(e);
      return std::find_if(c_.begin(),c_.end(),f)!=c_.end();
    }
    bool is_quote(Char e) {
      const char_eq f(e);
      return std::find_if(quote_.begin(),quote_.end(),f)!=quote_.end();
    }
    template <typename iterator, typename Token>
    void do_escape(iterator& next,iterator end,Token& tok) {
      if (++next >= end)
        // don't throw, but pass on verbatim
        tok+=escape_.front();
      if (Traits::eq(*next,'n')) {
        tok+='\n';
        return;
      }
      if (is_quote(*next)) {
        tok+=*next;
        return;
      }
      if (is_c(*next)) {
        tok+=*next;
        return;
      }
      if (is_escape(*next)) {
        tok+=*next;
        return;
      }
      // don't throw, but pass on verbatim
      tok+=escape_.front()+*next;
    }

  public:

    explicit EscapedListSeparator(Char  e = '\\')
      : escape_(1,e), c_(1,','), quote_(1,'\"'), last_(false) { }
    EscapedListSeparator(Char  e, Char c,Char  q = '\"')
      : escape_(1,e), c_(1,c), quote_(1,q), last_(false) { }

    void reset() {last_=false;}

    template <typename InputIterator, typename Token>
    bool operator()(InputIterator& next,InputIterator end,Token& tok) {
      bool bInQuote = false;
      tok = Token();

      if (next >= end) {
        next=end; // reset next in case it has adavanced beyond
        if (last_) {
          last_ = false;
          return true;
        }
        return false;
      }
      last_ = false;
      while (next < end) {
        if (is_escape(*next)) {
          do_escape(next,end,tok);
        }
        else if (is_c(*next)) {
          if (!bInQuote) {
            // If we are not in quote, then we are done
            ++next;
            // The last character was a c, that means there is
            // 1 more blank field
            last_ = true;
            return true;
          }
          tok+=*next;
        }
        else if (is_quote(*next)) {
          bInQuote=!bInQuote;
        }
        else {
          tok += *next;
        }
        ++next;
      }
      return true;
    }
  };

  using Parser=EscapedListSeparator<char>;

  struct SpaceSeparatorParser
  {
    char escape, quote;
    SpaceSeparatorParser(char escape='\\', char sep=' ', char quote='"'):
      escape(escape), quote(quote) {}
    template <class I>
    bool operator()(I& next, I end, std::string& tok)
    {
      tok.clear();
      bool quoted=false;
      while (next!=end)
        {
          if (*next==escape)
            tok+=*(++next);
          else if (*next==quote)
            quoted=!quoted;
          else if (!quoted && isspace(*next))
            {
              while (isspace(*next)) ++next;
              return true;
            }
          else
            tok+=*next;
          ++next;
        }
      return !tok.empty();
    }
    void reset() {}
  };
}

#endif
