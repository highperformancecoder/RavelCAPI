// limits.h - Excel limits parameterized by XLOPER and XLOPER12.
// Copyright (c) 2006 KALX, LLC. All rights reserved. No warranty is made.
#pragma once
#include "defines.h"

namespace xll {

	template<class X> struct limits { };

	template<>
	struct limits<XLOPER> {
		// The total number of available columns in Excel
		static const BYTE maxcols = 0xFF;
		// The total number of available rows in Excel
		static const WORD maxrows = 0xFFFF;
		// The number of characters that can be stored and displayed in a cell formatted as Text
		static const UCHAR maxchars = 0xFF;
		//Maximum number of arguments to a function
		static const int maxargs = 30;
	};

	template<>
	struct limits<XLOPER12> {
		// The total number of available columns.
		static const COL maxcols = 0x3FFF;
		// The total number of available rows.
		static const RW maxrows = 0xFFFFF;
		// The number of characters that can be stored in a cell formatted as Text
		static const XCHAR maxchars = 0x7FFF;
		//Maximum number of arguments to a function
		static const int maxargs = 255;
	};
}
