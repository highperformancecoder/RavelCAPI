// traits.h - Parameterize by XLOPER and XLOPER12.
// Copyright (c) 2011 KALX, LLC. All rights reserved. No warranty is made.
#pragma once
#include "limits.h"

namespace xll {

	template<class X>
	struct traits { };

	template<>
	struct traits<XLOPER> {
		typedef XLOPER type;
		typedef XLOPER* pointer_type;
		typedef unsigned short xbool;
		typedef short int xint;
		typedef unsigned short xword;
		typedef char xchar;
		typedef const char* xcstr;
		typedef _FP xfp;
		typedef WORD xrw;
		typedef BYTE xcol;
		typedef std::basic_string<xchar> xstring;
		typedef XLREF xlref;
		typedef XLMREF xlmref;

		typedef WORD xltype;

		static const int strmax = 255; // maximum number of characters in a str

		static int Excel(int xlfn, LPXLOPER operRes, int count, ...)
		{
			LPXLOPER arg[limits<XLOPER>::maxargs];

			ensure (count <= limits<XLOPER>::maxargs);

			va_list ap;
			va_start(ap, count);
			for (int i = 0; i < count; ++i)
				arg[i] = va_arg(ap, LPXLOPER);
			va_end(ap);

			return Excel4v(xlfn, operRes, count, &arg[0]);
		}
		static int Excelv(int xlfn, LPXLOPER operRes, int count, const LPXLOPER opers[])
		{
			return Excel4v(xlfn, operRes, count, const_cast<LPXLOPER*>(opers));
		}

		static xchar* strncpy(xchar* dest, xcstr src, xchar n)
		{
			return ::strncpy(dest, src, n);
		}
		static int strnicmp(xcstr s, xcstr t, xchar n)
		{
			return ::_strnicmp(s, t, n);
		}
		static xchar strlen(xcstr s)
		{
			if (!s || !*s)
				return 0;

			size_t n = ::strlen(s);
			ensure (n <= limits<XLOPER>::maxchars);

			return static_cast<xchar>(n);
		}
		static xcstr null(void)
		{
			return "";
		}
		static const XLOPER& format(void)
		{
			static XLOPER fmt;

			fmt.xltype = xltypeStr;
			fmt.val.str = "\01@";

			return fmt;
		}
		static const XLOPER& dotll(void) {
			static XLOPER dll;

			dll.xltype = xltypeStr;
			dll.val.str = "\04.?ll";

			return dll;
		}
		static const XLOPER& chmbang(void) {
			static XLOPER chm;

			chm.xltype = xltypeStr;
			chm.val.str = "\04chm!";

			return chm;
		}
		static int tolower(xchar c)
		{
			return ::tolower(c);
		}
		static xcstr Err(int type)
		{
			switch (type) {
			case xlerrNull:  return "#NULL!";
			case xlerrDiv0:  return "#DIV/0!";
			case xlerrValue: return "#VALUE!";
			case xlerrRef:   return "#REF!";
			case xlerrName:  return "#NAME?";
			case xlerrNum:   return "#NUM!";
			case xlerrNA:    return "#N/A";
			}

			return "#UNK!?";
		}
	};

	template<>
	struct traits<XLOPER12> {
		typedef XLOPER12 type;
		typedef XLOPER12* pointer_type;
		typedef int xbool;
		typedef int xint;
		typedef INT32 xword;
		typedef wchar_t xchar;
		typedef const wchar_t* xcstr;
		typedef _FP12 xfp;
		typedef RW xrw;
		typedef COL xcol;
		typedef std::basic_string<xchar> xstring;
		typedef XLREF12 xlref;
		typedef XLMREF12 xlmref; 

		typedef DWORD xltype;

		static const int strmax = 32767;

		static int Excel(int xlfn, LPXLOPER12 operRes, int count, ...)
		{
			LPXLOPER12 arg[limits<XLOPER12>::maxargs];

			ensure (count <= limits<XLOPER12>::maxargs);

			va_list ap;
			va_start(ap, count);
			for (int i = 0; i < count; ++i)
				arg[i] = va_arg(ap, LPXLOPER12);
			va_end(ap);

			return Excel12v(xlfn, operRes, count, &arg[0]);
		}
		static int Excelv(int xlfn, LPXLOPER12 operRes, int count, const LPXLOPER12 opers[])
		{
			return Excel12v(xlfn, operRes, count, const_cast<LPXLOPER12*>(opers));
		}

		static xchar* strncpy(xchar* dest, xcstr src, xchar n)
		{
			return ::wcsncpy(dest, src, n);
		}
		static int strnicmp(xcstr s, xcstr t, xchar n)
		{
			return ::_wcsnicmp(s, t, n);
		}
		static xchar strlen(xcstr s)
		{
			if (!s || !*s)
				return 0;

			size_t n = ::wcslen(s);
			ensure (n <= limits<XLOPER12>::maxchars);

			return static_cast<xchar>(n);
		}
		static xcstr null(void)
		{
			return L"";
		}
		static const XLOPER12& format(void)
		{
			static XLOPER12 fmt;

			fmt.xltype = xltypeStr;
			fmt.val.str = L"\01@";

			return fmt;
		}
		static const XLOPER12& dotll(void) {
			static XLOPER12 dll;

			dll.xltype = xltypeStr;
			dll.val.str = L"\04.?ll";

			return dll;
		}
		static const XLOPER12& chmbang(void) {
			static XLOPER12 chm;

			chm.xltype = xltypeStr;
			chm.val.str = L"\04chm!";

			return chm;
		}
		static int tolower(xchar c)
		{
			return ::towlower(c);
		}
		static xcstr Err(int type)
		{
			switch (type) {
			case xlerrNull:  return L"#NULL!";
			case xlerrDiv0:  return L"#DIV/0!";
			case xlerrValue: return L"#VALUE!";
			case xlerrRef:   return L"#REF!";
			case xlerrName:  return L"#NAME?";
			case xlerrNum:   return L"#NUM!";
			case xlerrNA:    return L"#N/A";
			}

			return L"#UNK!?";
		}

	};

} // namespance xll