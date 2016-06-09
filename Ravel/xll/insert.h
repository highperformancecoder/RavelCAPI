// insert.h - insert a function template into a worksheet
// Copyright (c) 2012 KALX, LLC. All rights reserved. No warranty is made.
#include "addin.h"

using namespace xll;

#define EX Excel<X>

template<class X>
inline void
Move(int r, int c)
{
	EX(xlcSelect, EX(xlfOffset, EX(xlfActiveCell), XOPER<X>(r), XOPER<X>(c)));
}

template<class X>
inline int
xll_insertx(const XAddIn<X>& ai)
{
	try {
		const XArgs<X>& ar(ai.Arg());

		Move<X>(0,0);

		for (traits<X>::xword i = 0; i < ar.ArgCount(); ++i) {
			EX(xlSet, EX(xlfActiveCell), ar.Arg(i + 1).Name());
			EX(xlcFormatFont, MissingX(), MissingX(), BoolX(true)); // bold
			EX(xlcAlignment, OPERX(4)); // right

			Move<X>(0,1);
			EX(xlcDefineName, ar.Arg(i + 1).Name(), EX(xlfActiveCell));
			Move<X>(1,-1);
		}

		EX(xlSet, EX(xlfActiveCell), ar.FunctionText());
		EX(xlcFormatFont, MissingX(), MissingX(), MissingX(), BoolX(true)); // italic
		EX(xlcAlignment, OPERX(4)); // right
		Move<X>(0,1);


		OPERX fo(EX(xlfConcatenate, XOPER<X>("="), ar.FunctionText(), XOPER<X>("("), ar.ArgumentText(), XOPER<X>(")")));
		EX(xlcFormula, fo, EX(xlfActiveCell));
	}
	catch (const std::exception& ex) {
		XLL_ERROR(ex.what());

		return 0;
	}

	return 1;
}
