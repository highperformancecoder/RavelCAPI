// defines.h - Excel definitions
// Copyright (c) 2011 KALX, LLC. All rights reserved. No warranty is made.
#pragma once
#include <algorithm>
#include "utility/ensure.h"

#ifdef EXCEL12

// standard i18n defines
#undef _MBCS
#ifndef _UNICODE
#define _UNICODE
#endif
#ifndef UNICODE
#define UNICODE
#endif

#else

#ifndef _MBCS
#define _MBCS
#endif
#undef _UNICODE
#undef UNICODE

#endif

// http://support.microsoft.com/kb/166474
#define VC_EXTRALEAN 
#define WIN32_LEAN_AND_MEAN
// going bareback since the _s functions append null bytes
#pragma warning(disable: 4996)
#include <windows.h>
#include <tchar.h>

extern "C" {
#include "XLCALL.H"
}
// xlAuto* functions
#include "auto.h"

// don't use typedef
#ifdef EXCEL12
#define XLOPERX XLOPER12
#define XLREFX XLREF12
#else
#define XLOPERX XLOPER
#define XLREFX XLREF
#endif

#define dimof(x) ((sizeof(x))/(sizeof(*x)))

// Use #pragma XLLEXPORT in function body instead of DEF file.
#define XLLEXPORT comment(linker, "/export:" __FUNCDNAME__ "=" __FUNCTION__)

#define XLL_HASH_(x) #x
#define XLL_STRZ_(x) XLL_HASH_(x)

// usage: AddInX xai_xyz(DocumentX(CATEGORY).Documentation(XML_FILE("file.xml")));
// #include "utility/file.h" and set C/C++ > Advanced > Use Full Paths = Yes(/FC)
#define XML_FILE(file) xml::File(Directory::Basename(_T(__FILE__)).append(file).c_str())

// Use xll::Enum instead.
// #pragma deprecated("XLL_ENUM", "XLL_ENUM_DOC")

// decorate extern "C" symbol for platform
#ifdef _M_X64
#define XLL_DECORATE(s,n) s
#define XLL_DECORATE12(s,n) s
#else // !_M_X64
#define XLL_DECORATE(s,n) "_" s "@" XLL_STRZ_(n)
#define XLL_DECORATE12(s,n) L"_" s L"@" XLL_STRZ_(n)
#endif

#ifdef EXCEL12
#define XLL_DECORATEX XLL_DECORATE12
#else
#define XLL_DECORATEX XLL_DECORATE
#endif


// Used for single source dual use.
#ifdef EXCEL12
#define X_(f) f##12
#define TX_(s) _T(s) _T("12")
#else
#define X_(f) f
#define TX_(s) s
#endif

// Evaluates to XLOPER or XLOPER12 depending on EXCEL12 being undefined or defined
typedef X_(XLOPER) *LPXLOPERX;

// Excel C data types for xlfRegister/AddIn.
#define XLL_BOOL     "A"  // short int used as logical
#define XLL_DOUBLE   "B"  // double
#define XLL_CSTRING  "C"  // char * to C style NULL terminated string (up to 255 characters)
#define XLL_PSTRING  "D"  // unsigned char * to Pascal style byte counted string (up to 255 characters)
#define XLL_DOUBLE_  "E"  // pointer to double
#define XLL_CSTRING_ "F"  // reference to a null terminated string
#define XLL_PSTRING_ "G"  // reference to a byte counted string
#define XLL_USHORT   "H"  // unsigned 2 byte int
#define XLL_WORD     "H"  // unsigned 2 byte int
#define XLL_SHORT    "I"  // signed 2 byte int
#define XLL_LONG     "J"  // signed 4 byte int
#define XLL_FP       "K"  // pointer to struct FP
#define XLL_BOOL_    "L"  // reference to a boolean
#define XLL_SHORT_   "M"  // reference to signed 2 byte int
#define XLL_LONG_    "N"  // reference to signed 4 byte int
#define XLL_LPOPER   "P"  // pointer to OPER struct (never a reference type).
#define XLL_LPXLOPER "R"  // pointer to XLOPER struct
// Modify add-in function behaviour.
#define XLL_VOLATILE "!"  // called every time sheet is recalced
#define XLL_UNCALCED "#"  // dereferencing uncalced cells returns old value
#define XLL_VOID ">"	// in-place function.
#define XLL_THREAD_SAFE "" // declares function to be thread safe
#define XLL_CLUSTER_SAFE ""	// declares function to be cluster safe
#define XLL_ASYNCHRONOUS ""	// declases function to be asynchronous
// Special data types for doubles
#define XLL_DATE "B1"
#define XLL_HANDLE "B2"
 
// Extensions
//#define XLL_HANDLE XLL_DOUBLE

#define XLL_BOOL12     L"A"  // short int used as logical
#define XLL_DOUBLE12   L"B"  // double
#define XLL_CSTRING12  L"C%" // XCHAR* to C style NULL terminated unicode string
#define XLL_PSTRING12  L"D%" // XCHAR* to Pascal style byte counted unicode string
#define XLL_DOUBLE_12  L"E"  // pointer to double
#define XLL_CSTRING_12 L"F%" // reference to a null terminated unicode string
#define XLL_PSTRING_12 L"G%" // reference to a byte counted unicode string
#define XLL_USHORT12   L"H"  // unsigned 2 byte int
#define XLL_WORD12     L"J"  // signed 4 byte int
#define XLL_SHORT12    L"I"  // signed 2 byte int
#define XLL_LONG12     L"J"  // signed 4 byte int
#define XLL_FP12       L"K%" // pointer to struct FP
#define XLL_BOOL_12    L"L"  // reference to a boolean
#define XLL_SHORT_12   L"M"  // reference to signed 2 byte int
#define XLL_LONG_12    L"N"  // reference to signed 4 byte int
#define XLL_LPOPER12   L"Q"  // pointer to OPER struct (never a reference type).
#define XLL_LPXLOPER12 L"U"  // pointer to XLOPER struct
// Modify add-in function behaviour.
#define XLL_VOLATILE12 L"!"  // called every time sheet is recalced
#define XLL_UNCALCED12 L"#"  // dereferencing uncalced cells returns old value
#define XLL_THREAD_SAFE12 L"$" // declares function to be thread safe
#define XLL_CLUSTER_SAFE12 L"&"	// declares function to be cluster safe
#define XLL_ASYNCHRONOUS12 L"X"	// declases function to be asynchronous
#define XLL_VOID12     L">"	// return type to use for asynchronous functions
// Special data types for doubles
#define XLL_DATE12 L"B1"
#define XLL_HANDLE12 L"B2"

#ifdef EXCEL12
#define XLL_BOOLX     XLL_BOOL12
#define XLL_DOUBLEX   XLL_DOUBLE12
#define XLL_CSTRINGX  XLL_CSTRING12
#define XLL_PSTRINGX  XLL_PSTRING12
#define XLL_DOUBLE_X  XLL_DOUBLE_12
#define XLL_CSTRING_X XLL_CSTRING_12
#define XLL_PSTRING_X XLL_PSTRING_12
#define XLL_USHORTX   XLL_USHORT12
#define XLL_WORDX     XLL_WORD12
#define XLL_SHORTX    XLL_SHORT12
#define XLL_LONGX     XLL_LONG12
#define XLL_FPX       XLL_FP12
#define XLL_BOOL_X    XLL_BOOL_12
#define XLL_SHORT_X   XLL_SHORT_12
#define XLL_LONG_X    XLL_LONG_12
#define XLL_LPOPERX   XLL_LPOPER12
#define XLL_LPXLOPERX XLL_LPXLOPER12
#define XLL_VOLATILEX XLL_VOLATILE12
#define XLL_UNCALCEDX XLL_UNCALCED12
#define XLL_THREAD_SAFEX XLL_THREAD_SAFE12
#define XLL_CLUSTER_SAFEX XLL_CLUSTER_SAFE12
#define XLL_VOIDX XLL_VOID12
// Special data types for doubles
#define XLL_DATEX XLL_DATE12
#define XLL_HANDLEX XLL_HANDLE12

#else
#define XLL_BOOLX     XLL_BOOL
#define XLL_DOUBLEX   XLL_DOUBLE
#define XLL_CSTRINGX  XLL_CSTRING
#define XLL_PSTRINGX  XLL_PSTRING
#define XLL_DOUBLE_X  XLL_DOUBLE_
#define XLL_CSTRING_X XLL_CSTRING_
#define XLL_PSTRING_X XLL_PSTRING_
#define XLL_USHORTX   XLL_USHORT
#define XLL_WORDX     XLL_WORD
#define XLL_SHORTX    XLL_SHORT
#define XLL_LONGX     XLL_LONG
#define XLL_FPX       XLL_FP
#define XLL_BOOL_X    XLL_BOOL_
#define XLL_SHORT_X   XLL_SHORT_
#define XLL_LONG_X    XLL_LONG_
#define XLL_LPOPERX   XLL_LPOPER
#define XLL_LPXLOPERX XLL_LPXLOPER
#define XLL_VOLATILEX XLL_VOLATILE
#define XLL_UNCALCEDX XLL_UNCALCED
#define XLL_THREAD_SAFEX XLL_THREAD_SAFE
#define XLL_CLUSTER_SAFEX XLL_CLUSTER_SAFE
#define XLL_VOIDX XLL_VOID
#define XLL_DATEX XLL_DATE
#define XLL_HANDLEX XLL_HANDLE

#endif

// Convenience wrappers for Excel calls
#define XLL_XLF(fn, ...) Excel<XLOPERX>(xlf##fn, __VA_ARGS__)
#define XLL_XLC(fn, ...) Excel<XLOPERX>(xlc##fn, __VA_ARGS__)
#define XLL_XL_(fn, ...) Excel<XLOPERX>(xl##fn, __VA_ARGS__)
#define XLL_MOVE(r, c) XLL_XLC(Select, XLL_XLF(Offset, XLL_XLF(ActiveCell), OPERX(r), OPERX(c)))

#ifdef _M_X64

// Enumerated type XLL_ENUM(C_VALUE, ExcelName, _T("Category"), _T("Description"))
#define XLL_ENUM(value, name, cat, desc) static xll::AddInX xai_##name(   \
    _T(XLL_STRZ_(xll_##name)), XLL_LPOPERX, _T(#name), _T(""), cat, desc _T(" ")); \
    extern "C" __declspec(dllexport) LPOPERX WINAPI xll_##name(void)      \
	{ static OPERX o(value); return &o; }

#define XLL_ENUM_DOC(value, name, cat, desc, doc) static xll::AddInX xai_##name(   \
    FunctionX(XLL_LPOPERX, _T(XLL_STRZ_(xll_##name)), _T(#name)) \
	.Category(cat).FunctionHelp(desc _T(" ")).Documentation(doc)); \
    extern "C" __declspec(dllexport) LPOPERX WINAPI xll_##name(void)      \
	{ static OPERX o(value); return &o; }

#define XLL_ENUM_SORT(group, value, name, cat, desc, doc) static xll::AddInX xai_##name(   \
    FunctionX(XLL_LPOPERX, _T(XLL_STRZ_(xll_##name)), _T(#name)) \
	.Category(cat).FunctionHelp(desc _T(" ")).Documentation(doc) \
	.Sort(_T(#group), OPERX(value))); \
    extern "C" __declspec(dllexport) LPOPERX WINAPI xll_##name(void)      \
	{ static OPERX o(value); return &o; }

// member type, C struct, C member, Excel Type Struct.Member, category, description, documentation
#define XLL_MEMBER(t, s, m, T, S, M, cat, desc, doc) \
	static xll::AddIn xai_##s##_##m(Function(T, XLL_STRZ_(xll_##s##_##m), S "." M) \
	.Arg(XLL_HANDLEX, S, "is a handle to a " S " that contains " M ". ") \
	.Category(cat).FunctionHelp(desc).Documentation(doc)); \
	extern "C" __declspec(dllexport) t WINAPI xll_##s##_##m(HANDLEX h) { \
		return !xll::handle_name::exists(h) ? 0 : h2p<s>(h)->m; } \
	static xll::Enum xen_##s##_##m(Name(typeid_name<s>()).Item(M, S "." M, desc));

// for members that are pointer types
#define XLL_MEMBER_(t, s, m, T, S, M, cat, desc, doc) \
	static xll::AddIn xai_##s##_##m(Function(XLL_HANDLE, XLL_STRZ_(xll_##s##_##m), S "." M) \
	.Arg(XLL_HANDLEX, S, "is a handle to a " S " that contains a pointer to " M ". ") \
	.Category(cat).FunctionHelp(desc).Documentation(doc)); \
	extern "C" __declspec(dllexport) HANDLEX WINAPI xll_##s##_##m(HANDLEX h) { \
		return !xll::handle_name::exists(h) ? 0 : p2h<void>(h2p<s>(h)->m); } \
	static xll::Enum xen_##s##_##m(Name(typeid_name<s>()).Item(M, S "." M, desc));

// For one dimensional arrays
#define XLL_MEMBER_1D
// For two dimensional arrays
#define XLL_MEMBER_2D

#else

// Enumerated type XLL_ENUM(C_VALUE, ExcelName, _T("Category"), _T("Description"))
#define XLL_ENUM(value, name, cat, desc) static xll::AddInX xai_##name(   \
    _T(XLL_STRZ_(_xll_##name##@0)), XLL_LPOPERX, _T(#name), _T(""), cat, desc _T(" ")); \
    extern "C" __declspec(dllexport) LPOPERX WINAPI xll_##name(void)      \
	{ static OPERX o(value); return &o; }

#define XLL_ENUM_DOC(value, name, cat, desc, doc) static xll::AddInX xai_##name(   \
    FunctionX(XLL_LPOPERX, _T(XLL_STRZ_(_xll_##name##@0)), _T(#name)) \
	.Category(cat).FunctionHelp(desc _T(" ")).Documentation(doc)); \
    extern "C" __declspec(dllexport) LPOPERX WINAPI xll_##name(void)      \
	{ static OPERX o(value); return &o; }

// sort by group then by name
#define XLL_ENUM_SORT(group, value, name, cat, desc, doc) static xll::AddInX xai_##name(   \
    FunctionX(XLL_LPOPERX, _T(XLL_STRZ_(_xll_##name##@0)), _T(#name)) \
	.Category(cat).FunctionHelp(desc _T(" ")).Documentation(doc) \
	.Sort(_T(#group), OPERX(value))); \
    extern "C" __declspec(dllexport) LPOPERX WINAPI xll_##name(void)      \
	{ static OPERX o(value); return &o; }
/*
// member type, C struct, C member, Excel Type Struct.Member, category, description, documentation
#define XLL_MEMBER(t, s, m, T, S, M, cat, desc, doc) \
	static xll::AddIn xai_##s##_##m(Function(T, XLL_STRZ_(_xll_##s##_##m@8), S "." M) \
	.Arg(XLL_HANDLEX, S, "is a handle to a " S " that contains " M ". ") \
	.Category(cat).FunctionHelp(desc).Documentation(doc)); \
	extern "C" __declspec(dllexport) t WINAPI xll_##s##_##m(HANDLEX h) { \
		return !xll::handle_name::exists(h) ? 0 : h2p<s>(h)->m; } \
	static xll::Enum xen_##s##_##m(Name(typeid_name<s>()).Item(M, S "." M, desc));

// for members that are pointer types
#define XLL_MEMBER_(t, s, m, T, S, M, cat, desc, doc) \
	static xll::AddIn xai_##s##_##m(Function(XLL_HANDLE, XLL_STRZ_(_xll_##s##_##m@8), S "." M) \
	.Arg(XLL_HANDLEX, S, "is a handle to a " S " that contains a pointer to " M ". ") \
	.Category(cat).FunctionHelp(desc).Documentation(doc)); \
	extern "C" __declspec(dllexport) HANDLEX WINAPI xll_##s##_##m(HANDLEX h) { \
		return !xll::handle_name::exists(h) ? 0 : p2h<void>(h2p<s>(h)->m); } \
	static xll::Enum xen_##s##_##m(Name(typeid_name<s>()).Item(M, S "." M, desc));
*/
// For one dimensional arrays
#define XLL_MEMBER_1D
// For two dimensional arrays
#define XLL_MEMBER_2D
#endif // _M_X64

// std::default_delete for Windows handles
// e.g. HANDLE_DEFAULT_DELETE(HANDLE, CloseHandle)
#define HANDLE_DEFAULT_DELETE(T, D) namespace std { template<> struct default_delete<T> { void operator()(T* p) { D(*p); delete p; } }; }
// Type, pointer, open call
// e.g., UNIQUE_HANDLE(HINTERNET, p, InternetOpen(_T("Agent"), INTERNET_OPEN_TYPE_DIRECT, 0, 0, INTERNET_FLAG_ASYNC));
#define UNIQUE_HANDLE(T, p, O) std::unique_ptr<T> p(new T(O))