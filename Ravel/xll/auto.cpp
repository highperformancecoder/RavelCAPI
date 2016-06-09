// auto.cpp - Implement well-known xlAutoXXX entry points required for xll's.
// Copyright (c) 2011 KALX, LLC. All rights reserved. No warranty is made.
#include "xll.h"

using namespace xll;

// Called by Excel when the xll is opened.
extern "C" 
int __declspec(dllexport) WINAPI
xlAutoOpen(void)
{
	try {
		ensure (Auto<Open>::Call());
		ensure (AddIn::RegisterAll());
		ensure (Auto<OpenAfter>::Call());

		if (XLCallVer() >= 0x0C00) {
			ensure (Auto<Open12>::Call());
			ensure (AddIn12::RegisterAll());
			ensure (Auto<OpenAfter12>::Call());
		}

		// register OnXXX macros
		ensure (On<Data>::Open());
		ensure (On<Doubleclick>::Open());
		ensure (On<Entry>::Open());
		ensure (On<Key>::Open());
		ensure (On<Recalc>::Open());
		ensure (On<Sheet>::Open());
		ensure (On<Time>::Open());
		ensure (On<Window>::Open());
	}
	catch (const std::exception& ex) {
		Excel<XLOPER>(xlcAlert, Str(ex.what()));

		return 0;
	}

	return 1;
}

extern "C"
int __declspec(dllexport) WINAPI
xlAutoClose(void)
{
	try {
		ensure (Auto<Close>::Call());
		if (XLCallVer() >= 0x0C00) {
			ensure (Auto<Close12>::Call());
		}
	}
	catch (const std::exception& ex) {
		Excel<XLOPER>(xlcAlert, Str(ex.what()));

		return 0;
	}

	return 1;
}

extern "C"
int __declspec(dllexport) WINAPI
xlAutoAdd(void)
{
	try {
		ensure (Auto<Add>::Call());
		if (XLCallVer() >= 0x0C00) {
			ensure (Auto<Add12>::Call());
		}
	}
	catch (const std::exception& ex) {
		Excel<XLOPER>(xlcAlert, Str(ex.what()));

		return 0;
	}

	return 1;
}

extern "C"
int __declspec(dllexport) WINAPI
xlAutoRemove(void)
{
	try {
		ensure (Auto<RemoveBefore>::Call());
		ensure (AddIn::UnregisterAll());
		ensure (Auto<Remove>::Call());

		if (XLCallVer() >= 0x0C00) {
			ensure (Auto<RemoveBefore12>::Call());
			ensure (AddIn12::UnregisterAll());
			ensure (Auto<Remove12>::Call());
		}

		// unregister OnXXX macros
		ensure (On<Data>::Close());
		ensure (On<Doubleclick>::Close());
		ensure (On<Entry>::Close());
		ensure (On<Key>::Close());
		ensure (On<Recalc>::Close());
		ensure (On<Sheet>::Close());
		ensure (On<Time>::Close());
		ensure (On<Window>::Close());

	}
	catch (const std::exception& ex) {
		Excel<XLOPER>(xlcAlert, Str(ex.what()));

		return 0;
	}

	return 1;
}

extern "C"
void __declspec(dllexport) WINAPI
xlAutoFree(LPOPER px)
{
	if ((px->xltype)&xlbitDLLFree)
		delete px;
	else if ((px->xltype)&xlbitXLFree)
		Excel4(xlFree, 0, 1, px);
}

extern "C"
void __declspec(dllexport) WINAPI
xlAutoFree12(LPOPER12 px)
{
	if ((px->xltype)&xlbitDLLFree)
		delete px;
	else if ((px->xltype)&xlbitXLFree)
		Excel12(xlFree, 0, 1, px);
}

extern "C"
LPXLOPER __declspec(dllexport) WINAPI 
xlAutoRegister(LPXLOPER pxName)
{
	static OPER xResult;

	try {
		const Args* parg = ArgsMap::Find(*pxName);

		if (parg) {
			xResult = xll::Register(*parg);
		}
	}
	catch (const std::exception& ex) {
		XLL_ERROR(ex.what());

		xResult = Err(xlerrValue);
	}

	return &xResult;
}

extern "C"
LPXLOPER12 __declspec(dllexport) WINAPI 
xlAutoRegister12(LPXLOPER12 pxName)
{
	static OPER12 xResult;

	try {
		const Args12* parg = ArgsMap12::Find(*pxName);

		if (parg) {
			xResult = xll::Register(*parg);
		}
	}
	catch (const std::exception& ex) {
		XLL_ERROR(ex.what());

		xResult = Err12(xlerrValue);
	}

	return &xResult;
}
