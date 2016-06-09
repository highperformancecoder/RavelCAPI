// strings.h - coversion routines for strings
#pragma once
#include <cstdlib>
#include <string>

namespace strings {

	inline std::string
	wcstombs(const std::basic_string<wchar_t>& xs)
	{
		std::string s;

		s.resize(xs.size() + 1);
		::wcstombs_s(0, &s[0], s.size(), &xs[0], xs.size());

		return s;
	}
	inline std::string
	wcstombs(const wchar_t* xs, std::size_t n = 0)
	{
		std::string s;

		if (!n)
			n = wcslen(xs);

		s.resize(n + 1);
		::wcstombs_s(0, &s[0], s.size(), &xs[0], n);

		return s;
	}
	inline std::basic_string<wchar_t>
	mbstowcs(const std::string& s)
	{
		std::basic_string<wchar_t> xs;

		xs.resize(s.size() + 1);
		::mbstowcs_s(0, &xs[0], xs.size(), &s[0], s.size()); 

		return xs;
	}

	#ifdef EXCEL12
	// ensure string is char
	inline std::basic_string<char> MBS(const std::basic_string<wchar_t>& x)
	{ 
		return strings::wcstombs(x);
	}
	// ensure string is wchar_t
	inline std::basic_string<wchar_t> WCS(const std::basic_string<wchar_t>& x)
	{
		return x;
	}
	#define TO_MBS(o) strings::wcstombs(o.val.str + 1, o.val.str[0])
	#else
	// ensure string is char
	inline std::basic_string<char> MBS(const std::basic_string<char>& x)
	{
		return x;
	}
	// ensure string is wchar_t
	inline std::basic_string<wchar_t> WCS (const std::basic_string<char>& x)
	{ 
		return strings::mbstowcs(x);
	}
	#define T0_MBS(o) std::string(o.val.str + 1, o.val.str[0])
	#endif

} // strings