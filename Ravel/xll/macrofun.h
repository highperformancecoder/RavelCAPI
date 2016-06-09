// macrofun.h - Helper functions for Excel4 macro functions documented in MACROFUN.HLP.
// Copyright (c) KALX, LLC. All rights reserved. No warranty is made.
#pragma once
#include "oper.h"

namespace xll {
	//
	// Enumerations related to MACROFUN.HLP
	//

	// ALIGNMENT()
	enum HAlign { 
		HA_NONE,
		HA_GENERAL,
		HA_LEFT,
		HA_CENTER,
		HA_RIGHT,
		HA_FILL,
		HA_JUSTIFY,
		HA_CENTER_ACROSS_SELECTION
	};
	enum VAlign {
		VA_NONE,
		VA_TOP,
		VA_CENTER, 
		VA_BOTTOM,
		VA_JUSTIFY
	};
	enum Orientation {
		O_HORIZONTAL,
		O_VERTICAL,
		O_UPWARD,
		O_DOWNWARD,
		O_AUTOMATIC
	};

	// BORDER()
	enum BorderType {
		BT_OUTLINE, 
		BT_LEFT, 
		BT_RIGHT, 
		BT_TOP, 
		BT_BOTTOM
	};
	// Font
	enum Color {
		AUTOMATIC, 
		BLACK = 1,
		BROWN = 53,
		OLIVE_GREEN = 52,
		DARK_GREEN = 51,
		DARK_TEAL = 49,
		DARK_BLUE = 11,
		INDIGO = 55,
		GRAY_80 = 56,
		DARK_RED = 9,
		ORANGE = 46,
		DARK_YELLOW = 12,
		GREEN = 10,
		TEAL = 14,
		BLUE = 5,
		BLUE_GRAY = 47,
		GRAY_50 = 16,
		RED = 3,
		LIGHT_ORANGE = 45,
		LIME = 43,
		SEA_GREEN = 50,
		AQUA = 42,
		LIGHT_BLUE = 41,
		VIOLET = 13,
		GRAY_40 = 48,
		PINK = 7,
		GOLD = 44,
		YELLOW = 6,
		BRIGHT_GREEN = 4,
		TURQUOISE = 8,
		SKY_BLUE = 33,
		PLUM = 54,
		GRAY_25 = 15,
		ROSE = 38,
		TAN = 40,
		LIGHT_YELLOW = 36,
		LIGHT_GREEN = 35,
		LIGHT_TURQUOISE = 34,
		PALE_BLUE = 37,
		LAVENDER = 39,
		WHITE = 2,
	};

	// DEFINE.STYLE()
	enum DefineStyle {
		DS_NUMBER_FORMAT = 2,
		DS_FONT_FORMAT = 3,
		DS_ALIGNMENT = 4,
		DS_BORDER = 5,
		DS_PATTERN = 6,
		DS_CELL_PROTECTION = 7
	};
	// FONT.PROPERTIES() for DS_FONT_FORMAT
	enum FontStyle {
		FS_NONE = 0,
		FS_BOLD = 1, 
		FS_ITALIC = 2, 
		FS_UNDERLINE = 4,
		FS_STRIKE = 8
	};
	enum UnderlineStyle {
		US_NONE,
		US_SINGLE,
		US_DOUBLE,
		US_SINGLE_ACCOUNTING,
		US_DOUBLE_ACCOUNTING
	};

	enum BorderStyle {
		BS_NONE,
		BS_THIN,
		BS_MEDIUM,
		BS_DASHED,
		BS_DOTTED,
		BS_THICK,
		BS_DOUBLE,
		BS_HAIRLINE
	};

	#define FOO_BOOL(a,b) OPERX(((a)&(b))!=0)

	// PATTERNS()
	enum PatternAuto { // either AreaAuto or BorderAuto
		PA_USER, // custom
		PA_AUTOMATIC, // set by Excel
		PA_NONE
	};

	// Common formatting functions
	inline void
	Align(HAlign ha, VAlign va = VA_NONE, bool wrap = false)
	{
		if (va == VA_NONE)
			XLL_XLC(Alignment, OPERX(ha), OPERX(wrap));
		else
			XLL_XLC(Alignment, OPERX(ha), OPERX(wrap), OPERX(va));
	}
	inline void
	Border(BorderType bt, BorderStyle bs, Color c = AUTOMATIC)
	{
		switch (bt) {
		case BT_OUTLINE:
			XLL_XLC(Border, OPERX(bs), MissingX(), MissingX(), MissingX(), MissingX(), MissingX(), OPERX(c));
			break;
		case BT_LEFT:
			XLL_XLC(Border, MissingX(), OPERX(bs), MissingX(), MissingX(), MissingX(),  MissingX(), MissingX(), OPERX(c));
			break;
		case BT_RIGHT:
			XLL_XLC(Border, MissingX(), MissingX(), OPERX(bs), MissingX(), MissingX(),  MissingX(), MissingX(), MissingX(), OPERX(c));
			break;
		case BT_TOP:
			XLL_XLC(Border, MissingX(), MissingX(), MissingX(), OPERX(bs), MissingX(),  MissingX(), MissingX(), MissingX(), MissingX(), OPERX(c));
			break;
		case BT_BOTTOM:
			XLL_XLC(Border, MissingX(), MissingX(), MissingX(), MissingX(), OPERX(bs),  MissingX(), MissingX(), MissingX(), MissingX(), MissingX(), OPERX(c));
			break;
		}
	}
	inline void DefineStyleNumber(LPCTSTR name, LPCTSTR format)
	{
		XLL_XLC(DefineStyle, OPERX(name), OPERX(DS_NUMBER_FORMAT), OPERX(format));
	}
	inline void DefineStyleFont(LPCTSTR name, LPCTSTR font, unsigned int size = 0, FontStyle bius = FS_NONE, Color c = AUTOMATIC)
	{
		OPERX b(FOO_BOOL(bius, FS_BOLD)), i(FOO_BOOL(bius, FS_ITALIC)), u(FOO_BOOL(bius, FS_UNDERLINE)), s(FOO_BOOL(bius, FS_STRIKE));

		XLL_XLC(DefineStyle, OPERX(name), OPERX(DS_FONT_FORMAT), OPERX(font), size ? OPERX(size) : MissingX(), b, i, u, s, OPERX(c));
	}
	inline void DefineStyleBorder(LPCTSTR name, BorderType bt, BorderStyle bs = BS_MEDIUM, Color c = AUTOMATIC)
	{
		switch (bt) {
		case BT_OUTLINE:
			XLL_XLC(DefineStyle, OPERX(name), OPERX(DS_BORDER), OPERX(bs), MissingX(), MissingX(), MissingX(), MissingX(), MissingX(), OPERX(c));
			break;
		case BT_LEFT:
			XLL_XLC(DefineStyle, OPERX(name), OPERX(DS_BORDER), MissingX(), OPERX(bs), MissingX(), MissingX(), MissingX(),  MissingX(), MissingX(), OPERX(c));
			break;
		case BT_RIGHT:
			XLL_XLC(DefineStyle, OPERX(name), OPERX(DS_BORDER), MissingX(), MissingX(), OPERX(bs), MissingX(), MissingX(),  MissingX(), MissingX(), MissingX(), OPERX(c));
			break;
		case BT_TOP:
			XLL_XLC(DefineStyle, OPERX(name), OPERX(DS_BORDER), MissingX(), MissingX(), MissingX(), OPERX(bs), MissingX(),  MissingX(), MissingX(), MissingX(), MissingX(), OPERX(c));
			break;
		case BT_BOTTOM:
			XLL_XLC(DefineStyle, OPERX(name), OPERX(DS_BORDER), MissingX(), MissingX(), MissingX(), MissingX(), OPERX(bs),  MissingX(), MissingX(), MissingX(), MissingX(), MissingX(), OPERX(c));
			break;
		}
	}
	inline void
	DefineStyleAlign(LPCTSTR name, HAlign ha, bool wrap = false, VAlign va = VA_NONE)
	{
		if (va == VA_NONE)
			XLL_XLC(DefineStyle, OPERX(name), OPERX(DS_ALIGNMENT), OPERX(ha), OPERX(wrap));
		else
			XLL_XLC(DefineStyle, OPERX(name), OPERX(DS_ALIGNMENT), OPERX(ha), OPERX(wrap), OPERX(va));
	}
	inline void DefineStyleProtection(LPCTSTR name, bool locked, bool hidden = false)
	{
		XLL_XLC(DefineStyle, OPERX(name), OPERX(DS_CELL_PROTECTION), OPERX(locked), OPERX(hidden));
	}
	

	// put t in active cell
	template<class T>
	inline OPERX
	Formula(const T& t)
	{
		OPERX ref(XLL_XLF(ActiveCell));

		XLL_XLC(Select, ref);
		XLL_XLC(Formula, OPERX(t));

		return ref;
	}
	template<>
	inline OPERX
	Formula<OPERX>(const OPERX& t)
	{
		OPERX ref(XLL_XLF(Offset, XLL_XLF(ActiveCell), OPERX(0), OPERX(0), OPERX(t.rows()), OPERX(t.columns())));

		if (t)
			XLL_XLC(Formula, t);

		return ref;
	}

	inline void
	FormatFont(LPCTSTR font, unsigned int size = 0, FontStyle bius = FS_NONE, Color c = AUTOMATIC)
	{
		OPERX b(FOO_BOOL(bius, FS_BOLD)), i(FOO_BOOL(bius, FS_ITALIC)), u(FOO_BOOL(bius, FS_UNDERLINE)), s(FOO_BOOL(bius, FS_STRIKE));

		XLL_XLC(FormatFont, OPERX(font), size ? OPERX(size) : MissingX(), b, i, u, s, OPERX(c));
	}
	#undef FOO_BOOL



} // namespace xll
