// registry.h - Windows registry wrappers
// Copyright (c) 2011 KALX, LLC. All rights reserved. No warranty is made.
#pragma once
#include <cassert>
#include <string>
#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#include "ensure.h"

#pragma warning(disable: 4100)
namespace Reg {

	template<class T> 
	struct traits {
		static const DWORD type = REG_BINARY;
		static BYTE* data(T& t) { return (BYTE*)&t; }
		static const BYTE* data(const T& t) { return (const BYTE*)&t; }
		static DWORD size(const T& t) { return static_cast<DWORD>(sizeof(t)); }
	};

	template<>
	struct traits<DWORD> {
		static const DWORD type = REG_DWORD;
		static BYTE* data(DWORD& t) { return (BYTE*)&t; }
		static const BYTE* data(const DWORD& t) { return (const BYTE*)&t; }
		static DWORD size(DWORD t) { return static_cast<DWORD>(sizeof(t)); }
	};
	// 64-bit type
	template<>
	struct traits<double> {
		static const DWORD type = REG_QWORD;
		static BYTE* data(double& t) { return (BYTE*)&t; }
		static const BYTE* data(const double& t) { return (const BYTE*)&t; }
		static DWORD size(double t) { return static_cast<DWORD>(sizeof(t)); }
	};
	template<> 
	struct traits<int> {
		static const DWORD type = REG_DWORD;
		static BYTE* data(int& t) { return (BYTE*)&t; }
		static const BYTE* data(const int& t) { return (const BYTE*)&t; }
		static DWORD size(int t) { return static_cast<DWORD>(sizeof(t)); }
	};
	template<> 
	struct traits<TCHAR*> {
		static const DWORD type = REG_SZ;
		static BYTE* data(TCHAR* t) { return (BYTE*)t; }
		static const BYTE* data(const TCHAR* t) { return (const BYTE*)t; }
		static DWORD size(TCHAR* t) { return static_cast<DWORD>(sizeof(TCHAR)*(1 + _tcslen(t))); }
	};
	template<> 
	struct traits<std::basic_string<TCHAR> > {
		static const DWORD type = REG_SZ;
		static BYTE* data(std::basic_string<TCHAR>& t) { return (BYTE*)&t[0]; }
		static const BYTE* data(const std::basic_string<TCHAR>& t) { return (const BYTE*)&t[0]; }
		static DWORD size(const std::basic_string<TCHAR>& t) { return static_cast<DWORD>(sizeof(TCHAR)*(1 + t.length())); }
	};

	template<class X>
	inline LSTATUS RegQueryValue_(HKEY, const X*, X*, PLONG);
	template<>
	inline LSTATUS RegQueryValue_<char>(HKEY h, const char* k, char* pd, PLONG pnd)
	{
		return RegQueryValueA(h, k, pd, pnd);
	}
	template<>
	inline LSTATUS RegQueryValue_<wchar_t>(HKEY h, const wchar_t* k, wchar_t* pd, PLONG pnd)
	{
		return RegQueryValueW(h, k, pd, pnd);
	}

	template<class X>
	inline LSTATUS RegQueryValueEx_(HKEY, const X*, LPDWORD, LPDWORD, LPBYTE, LPDWORD);
	template<>
	inline LSTATUS RegQueryValueEx_<char>(HKEY h, const char* k, LPDWORD r, LPDWORD pt, LPBYTE pb, LPDWORD pnb)
	{
		return RegQueryValueExA(h, k, r, pt, pb, pnb);
	}
	template<>
	inline LSTATUS RegQueryValueEx_<wchar_t>(HKEY h, const wchar_t* k, LPDWORD r, LPDWORD pt, LPBYTE pb, LPDWORD pnb)
	{
		return RegQueryValueExW(h, k, r, pt, pb, pnb);
	}

	template<class X>
	inline LSTATUS RegSetValueEx_(HKEY, const X*, DWORD, DWORD, CONST BYTE*, DWORD);
	template<>
	inline LSTATUS RegSetValueEx_<char>(HKEY h, const char* k, DWORD r, DWORD t, CONST BYTE* pb, DWORD nb)
	{
		return RegSetValueExA(h, k, r, t, pb, nb);
	}
	template<>
	inline LSTATUS RegSetValueEx_<wchar_t>(HKEY h, const wchar_t* k, DWORD r, DWORD t, CONST BYTE* pb, DWORD nb)
	{
		return RegSetValueExW(h, k, r, t, pb, nb);
	}

	template<class X>
	inline LSTATUS RegOpenKeyEx_(HKEY, const X*, DWORD, REGSAM, PHKEY);
	template<>
	inline LSTATUS RegOpenKeyEx_<char>(HKEY h, const char* k, DWORD r, REGSAM rs, PHKEY ph)
	{
		return RegOpenKeyExA(h, k, r, rs, ph);
	}
	template<>
	inline LSTATUS RegOpenKeyEx_<wchar_t>(HKEY h, const wchar_t* k, DWORD r, REGSAM rs, PHKEY ph)
	{
		return RegOpenKeyExW(h, k, r, rs, ph);
	}

	template<class X>
	inline LSTATUS RegCreateKeyEx_(HKEY, const X*, DWORD, X*, DWORD, REGSAM, CONST LPSECURITY_ATTRIBUTES, PHKEY, LPDWORD);
	template<>
	inline LSTATUS RegCreateKeyEx_<char>(HKEY h, const char* k, DWORD r, char* c, DWORD o, REGSAM d, CONST LPSECURITY_ATTRIBUTES psa, PHKEY pk, LPDWORD pd)
	{
		return RegCreateKeyExA(h, k, r, c, o, d, psa, pk, pd);
	}
	template<>
	inline LSTATUS RegCreateKeyEx_<wchar_t>(HKEY h, const wchar_t* k, DWORD r, wchar_t* c, DWORD o, REGSAM d, CONST LPSECURITY_ATTRIBUTES psa, PHKEY pk, LPDWORD pd)
	{
		return RegCreateKeyExW(h, k, r, c, o, d, psa, pk, pd);
	}

	// parameterize by character type
	template<class X>
	class Key {
	protected:
		HKEY h_;
		Key(const Key&);
		Key& operator=(const Key&);
	public:
		explicit Key(HKEY h) : h_(h) { }
		~Key()
		{ }
		operator HKEY()
		{
			return h_;
		}
		HKEY* operator&()
		{
			return &h_;
		}

		// Type of registy entry value.
		DWORD Type(const X* name)
		{
			DWORD type;

			ensure (ERROR_SUCCESS == RegQueryValueEx_<X>(h_, name, 0, &type, 0, 0));

			return type;
		}

		// T must be POD for this to work
		template<class T>
		T QueryValue(const X* name)
		{
			T t;
			DWORD type, size(traits<T>::size(T()));

			ensure (ERROR_SUCCESS == RegQueryValueEx_<X>(h_, name, 0, &type, traits<T>::data(t), &size));
			ensure (traits<T>::type == type);

			return t;
		}

		//
		// Specializations
		//
		template<>
		std::basic_string<X> QueryValue(const X* name)
		{
			DWORD type, size;
			std::basic_string<X> sz;

			ensure (ERROR_SUCCESS == RegQueryValueEx_<X>(h_, name, 0, &type, 0, &size));
			ensure (REG_SZ == type || REG_EXPAND_SZ == type);

			sz.resize(size/sizeof(X));
			ensure (ERROR_SUCCESS == RegQueryValueEx_<X>(h_, name, 0, &type, reinterpret_cast<LPBYTE>(&sz[0]), &size));
			sz.resize(_tcslen(&sz[0]));

			return sz;
		}

		template<class T>
		LONG SetValue(const X* name, const T& t)
		{
			LONG result;

			result = RegSetValueEx_<X>(h_, name, 0, traits<T>::type, traits<T>::data(t), traits<T>::size(t));

			return result;
		}
		LONG SetValue(const X* name, const X* value)
		{
			LONG result;

			result = RegSetValueEx_<X>(h_, name, 0, REG_SZ, reinterpret_cast<const BYTE*>(value), static_cast<DWORD>(_tcslen(value)*sizeof(X)));

			return result;
		}
	};

	template<class X>
	class OpenKey : public Key<X> {
		OpenKey(const OpenKey&);
		OpenKey& operator=(const OpenKey&);
	public:
		OpenKey(HKEY h, const X* k = 0, REGSAM r = KEY_READ )
			: Key(h)
		{
			ensure (ERROR_SUCCESS == RegOpenKeyEx_<X>(h, k, 0, r, &h_));			
		}
		~OpenKey()
		{
			RegCloseKey(h_);
		}
	};

	template<class X>
	class CreateKey : public Key<X> {
		CreateKey(const CreateKey&);
		CreateKey& operator=(const CreateKey&);
	public:
		CreateKey(HKEY h, const X* k, REGSAM r = KEY_ALL_ACCESS)
			: Key(h)
		{
			ensure (ERROR_SUCCESS == RegCreateKeyEx_<X>(h, k, 0, NULL, REG_OPTION_VOLATILE, r, NULL, &h_, 0));
		}
		~CreateKey()
		{
			RegCloseKey(h_);
		}
	};

	// registry object of type T
	template<class X, class T>
	class Object {
		CreateKey<X> h_;
		std::basic_string<X> name_;
		T t_;
		Object(const Object&);
		Object& operator=(const Object&);
	public:
		Object(HKEY h, const X* k, const X* name, const T& init = 0)
			: h_(h, k, KEY_ALL_ACCESS), name_(name)
		{
			try {
				t_ = h_.QueryValue<T>(name);
			}
			catch (const std::exception&) {
				t_ = init;
				h_.SetValue(name, t_);
			}
		}
		~Object()
		{ }

		operator T() 
		{
			return t_;
		}
		T* operator&()
		{
			return &t_;
		}

		bool operator==(const X* str) const
		{
			return 0 == _tcscmp(t_.c_str(),  str);
		}

		friend class ProxyObject;

		class ProxyObject {
			Object& o_;
		public:
			ProxyObject(Object& o)
				: o_(o)
			{ }
			ProxyObject& operator=(const ProxyObject& o)
			{
				if (this != &o)
					o_ = o;

				return *this;
			}
			~ProxyObject()
			{
				o_.h_.SetValue(o_.name_.c_str(), o_.t_);
			}
			operator T&()
			{
				return o_.t_;
			}
		};

		// forward when used as l-value so ~ProxyObject flushes to registry
		ProxyObject operator=(const T& t)
		{
			t_ = t;

			return ProxyObject(*this);
		}
		ProxyObject operator++()
		{
			++t_;

			return ProxyObject(*this);
		}
		ProxyObject operator++(int)
		{
			t_++;

			return ProxyObject(*this);
		}
		ProxyObject operator--()
		{
			--t_;

			return ProxyObject(*this);
		}
		ProxyObject operator--(int)
		{
			t_--;

			return ProxyObject(*this);
		}
		ProxyObject operator&=(int t)
		{
			t_ &= t;

			return ProxyObject(*this);
		}
		ProxyObject operator|=(int t)
		{
			t_ |= t;

			return ProxyObject(*this);
		}
		// should add the complete list - welcome to proxy objects!
	};

} // namespace Reg

