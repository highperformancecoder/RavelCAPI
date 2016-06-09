// enum.cpp - evaluate enumerated values
#include "addin.h"
#include "error.h"

using namespace xll;

static AddIn xai_xll_enum(
	XLL_DECORATE("xll_xll_enum", 8), XLL_LPOPER XLL_LPOPER XLL_LPOPER,
	"ENUM", "Name, Item",
	"Utility", "Return an enumeration given its Name and Item."
);
extern "C" LPOPER __declspec(dllexport) WINAPI
xll_xll_enum(LPOPER pName, LPOPER pItem)
{
	static OPER o;
	
	try {
		if (pItem->xltype == xltypeMissing) {
			if (pName->xltype == xltypeMissing)
				o = Enum::Value();
			else
				o = Enum::Value(*pName);
		}
		else {
			o = Enum::Value(*pName, *pItem);
		}
	}
	catch (const std::exception& ex) {
		XLL_ERROR(ex.what());

		o = Err(xlerrNA);
	}

	return &o;
}
