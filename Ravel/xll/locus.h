// locus.h - location in a spreadsheet
// Copyright (c) 2011 KALX, LLC. All rights reserved. No warranty is made.
#pragma once
#include <cctype>
#include <map>
#include "oper.h"

namespace xll {

	inline void
	text2name(traits<XLOPERX>::xchar* t)
	{
		for (int i = 1; i <= t[0]; ++i)
			if (!isalnum(t[i]))
				t[i] = _T('_');
	}
	inline void
	interchange(traits<XLOPERX>::xchar* t, traits<XLOPERX>::xchar r, traits<XLOPERX>::xchar c)
	{
		for (int i = 1; i < t[0]; ++i) {
			if (t[i] == r)
				t[i] = c;
			else if (t[i] == c)
				t[i] = r;
		}
	}

	// associate OPER with a spreadsheet location
	// When first called, set cell name to cell location.
	// If cell is moved, Excel will update to new cell location.
	// Use get def to find original cell.
	// Use that to find location value.
	class locus {
		OPERX ref_, val_;
	public:
		// locus l; // in uncalced function
		locus()
		{
			OPERX xCaller = ExcelX(xlfCaller);
			OPERX xReftext = ExcelX(xlfReftext, xCaller);

			ref_ = ExcelX(xlfGetDef, xReftext);
			if (ref_.xltype == xltypeErr) {
				ref_ = xReftext;
				text2name(ref_.val.str);
				Name(ref_, xCaller); // not xReftext!
				//!!! if deleted undefine old name

			}
			// name used for value
			val_ = ref_;
			interchange(val_.val.str, _T('R'), _T('C'));

			OPERX val = ExcelX(xlfGetName, val_);
			OPERX def = ExcelX(xlfGetDef, val);
			if (val.xltype == xltypeErr)
				Name(val_, OPERX());
		}
		~locus()
		{ }
		OPERX get()
		{
			return ExcelX(xlfEvaluate, ExcelX(xlfGetName, val_));
		}
		void set(const XLOPERX& v)
		{
			Name(val_, v);
		}
		locus& operator=(const XLOPERX& v)
		{
			Name(v);

			return *this;
		}
		static OPERX Names()
		{
			OPERX o;

			for (std::map<OPERX,OPERX>::const_iterator i = names().begin(); i != names().end(); ++i)
				o.push_back(ExcelX(xlfConcatenate, i->first, OPERX(_T(":")), OPERX(to_string<XLOPERX>(i->second).c_str())));

			return o;
		}
	private:
		static std::map<OPERX, OPERX>& names()
		{
			static std::map<OPERX, OPERX> names_;

			return names_;
		}
		void Name(const OPERX& name, const OPERX& value)
		{
			ExcelX(xlfSetName, name, value);
			names()[name] = value;
		}
		void Name(const OPERX& name)
		{
			ExcelX(xlfSetName, name);
			names().erase(name);
		}
	};

} // namespace xll