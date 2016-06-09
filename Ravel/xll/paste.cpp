// paste.cpp - Paste arguments into Excel given function name or register id.
// If it is a number, just paste the default arguments and do not define names.
// If it is a string, use that as a rdb prefix for defined names.
#include "addin.h"
#include "error.h"
#include "on.h"
#include "paste.h"

using namespace xll;

typedef traits<XLOPERX>::xword xword;

template<class X>
struct select {
	static XOPER<X> up;
	static XOPER<X> down;
	static XOPER<X> right;
	static XOPER<X> left;
	static XOPER<X> alert;
	static XOPER<X> input;
	static XOPER<X> r_;
	static XOPER<X> c0;
	static XOPER<X> range_set;
	static XOPER<X> range_get;
};

OPER select<XLOPER>::up = OPER("R[-1]C[0]");
OPER select<XLOPER>::down = OPER("R[1]C[0]");
OPER select<XLOPER>::right = OPER("R[0]C[1]");
OPER select<XLOPER>::left = OPER("R[0]C[-1]");
OPER select<XLOPER>::alert = OPER("Would you like to replace the existing definition of ");
OPER select<XLOPER>::input = OPER("Enter the range name.");
OPER select<XLOPER>::r_ = OPER("R[");
OPER select<XLOPER>::c0 = OPER("]C[0]");
OPER select<XLOPER>::range_set = OPER("=RANGE.SET(");
OPER select<XLOPER>::range_get = OPER("=RANGE.GET(");

OPER12 select<XLOPER12>::up = OPER12(L"R[-1]C[0]");
OPER12 select<XLOPER12>::down = OPER12(L"R[1]C[0]");
OPER12 select<XLOPER12>::right = OPER12(L"R[0]C[1]");
OPER12 select<XLOPER12>::left = OPER12(L"R[0]C[-1]");
OPER12 select<XLOPER12>::alert = OPER12(L"Would you like to replace the existing definition of ");
OPER12 select<XLOPER12>::input = OPER12(L"Enter the range name.");
OPER12 select<XLOPER12>::r_ = OPER12(L"R[");
OPER12 select<XLOPER12>::c0 = OPER12(L"]C[0]");
OPER12 select<XLOPER12>::range_set = OPER12(L"=RANGE.SET(");
OPER12 select<XLOPER12>::range_get = OPER12(L"=RANGE.GET(");


#define UP Excel<X>(xlcSelect, XOPER<X>(select<X>::up))
#define DOWN Excel<X>(xlcSelect, XOPER<X>(select<X>::down))
#define RIGHT Excel<X>(xlcSelect, XOPER<X>(select<X>::right))
#define LEFT Excel<X>(xlcSelect, XOPER<X>(select<X>::left))

template<class X>
inline XOPER<X> char1(typename traits<X>::xchar c)
{
	return XOPER<X>(&c, 1);
}
template<class X>
inline XOPER<X>& append(XOPER<X>& x, const XOPER<X> y)
{
	return x = Excel<X>(xlfConcatenate, x, y);
}

enum Style {
	STYLE_INPUT,
	STYLE_OUTPUT,
	STYLE_HANDLE,
	STYLE_OPTIONAL
};
OPER StyleName[] = {
	OPER("Input"),
	OPER("Output"),
	OPER("Handle"),
	OPER("Optional")
};
inline void
ApplyStyle(Style s)
{
	Excel<XLOPER>(xlcDefineStyle, StyleName[s]);
	Excel<XLOPER>(xlcApplyStyle, StyleName[s]);
}

// paste argument into xActi, return reference for formula
template<class X>
inline XOPER<X> paste_default(const XOPER<X>& xAct, const XOPER<X>& xActi, const XOPER<X>& xDef)
{
	XOPER<X> xRel = Excel<X>(xlfRelref, xActi, xAct);

	if (xDef && xDef.xltype == xltypeStr && xDef.val.str[0] > 0 && xDef.val.str[1] == '=') {
		XOPER<X> xEval = Excel<X>(xlfEvaluate, xDef);
		if (xEval.size() > 1) {
			XOPER<X> xOff = Excel<X>(xlfOffset, xActi, XOPER<X>(0), XOPER<X>(0), XOPER<X>(1), XOPER<X>(xEval.size()));
			Excel<X>(xlSet, xOff, xEval);

			xRel = Excel<X>(xlfRelref, xOff, xAct);
		}
		else {
			Excel<X>(xlcFormula, xDef);
		}
	}
	else {
		Excel<X>(xlSet, xActi, xDef);
	}

	return xRel;
}

template<class X>
void xll_paste_regid(const XArgs<X>* pargs)
{
	XOPER<X> xAct = Excel<X>(xlfActiveCell);

	XOPER<X> xFor(char1<X>('='));
	append(xFor, pargs->FunctionText());
	append(xFor, char1<X>('('));

	for (unsigned short i = 1; i < pargs->ArgCount(); ++i) {
		DOWN;

		XOPER<X> xActi = Excel<X>(xlfActiveCell);
		XOPER<X> xDef = pargs->Arg(i).Default();
		XOPER<X> xRel = paste_default(xAct, xActi, xDef);

		if (i > 1) {
			append(xFor, char1<X>(','));
			append(xFor, char1<X>(' '));
		}
		append(xFor, xRel);
	}
	append(xFor, char1<X>(')'));

	Excel<X>(xlcSelect, xAct);
	Excel<X>(xlcFormula, xFor);
}

void
xll_paste_regidx(void)
{
	const Args* parg;
	const Args12* parg12;

	double regid = Excel<XLOPER>(xlCoerce, Excel<XLOPER>(xlfActiveCell)).val.num;

	if (0 != (parg = ArgsMap::Find(regid))) {
		if (parg->isFunction())
			return xll_paste_regid(parg);
	}
	else if (0 != (parg12 = ArgsMap12::Find(regid))) {
		if (parg12->isFunction())
			return xll_paste_regid(parg12);
	}
	else {
		throw std::runtime_error("XLL.PASTE.FUNCTION: register id not found");
	}
}

template<class X>
void xll_paste_name(const XArgs<X>* pargs)
{
	XOPER<X> xAct = Excel<X>(xlfActiveCell);
	XOPER<X> xPre = Excel<X>(xlCoerce, xAct);

	XOPER<X> xFor(char1<X>('='));
	append(xFor, pargs->FunctionText());
	append(xFor, char1<X>('('));

	for (unsigned short i = 1; i < pargs->ArgCount(); ++i) {
		DOWN;

		Excel<X>(xlcFormula, pargs->Arg(i).Name());
		XOPER<X> xNamei = Excel<X>(xlfConcatenate, xPre, pargs->Arg(i).Name());

		RIGHT;
		XOPER<X> xActi = Excel<X>(xlfActiveCell);
		XOPER<X> xDef = pargs->Arg(i).Default();
		XOPER<X> xRel = paste_default(xAct, xActi, xDef);
		Excel<X>(xlcDefineName, xNamei, Excel<X>(xlfAbsref, xRel, xAct));
		LEFT;

		if (i > 1) {
			append(xFor, char1<X>(','));
			append(xFor, char1<X>(' '));
		}
		append(xFor, xNamei);
	}
	append(xFor, char1<X>(')'));

	Excel<X>(xlcSelect, xAct);
	RIGHT;
	Excel<X>(xlcFormula, xFor);
	LEFT;
}

void
xll_paste_namex(void)
{
	const Args* parg;
	const Args12* parg12;

	double regid = Excel<XLOPER>(xlCoerce, Excel<XLOPER>(xlfOffset, Excel<XLOPER>(xlfActiveCell), OPER(0), OPER(1))).val.num;

	if (0 != (parg = ArgsMap::Find(regid))) {
		if (parg->isFunction())
			return xll_paste_name(parg);
	}
	else if (0 != (parg12 = ArgsMap12::Find(regid))) {
		if (parg12->isFunction())
			return xll_paste_name(parg12);
	}
	else {
		throw std::runtime_error("XLL.PASTE.FUNCTION: register id not found");
	}
}
/*
static AddIn xai_xll_paste(
	MacroX("_xll_paste@0", "XLL.PASTE.FUNCTION")
	.Category(CATEGORY)
	.FunctionHelp(_T("Paste a function into Excel. "))
	.Documentation(
		"When the active cell is a register id, paste the default arguments below the active cell "
		"and replace the current cell with a call to the function using relative arguments. "
		"When the active cell is a string, look for the register id or a call to the correponding function "
		"in the cell to its right, paste the argument names below the active cell, and the default values "
		"to their right and name them using the active cell contents as a prefix. "
		"Replace the register id/function call with a function call using the named ranges. "
	)
);
extern "C" int __declspec(dllexport) WINAPI
xll_paste(void)
{
	int result(0);

	try {
		Excel<XLOPER>(xlcEcho, OPER(false));

		OPER oAct = Excel<XLOPER>(xlCoerce, Excel<XLOPER>(xlfActiveCell));
		
		if (oAct.xltype == xltypeNum)
			xll_paste_regidx();
		else if (oAct.xltype == xltypeStr)
			xll_paste_namex();
		else
			throw std::runtime_error("XLL.PASTE.FUNCTION: Active cell must be a number or a string. ");

		Excel<XLOPER>(xlcEcho, OPER(true));

	}
	catch (const std::exception& ex) {
		Excel<XLOPER>(xlcEcho, OPER(true));
		XLL_ERROR(ex.what());

		return 0;
	}

	return result;
}

int
xll_paste_close(void)
{
	try {
		if (Excel<XLOPER>(xlfGetBar, OPER(7), OPER(4), OPER("Paste Function")))
			Excel<XLOPER>(xlfDeleteCommand, OPER(7), OPER(4), OPER("Paste Function"));
	}
	catch (const std::exception& ex) {
		XLL_INFO(ex.what());
		
		return 0;
	}

	return 1;
}
static Auto<Close> xac_paste(xll_paste_close);

int
xll_paste_open(void)
{
 	try {
		// Try to add just above first menu separator.
		OPER oPos;
		oPos = Excel<XLOPER>(xlfGetBar, OPER(7), OPER(4), OPER("-"));
		oPos = 5;

		OPER oAdj = Excel<XLOPER>(xlfGetBar, OPER(7), OPER(4), OPER("Paste Function"));
		if (oAdj == Err(xlerrNA)) {
			OPER oAdj(1, 5);
			oAdj(0, 0) = "Paste Function";
			oAdj(0, 1) = "XLL.PASTE.FUNCTION";
			oAdj(0, 3) = "Paste function under cursor into spreadsheet.";
			Excel<XLOPER>(xlfAddCommand, OPER(7), OPER(4), oAdj, oPos);
		}
	}
	catch (const std::exception& ex) {
		XLL_ERROR(ex.what());

		return 0;
	}

	return 1;
}
static Auto<Open> xao_paste(xll_paste_open);
*/

inline void
Move(short r, short c)
{
	ExcelX(xlcSelect, ExcelX(xlfOffset, ExcelX(xlfActiveCell), OPERX(r), OPERX(c)));
}
/*
static AddInX xai_paste_basic(
	MacroX(_T("_xll_paste_basic@0"), _T("XLL.PASTE.BASIC"))
	.Category(_T("Utility"))
	.FunctionHelp(_T("Paste a function with default arguments. Shortcut Ctrl-Shift-B."))
	.Documentation(_T("Does not define names. "))
);
*/

#ifdef _M_X64
static AddIn xai_paste_basic("xll_paste_basic", "XLL.PASTE.BASIC");
#else
static AddIn xai_paste_basic("_xll_paste_basic@0", "XLL.PASTE.BASIC");
#endif
extern "C" int __declspec(dllexport) WINAPI
xll_paste_basic(void)
{
	ExcelX(xlfEcho, OPERX(false));

	try {
		xll_paste_regidx();
	}
	catch (const std::exception& ex) {
		XLL_ERROR(ex.what());

		return 0;
	}
	ExcelX(xlfEcho, OPERX(true));

	return 1;
}
// Ctrl-Shift-B
static On<Key> xok_paste_basic(_T("^+B"), _T("XLL.PASTE.BASIC"));

// create named ranges for arguments
/*
static AddInX xai_paste_create(
	MacroX(_T("_xll_paste_create@0"), _T("XLL.PASTE.CREATE"))
	.Category(_T("Utility"))
	.FunctionHelp(_T("Paste a function and create named ranges for arguments. Shortcut Ctrl-Shift-C"))
	.Documentation(_T("Uses current cell as a prefix. "))
);
*/
#ifdef _M_X64
static AddIn xai_paste_create("xll_paste_create", "XLL.PASTE.CREATE");
#else
static AddIn xai_paste_create("_xll_paste_create@0", "XLL.PASTE.CREATE");
#endif
extern "C" int __declspec(dllexport) WINAPI
xll_paste_create(void)
{
	ExcelX(xlfEcho, OPERX(false));

	try {
		OPERX xAct = ExcelX(xlfActiveCell);
		OPERX xPre = ExcelX(xlCoerce, xAct);

		// use cell to right if in cell containing handle
		if (xPre.xltype == xltypeNum) {
			Move(0, -1);
			xAct = ExcelX(xlfActiveCell);
			xPre = ExcelX(xlCoerce, xAct);
		}

		if (xPre.xltype == xltypeStr) {
			ExcelX(xlcAlignment, OPERX(4)); // align right

			xPre = ExcelX(xlfConcatenate, xPre, OPERX(_T(".")));
		}
		else {
			xPre = _T("");
		}

		OPERX xFor = ExcelX(xlfGetCell, OPERX(6), ExcelX(xlfOffset, xAct, OPERX(0), OPERX(1))); // formula
		ensure (xFor.xltype == xltypeStr);
		ensure (xFor.val.str[1] == '=');

		// extract "=Function"
		OPERX xFind = ExcelX(xlfFind, OPERX(_T("(")), xFor);
		if (xFind.xltype == xltypeNum)
			xFor = ExcelX(xlfLeft, xFor, OPERX(xFind - 1));

		// get regid
		xFor = ExcelX(xlfEvaluate, xFor);
		ensure (xFor.xltype == xltypeNum);
		double regid = xFor.val.num;

		const ArgsX* pargs = ArgsMapX::Find(regid);

		if (!pargs) {
			XLL_WARNING("XLL.PASTE.CREATE: could not find register id of function");

			return 0;
		}

		xFor = ExcelX(xlfConcatenate, OPERX(_T("=")), pargs->FunctionText(), OPERX(_T("(")));

		for (unsigned short i = 1; i < pargs->ArgCount(); ++i) {
			Move(1, 0);

			ExcelX(xlcFormula, pargs->Arg(i).Name());
			ExcelX(xlcAlignment, OPERX(4)); // align right
			OPERX xNamei = ExcelX(xlfConcatenate, xPre, pargs->Arg(i).Name());

			Move(0, 1);
			ExcelX(xlcDefineName, xNamei);
			
			// paste default argument
			OPERX xDef = pargs->Arg(i).Default();
			if (xDef.xltype == xltypeStr && xDef.val.str[1] == '=') {
				OPERX xEval = ExcelX(xlfEvaluate, xDef);
				if (xEval.size() > 1) {
					OPERX xFor = ExcelX(xlfConcatenate, 
						OPERX(_T("=RANGE.SET(")), 
						OPERX(xDef.val.str + 2, xDef.val.str[0] - 1), 
						OPERX(_T(")")));
					ExcelX(xlcFormula, xFor);
					xNamei = ExcelX(xlfConcatenate, OPERX(_T("RANGE.GET(")), xNamei, OPERX(_T(")")));
				}
				else {
					ExcelX(xlcFormula, xDef);
				}
			}
			else {
				ExcelX(xlcFormula, xDef);
			}
			// style
			if (pargs->Arg(i).Name().val.str[1] == '_')
				ApplyStyle(STYLE_OPTIONAL);
			else 
				ApplyStyle(STYLE_OUTPUT);

			if (i > 1) {
				xFor = ExcelX(xlfConcatenate, xFor, OPERX(_T(", ")));
			}
			xFor = ExcelX(xlfConcatenate, xFor, xNamei);

			Move(0, -1);
		}
		xFor = ExcelX(xlfConcatenate, xFor, OPERX(_T(")")));

		ExcelX(xlcSelect, xAct);
		Move(0, 1);
		ExcelX(xlcFormula, xFor);
		Move(0, -1);

		ExcelX(xlcSelect, ExcelX(xlfOffset, xAct, OPER(0), OPER(0), OPER(pargs->ArgCount() + 1), OPER(2))); // select range for RDB.DEFINE
	}
	catch (const std::exception& ex) {
		XLL_ERROR(ex.what());

		return 0;
	}

	ExcelX(xlfEcho, OPERX(true));

	return 1;
}
// Ctrl-Shift-C
static On<Key> xok_paste_create(_T("^+C"), _T("XLL.PASTE.CREATE"));
