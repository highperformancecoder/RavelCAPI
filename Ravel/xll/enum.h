// enum.h - Enumerated values in Excel
// Copyright (c) 2013 KALX, LLC. All rights reserved. No warranty is made.
/*
	static Enum xll_name_enum(
		Name("Name")
		.Item("String", STRING_VALUE, "is a description of STRING_VALUE")
		.Item(...)
		.Category("Enum Category")
		.Description("This is the enum description.")
	);

	static AddIn ...
		.Arg(XLL_LPOPER, "Enum", "Is a Name enumeration") // !!!Create XLL_ENUM type
	...
	xll_function(..., LPOPER penum, ...)
	{
		...
		int val = Enum::Value("Name", *penum);
		...


	=ENUM("Name", "String") -> ENUM_VALUE
	=ENUM("Name") -> all enumerated names and their values

*/
#pragma once
#include <cctype>
#include "args.h"

#pragma comment(linker, "/include:" XLL_DECORATE("xll_xll_enum", 8))

namespace xll {

	// "Camel CaseString" -> "ccs" or original if all lower
	inline traits<XLOPERX>::xstring canonical_string(const traits<XLOPERX>::xstring& s)
	{
		traits<XLOPERX>::xstring t;

		for (traits<XLOPERX>::xcstr ps = s.c_str(); *ps; ++ps)
			if (!::isspace(*ps) && (::isupper(*ps) || ::isdigit(*ps)))
				t.push_back(static_cast<traits<XLOPERX>::xchar>(::tolower(*ps)));

		if (t.length() == 0)
			t = s;

		return t;
	}
	inline OPERX& tolower(OPERX& x)
	{
		if (x.xltype == xltypeStr)
			for (traits<XLOPERX>::xchar i = 1; i <= x.val.str[0]; ++i)
				x.val.str[i] = static_cast<traits<XLOPERX>::xchar>(::tolower(x.val.str[i]));

		return x;
	}

	// map cannonical name to items in enum
	template<class X>
	class XName {
		typedef typename xll::traits<X>::xcstr xcstr;
		typedef typename xll::traits<X>::xstring xstring;
		typedef typename std::map<xstring, XOPER<X>> map;
		map	map_;
		XOPER<X> name_, cat_, desc_;
	public:
		typedef typename map::const_iterator map_citer;
		XName(xcstr name = 0, xcstr cat = 0, xcstr desc = 0)
			: name_(name), cat_(cat), desc_(desc)
		{
			tolower(name_);
		}
		XName(const XName& n)
			:  map_(n.map_), name_(n.name_), cat_(n.cat_), desc_(n.desc_)
		{ }
		XName& operator=(const XName& n)
		{
			if (this != &n) {
				map_ = n.map_;
				name_ = n.name_;
				cat_ = n.cat_;
				desc_ = n.desc_;
			}

			return *this;
		}
		~XName()
		{ }

		const OPERX& operator[](xstring key) const
		{
			xstring ckey = canonical_string(key);
			auto i = map_.find(ckey);
			ensure (i != map_.end());

			return i->second;
		}
		map_citer cbegin() const
		{
			return map_.cbegin();
		}
		map_citer cend() const
		{
			return map_.cend();
		}

		XName& Name(xcstr name)
		{
			
			name_ = name;
			tolower(name_);

			return *this;
		}
		const XOPER<X>& Name(void) const
		{
			return name_;
		}
		XName& Category(xcstr cat)
		{
			cat_ = cat;

			return *this;
		}
		const XOPER<X>& Category(void) const
		{
			return cat_;
		}
		XName& Description(xcstr desc)
		{
			desc_ = desc;

			return *this;
		}
		const XOPER<X>& Description(void) const
		{
			return desc_;
		}
		template<class T>
		XName& Item(xcstr key, const T& value, xcstr desc = 0)
		{
			XOPER<X> item(1, 3);

			item[0] = key;
			item[1] = value;
			item[2] = desc;

			xstring cs = canonical_string(key);
			ensure (map_.find(cs) == map_.end());
			map_[cs].push_back(item);

			return *this;
		}
	};

	typedef XName<XLOPER>   Name;
	typedef XName<XLOPER12> Name12;
	typedef XName<XLOPERX>  NameX;

	template<class X>
	class XEnum {
		typedef typename xll::traits<X>::xcstr xcstr;
		typedef typename xll::traits<X>::xword xword;
		typedef typename xll::traits<X>::xstring xstring;
	public:
		typedef typename std::map<XOPER<X>,XName<X>> enum_map;
		typedef typename enum_map::const_iterator enum_citer;
		static enum_map& map(void) {
			static enum_map enum_map_;

			return enum_map_;
		}
		XEnum(const XName<X>& ea)
		{
			map()[ea.Name()] = ea; // possible to have multiple Enum constructors???
		}
		static XOPER<X>
		Value()
		{
			XOPER<X> o;

			enum_map& em = map();
			for (auto i = em.begin(); i != em.end(); ++i)
				o.push_back(i->first);

			return o;
		}
		static XOPER<X>
		Value(const XOPER<X>& name)
		{
			XOPER<X> o;

			enum_map& em = map();
			XOPER<X> name_ = name;
			enum_citer i = em.find(tolower(name_));
			ensure (i != em.end());

			const XName<X>& n = i->second;
			for (XName<X>::map_citer ni = n.cbegin(); ni != n.cend(); ++ni)
				o.push_back(ni->second);

			return o;
		}
		static XOPER<X>
		Value(const XOPER<X>& name, const XOPER<X>& key)
		{
			if (key.xltype == xltypeNum)
				return XOPER<X>(key.val.num);

			ensure (key.xltype == xltypeStr);

			enum_map& em = map();
			XOPER<X> name_ = name;
			enum_citer i = em.find(tolower(name_));
			ensure (i != em.end());

			const XName<X>& n = i->second;
			const XOPER<X>& kv = n[to_string<X>(key)];

			return kv[1];
		}
		// for C enum type
		static int
		value(const XOPER<X>& name, const XOPER<X>& key)
		{
			const XOPER<X>& o = Value(name, key);

			ensure (o.xltype == xltypeNum);

			return static_cast<int>(o.val.num);
		}
	};

	typedef XEnum<XLOPER>   Enum;
	typedef XEnum<XLOPER12> Enum12;
	typedef XEnum<XLOPERX>  EnumX;

} //namespace xll