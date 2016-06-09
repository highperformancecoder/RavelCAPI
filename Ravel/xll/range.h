// range.h - range functions
#pragma once
#include "xll.h"
/*
#ifndef _LIB
#pragma comment(linker, "/include:" XLL_DECORATE("xll_range_get", 8))
#pragma comment(linker, "/include:" XLL_DECORATE("xll_range_set", 4))
#pragma comment(linker, "/include:" XLL_DECORATE("xll_range_call", 8))
#endif // _LIB
*/
namespace range {
	template<class X>
	inline HANDLEX xll_range_setx(const XOPER<X>& r)
	{
		xll::handle<XOPER<X>> hr(new XOPER<X>(r));

		return hr.get();
	}
	
	template<class X>
	inline X* xll_range_getx(HANDLEX hr)
	{
		xll::handle<XOPER<X>> pr(hr);

		return pr.ptr();
	}
	
	template<class X>
	inline LXOPER<X> xll_range_callx(const XOPER<X>& f, const XOPER<X>& a)
	{
		XOPER<X> fa(f);

		if (fa.size() == 1)
			fa.push_back(a);

		// look up RANGE.SET values
		for (xll::traits<X>::xword i = 1; i < fa.size(); ++i) {
			if (fa[i].xltype == xltypeNum) {
				xll::handle<XOPER<X>> h(fa[i].val.num);
				if (h)
					fa[i] = *h.ptr(); // avoid copy!!!
			}
		}

		return Excelv<X>(xlUDF, fa.size(), fa.begin());
	}

	// implement take, drop, etc using iterators???
};