// auto.h - xlAutoXXX functions
// Copyright (c) 2011 KALX, LLC. All rights reserved. No warranty is made.
//
// Can't Auto<Open> void WINAPI foo(void) functions.
// Can't Auto<Open> mem_fun's.
// Use std::bind???
#pragma once
#include <functional>
#include <list>

extern HMODULE hModule; // from DllMain

// Export well known functions.
#ifdef _M_X64 
#pragma comment(linker, "/include:DllMain")
#pragma comment(linker, "/export:XLCallVer")
#pragma comment(linker, "/export:xlAutoOpen")
#pragma comment(linker, "/export:xlAutoClose")
#pragma comment(linker, "/export:xlAutoAdd")
#pragma comment(linker, "/export:xlAutoRemove")
#pragma comment(linker, "/export:xlAutoFree")
#pragma comment(linker, "/export:xlAutoFree12")
#pragma comment(linker, "/export:xlAutoRegister")
#pragma comment(linker, "/export:xlAutoRegister12")
//#pragma comment(linker, "/export:xlAddInManagerInfo") // default is add-in name
//#pragma comment(linker, "/export:xlAddInManagerInfo12")
#else
#pragma comment(linker, "/include:_DllMain@12")
#pragma comment(linker, "/export:_XLCallVer@0")
#pragma comment(linker, "/export:xlAutoOpen@0=xlAutoOpen")
#pragma comment(linker, "/export:xlAutoClose@0=xlAutoClose")
#pragma comment(linker, "/export:xlAutoAdd@0=xlAutoAdd")
#pragma comment(linker, "/export:xlAutoRemove@0=xlAutoRemove")
#pragma comment(linker, "/export:xlAutoFree@4=xlAutoFree")
#pragma comment(linker, "/export:xlAutoFree12@4=xlAutoFree12")
#pragma comment(linker, "/export:xlAutoRegister@4=xlAutoRegister")
#pragma comment(linker, "/export:xlAutoRegister12@4=xlAutoRegister12")
//#pragma comment(linker, "/export:xlAddInManagerInfo@4=xlAddInManagerInfo") // default is add-in name
//#pragma comment(linker, "/export:xlAddInManagerInfo12@4=xlAddInManagerInfo12")
#endif

#ifdef _DEBUG
#pragma comment(linker, "/export:?crtDbg@@3UCrtDbg@@A")
#endif

namespace xll {

	class Open {};
	class Open12 {};
	class OpenAfter {};
	class OpenAfter12 {};
	class Close {};
	class Close12 {};
	class Add {};
	class Add12 {};
	class RemoveBefore {};
	class RemoveBefore12 {};
	class Remove {};
	class Remove12 {};
	
#ifndef EXCEL12
	typedef Open         OpenX;
	typedef OpenAfter    OpenAfterX;
	typedef Close        CloseX;
	typedef Add          AddX;
	typedef RemoveBefore RemoveBeforeX;
	typedef Remove       RemoveX;
#else
	typedef Open12         OpenX;
	typedef OpenAfter12    OpenAfterX;
	typedef Close12        CloseX;
	typedef Add12          AddX;
	typedef RemoveBefore12 RemoveBeforeX;
	typedef Remove12       RemoveX;
#endif

	// Register macros to be called in xlAuto functions.
	template<class T>
	class Auto {
	public:
		typedef std::function<int(void)> macro;
		Auto(macro m)
		{
			macros().push_back(m);
		}
		static int Call(void)
		{
			int result(1);

			macro_list& m(macros());
			for (macro_iter i = m.begin(); i != m.end(); ++i) {
				result = (*i)();
				if (!result)
					return result;
			}

			return result;
		}
	private:
		typedef std::list<macro> macro_list;
		typedef macro_list::iterator macro_iter;
		static macro_list& macros(void)
		{
			static macro_list macros_;

			return macros_;
		}
	};
} // namespace xll
