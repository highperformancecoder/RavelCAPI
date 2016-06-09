#ifndef XLEXTRAS_H
#define XLEXTRAS_H


#pragma warning(disable:4244) //conversion of integral types to smaller integral types
#include "../xll/xll.h"
#pragma warning(disable:4512) // generation of assignment operators
#undef min
#undef max
#include <boost/locale.hpp>
#include <boost/any.hpp>

#include <string>

#include <xml_pack_base.h>
#include <xml_unpack_base.h>

namespace xlExtras
{
  using boost::locale::conv::utf_to_utf;

#ifdef EXCEL12
#define XLMREFX XLMREF12
#else
#define XLMREFX XLMREF
#endif

  // sheet specific OPERX (ltypeRef)
  struct SSOPERX : public OPERX
  {
    XLMREFX sref;
    SSOPERX(IDSHEET sheetId, unsigned r, unsigned c,
            unsigned nr = 1, unsigned nc = 1) 
    {
      sref.count = 1;
#ifdef EXCEL12
      sref.reftbl[0].colFirst = (unsigned short)c;
      sref.reftbl[0].colLast = (unsigned short)(c + nc - 1);
      sref.reftbl[0].rwFirst = (unsigned short)r;
      sref.reftbl[0].rwLast = (unsigned short)(r + nr - 1);
#else
      sref.reftbl[0].colFirst = (BYTE)c;
      sref.reftbl[0].colLast = (BYTE)(c + nc - 1);
      sref.reftbl[0].rwFirst = (BYTE)r;
      sref.reftbl[0].rwLast = (BYTE)(r + nr - 1);
#endif
      xltype = xltypeRef;
      val.mref.idSheet = sheetId;
      val.mref.lpmref = &sref;
    }                                           
  };

  struct SaveRestoreSelection
  {
    OPERX sel;
    SaveRestoreSelection() : sel(ExcelX(xlfSelection)) {}
    ~SaveRestoreSelection() { 
      try {ExcelX(xlcSelect, sel);} catch (...) {}
    }
  };


  template <class C> struct TF
  {
    static const C * True;
    static const C * False;
  };

  const char* TF<char>::True = "true";
  const char* TF<char>::False = "false";
  const wchar_t* TF<wchar_t>::True = L"true";
  const wchar_t* TF<wchar_t>::False = L"false";

  template <class C, class T>
  typename std::enable_if<!std::is_base_of<OPERX, T>::value, std::basic_string<C>>::type
    str(T x)
  {
    std::basic_ostringstream<C> os;
    os << x;
    return os.str();
  }

  template <class C>
  std::basic_string<C> str(const LXOPER<XLOPERX>& x)
  {return str<C>(OPERX(x));}

  template <class C, class T>
  typename std::enable_if<std::is_base_of<OPERX, T>::value, std::basic_string<C>>::type
          str(const T& x)
  {
    std::basic_ostringstream<C> os;
    switch (x.type())
      {
      case xltypeNum:
        os << x.val.num;
        return os.str();
      case xltypeStr:
        // x.val.str is length counted, so construct a string first before converting to UTF8
        return utf_to_utf<C>(basic_string<TCHAR>(x.val.str + 1, x.val.str[0]));
      case xltypeBool:
        return x.val.xbool ? TF<C>::True : TF<C>::False;
      case xltypeInt:
        os << x.val.w;
        return os.str();
      default:
        return basic_string<C>();
      }
  }

  // convert an OPERX to an any (double or string)
  inline boost::any toAny(const OPERX& x)
  {
    switch (x.type())
      {
      case xltypeNum:
        return x.val.num;
      case xltypeInt:
        return double(x.val.w);
      default:
        return str<char>(x);
      }
  }



  // return sheetId for current active shet
  inline IDSHEET sheetId()
  {
    return ExcelX(xlSheetId).val.mref.idSheet;
  }

  struct IntOperx: public OPERX
  {
    IntOperx(short v=0)
    {
      xltype=xltypeInt;
      val.w=v;
    }
  };

  // a better OPERX that handles strings
  struct RavelOPERX: public OPERX
  {
    RavelOPERX() {}
    template <class T> RavelOPERX(const T& x): OPERX(x) {}
    template <class T> RavelOPERX(const std::basic_string<T>& x):
      OPERX(utf_to_utf<xchar>(x).c_str()) {}
    RavelOPERX(const char* x): OPERX(utf_to_utf<xchar>(x).c_str()) {}
    RavelOPERX(const wchar_t* x): OPERX(utf_to_utf<xchar>(x).c_str()) {}
  };

  inline OPERX CallXL(int f) {return ExcelX(f);}

  template <class T1>
  OPERX CallXL(int f, const T1& t)
  {
    return ExcelX(f, xlExtras::RavelOPERX(t));
  }

  template <class T1, class T2>
  OPERX CallXL(int f, const T1& t1, const T2& t2)
  {
    return ExcelX(f, xlExtras::RavelOPERX(t1), xlExtras::RavelOPERX(t2));
  }

  template <class T1, class T2, class T3>
  OPERX CallXL(int f, const T1& t1, const T2& t2, const T3& t3)
  {
    return ExcelX(f, xlExtras::RavelOPERX(t1), xlExtras::RavelOPERX(t2), xlExtras::RavelOPERX(t3));
  }

  template <class T1, class T2, class T3, class T4>
  OPERX CallXL(int f, const T1& t1, const T2& t2, const T3& t3, const T4& t4)
  {
    return ExcelX(f, xlExtras::RavelOPERX(t1), xlExtras::RavelOPERX(t2), 
                  xlExtras::RavelOPERX(t3), xlExtras::RavelOPERX(t4));
  }

  template <class T1, class T2, class T3, class T4, class T5>
  OPERX CallXL(int f, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5)
  {
    return ExcelX(f, xlExtras::RavelOPERX(t1), xlExtras::RavelOPERX(t2), 
                  xlExtras::RavelOPERX(t3), xlExtras::RavelOPERX(t4), 
                  xlExtras::RavelOPERX(t5));
  }

  template <class T1, class T2, class T3, class T4, class T5, class T6>
  OPERX CallXL(int f, const T1& t1, const T2& t2, const T3& t3, const T4& t4,
               const T5& t5, const T6& t6)
  {
    return ExcelX(f, xlExtras::RavelOPERX(t1), xlExtras::RavelOPERX(t2), 
                  xlExtras::RavelOPERX(t3), xlExtras::RavelOPERX(t4), 
                  xlExtras::RavelOPERX(t5), xlExtras::RavelOPERX(t6));
  }

  template <class T1, class T2, class T3, class T4, class T5, class T6, class T7>
  OPERX CallXL(int f, const T1& t1, const T2& t2, const T3& t3, const T4& t4,
               const T5& t5, const T6& t6, const T7& t7)
  {
    return ExcelX(f, xlExtras::RavelOPERX(t1), xlExtras::RavelOPERX(t2), 
                  xlExtras::RavelOPERX(t3), xlExtras::RavelOPERX(t4), 
                  xlExtras::RavelOPERX(t5), xlExtras::RavelOPERX(t6),
                  xlExtras::RavelOPERX(t7));
  }

  template <class T1, class T2, class T3, class T4, class T5, class T6, class T7,
            class T8>
  OPERX CallXL(int f, const T1& t1, const T2& t2, const T3& t3, const T4& t4,
               const T5& t5, const T6& t6, const T7& t7, const T8& t8)
  {
    return ExcelX(f, xlExtras::RavelOPERX(t1), xlExtras::RavelOPERX(t2), 
                  xlExtras::RavelOPERX(t3), xlExtras::RavelOPERX(t4), 
                  xlExtras::RavelOPERX(t5), xlExtras::RavelOPERX(t6),
                  xlExtras::RavelOPERX(t7), xlExtras::RavelOPERX(t8));
  }

  template <class T1, class T2, class T3, class T4, class T5, class T6, class T7,
            class T8, class T9>
  OPERX CallXL(int f, const T1& t1, const T2& t2, const T3& t3, const T4& t4,
               const T5& t5, const T6& t6, const T7& t7, const T8& t8, const T9& t9)
  {
    return ExcelX(f, xlExtras::RavelOPERX(t1), xlExtras::RavelOPERX(t2), 
                  xlExtras::RavelOPERX(t3), xlExtras::RavelOPERX(t4), 
                  xlExtras::RavelOPERX(t5), xlExtras::RavelOPERX(t6),
                  xlExtras::RavelOPERX(t7), xlExtras::RavelOPERX(t8),
                  xlExtras::RavelOPERX(t9));
  }

  template <class T1, class T2, class T3, class T4, class T5, class T6, class T7,
            class T8, class T9, class T10>
  OPERX CallXL(int f, const T1& t1, const T2& t2, const T3& t3, const T4& t4,
               const T5& t5, const T6& t6, const T7& t7, const T8& t8, const T9& t9,
               const T10& t10)
  {
    return ExcelX(f, xlExtras::RavelOPERX(t1), xlExtras::RavelOPERX(t2), 
                  xlExtras::RavelOPERX(t3), xlExtras::RavelOPERX(t4), 
                  xlExtras::RavelOPERX(t5), xlExtras::RavelOPERX(t6),
                  xlExtras::RavelOPERX(t7), xlExtras::RavelOPERX(t8),
                  xlExtras::RavelOPERX(t9), xlExtras::RavelOPERX(t10));
  }

  template <class T1, class T2, class T3, class T4, class T5, class T6, class T7,
            class T8, class T9, class T10, class T11>
  OPERX CallXL(int f, const T1& t1, const T2& t2, const T3& t3, const T4& t4,
               const T5& t5, const T6& t6, const T7& t7, const T8& t8, const T9& t9,
               const T10& t10, const T11& t11)
  {
    return ExcelX(f, xlExtras::RavelOPERX(t1), xlExtras::RavelOPERX(t2), 
                  xlExtras::RavelOPERX(t3), xlExtras::RavelOPERX(t4), 
                  xlExtras::RavelOPERX(t5), xlExtras::RavelOPERX(t6),
                  xlExtras::RavelOPERX(t7), xlExtras::RavelOPERX(t8),
                  xlExtras::RavelOPERX(t9), xlExtras::RavelOPERX(t10),
                  xlExtras::RavelOPERX(t11));
  }

  template <class T1, class T2, class T3, class T4, class T5, class T6, class T7,
            class T8, class T9, class T10, class T11, class T12>
  OPERX CallXL(int f, const T1& t1, const T2& t2, const T3& t3, const T4& t4,
               const T5& t5, const T6& t6, const T7& t7, const T8& t8, const T9& t9,
               const T10& t10, const T11& t11, const T12& t12)
  {
    return ExcelX(f, xlExtras::RavelOPERX(t1), xlExtras::RavelOPERX(t2), 
                  xlExtras::RavelOPERX(t3), xlExtras::RavelOPERX(t4), 
                  xlExtras::RavelOPERX(t5), xlExtras::RavelOPERX(t6),
                  xlExtras::RavelOPERX(t7), xlExtras::RavelOPERX(t8),
                  xlExtras::RavelOPERX(t9), xlExtras::RavelOPERX(t10),
                  xlExtras::RavelOPERX(t11), xlExtras::RavelOPERX(t12));
  }

  template <class T1, class T2, class T3, class T4, class T5, class T6, class T7,
            class T8, class T9, class T10, class T11, class T12, class T13>
  OPERX CallXL(int f, const T1& t1, const T2& t2, const T3& t3, const T4& t4,
               const T5& t5, const T6& t6, const T7& t7, const T8& t8, const T9& t9,
               const T10& t10, const T11& t11, const T12& t12, const T13& t13)
  {
    return ExcelX(f, xlExtras::RavelOPERX(t1), xlExtras::RavelOPERX(t2), 
                  xlExtras::RavelOPERX(t3), xlExtras::RavelOPERX(t4), 
                  xlExtras::RavelOPERX(t5), xlExtras::RavelOPERX(t6),
                  xlExtras::RavelOPERX(t7), xlExtras::RavelOPERX(t8),
                  xlExtras::RavelOPERX(t9), xlExtras::RavelOPERX(t10),
                  xlExtras::RavelOPERX(t11), xlExtras::RavelOPERX(t12),
                  xlExtras::RavelOPERX(t13));
  }

  template <class T1, class T2, class T3, class T4, class T5, class T6, class T7,
            class T8, class T9, class T10, class T11, class T12, class T13, class T14>
  OPERX CallXL(int f, const T1& t1, const T2& t2, const T3& t3, const T4& t4,
               const T5& t5, const T6& t6, const T7& t7, const T8& t8, const T9& t9,
               const T10& t10, const T11& t11, const T12& t12, const T13& t13,
               const T14& t14)
  {
    return ExcelX(f, xlExtras::RavelOPERX(t1), xlExtras::RavelOPERX(t2), 
                  xlExtras::RavelOPERX(t3), xlExtras::RavelOPERX(t4), 
                  xlExtras::RavelOPERX(t5), xlExtras::RavelOPERX(t6),
                  xlExtras::RavelOPERX(t7), xlExtras::RavelOPERX(t8),
                  xlExtras::RavelOPERX(t9), xlExtras::RavelOPERX(t10),
                  xlExtras::RavelOPERX(t11), xlExtras::RavelOPERX(t12),
                  xlExtras::RavelOPERX(t13), xlExtras::RavelOPERX(t14));
  }

  //  workbook name. Unqualified name of the active sheet is returned
  //  in \a activeSheet
  inline std::string workbookName(std::string& activeSheet)
  {
    std::string fullSheetName = str<char>(CallXL(xlSheetNm, CallXL(xlSheetId)));
    size_t p = fullSheetName.find(']') + 1;
    activeSheet = fullSheetName.substr(p);
    return fullSheetName.substr(0, p);
  }

  inline std::string workbookName()
  {
    std::string tmp; return workbookName(tmp);
  }

  // return sheetId for given sheetName
  inline IDSHEET sheetId(std::string sheetName)
  {
    if (sheetName.find(']') == std::string::npos)
      sheetName = workbookName() + sheetName;
    return CallXL(xlSheetId, sheetName).val.mref.idSheet;
  }
			
  // used to indicate an OPERX that holds string data
  struct StrOPERX: public OPERX
  {
    StrOPERX() {}
    StrOPERX(const std::string& x): OPERX(utf_to_utf<TCHAR>(x).c_str()) {}
    StrOPERX& operator=(const OPERX& x) {
      assert(x.type() == xltypeStr);
      OPERX::operator=(x);
      return *this;
    }
  };


}

#ifdef _CLASSDESC
#pragma omit StrOPERX
#endif

inline void xml_pack(classdesc::xml_pack_t& x, const std::string& d, xlExtras::StrOPERX& a)
{
  xml_pack(x,d,xlExtras::str<char>(a));
}
inline void xml_unpack(classdesc::xml_unpack_t& x, const std::string& d, xlExtras::StrOPERX& a)
{
  std::string tmp;
  xml_unpack(x,d,tmp);
  a=xlExtras::StrOPERX(tmp);
}

/// declare a macro \a name
#define MACRO(name)                                             \
  static AddInX s_##name(_T("?xai_") _T(#name), _T(#name));      \
  int WINAPI xai_##name()

#endif
