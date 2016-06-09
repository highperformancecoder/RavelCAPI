// range.cpp - range functions
#include "range.h"

using namespace range;
using namespace xll;

static const char* xav_range_set[] = {
	"is a range. "
};
static AddIn xai_range_set(
	XLL_DECORATE("xll_range_set", 4), XLL_HANDLEX XLL_LPOPER XLL_UNCALCED,
	"RANGE.SET", "Range",
	"Range", "Return a handle to an in-memory Range.",
	1, xav_range_set);
extern "C" __declspec(dllexport) HANDLEX WINAPI
xll_range_set(const LPOPER pr)
{
	return xll_range_setx<XLOPER>(*pr);
}

static const char* xav_range_get[] = {
	"is a handle to a range returned by RANGE.SET. "
};
static AddIn xai_range_get(
	XLL_DECORATE("xll_range_get", 8), XLL_LPXLOPER XLL_HANDLEX,
	"RANGE.GET", "Handle",
	"Range", "Return an in-memory range referred to by Handle.",
	1, xav_range_set);
extern "C" __declspec(dllexport) LPXLOPER WINAPI
xll_range_get(HANDLEX hr)
{
	return xll_range_getx<XLOPER>(hr);
}

static const char* xav_range_call[] = {
	"is a user defined function or a range having first cell a UDF.",
	"is an optional range of arguments to pass to Function. "
};
static AddIn xai_range_call(
	XLL_DECORATE("xll_range_call", 8), XLL_LPXLOPER XLL_LPOPER XLL_LPOPER XLL_UNCALCED,
	"RANGE.CALL", "Function, Arguments",
	"Range", "Call Function on Arguments. ",
	1, xav_range_call);
extern "C" __declspec(dllexport) LPXLOPER WINAPI
xll_range_call(const LPOPER pf, const LPOPER pa)
{
	static LOPER o;
	
	o = xll_range_callx<XLOPER>(*pf, *pa);

	return o.xlfree();
}

#if 0
#ifdef _DEBUG

template<class X>
void xll_test_rangex(const XOPER<X>& f)
{
	XOPER<X> o(10,20);

	o(4,5) = 1.23;

	HANDLEX h = xll_range_setx<X>(o);
//	const XOPER<X>& o_ = *xll_range_getx<X>(h);
	XOPER<X> o_ = *xll_range_getx<X>(h);

	ensure (o_(4,5) == 1.23);
/*
	LXOPER<X> p = xll_range_callx<X>(Excel<X>(xlfEvaluate, f), o);
	ensure (p.xltype == xltypeNum);
	xll::handle<XOPER<X>> h_(p.val.num);
	ensure (h_->operator()(4, 5) == 1.23);
*/
}

int xll_test_range(void)
{
	try {
		xll_test_rangex<XLOPER>(OPER("RANGE.SET"));
	}
	catch (const std::exception& ex) {
		XLL_ERROR(ex.what());

		return 0;
	}

	return 1;
}
static Auto<OpenAfter> xao_test_range(xll_test_range);

#endif // _DEBUG

#endif