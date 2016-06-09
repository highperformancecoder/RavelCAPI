// error.h - Error reporting functions
// Copyright (c) 2005-2011 KALX, LLC. All rights reserved.
#pragma once
#include "utility/registry.h"


// http://support.microsoft.com/kb/158552
//#ifndef _M_X64
//#pragma comment(linker, "/SECTION:.rsrc,rw")
//#endif

// work around for typeid::name memory leak
/*
template<class S>
class typeid_name {
	typeid_name(const typeid_name&);
	typeid_name & operator=(const typeid_name&);
	int flag_;
public:
	typeid_name()
		: flag_(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))
	{
		_CrtSetDbgFlag(flag_ &= ~_CRTDBG_ALLOC_MEM_DF);
	}
	~typeid_name()
	{
		_CrtSetDbgFlag(flag_);
	}
	operator const char*()
	{
		return typeid(S).name();
	}
};
*/
// !!!Doesn't work with Excel 2007 and above.
inline bool in_function_wizard(void)
{
	return !ExcelX(xlfGetTool, OPERX(4), OPERX(_T("Standard")), OPERX(1));
}

// error reporting macros
#ifndef _LIB
#ifdef _M_X64 
#pragma comment(linker, "/include:xll_alert_filter12")
#else
#pragma comment(linker, "/include:_xll_alert_filter@0")
#endif
#pragma comment(linker, "/include:?xll_alert_level@@3V?$Object@DK@Reg@@A")

#endif // _LIB

enum { 
	XLL_ALERT_ERROR   = 1,
	XLL_ALERT_WARNING = 2, 
	XLL_ALERT_INFO    = 4,
	XLL_ALERT_LOG     = 8	// turn on logging (not yet implemented)
};

int XLL_ERROR(const char* e, bool force = false);
int XLL_WARNING(const char* e, bool force = false);
int XLL_INFO(const char* e, bool force = false);
inline int XLL_INFORMATION(const char* e, bool force = false) { return XLL_INFO(e, force); }

extern Reg::Object<char, DWORD> xll_alert_level;
extern Reg::Object<wchar_t, DWORD> xll_alert_level12;

#ifdef EXCEL12
#define xll_alert_levelx xll_alert_level12
#else
#define xll_alert_levelx xll_alert_level
#endif