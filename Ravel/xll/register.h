// register.h - Register a macro or function.
// Copyright (c) 2011 KALX, LLC. All rights reserved. No warranty is made.
#pragma once
#include <map>
#include <string>
#include "enum.h"

namespace xll {

	template<class X>

	struct XArgsMap {
		typedef std::map<double, const XArgs<X>* > args_map;

		static args_map& Map()
		{
			static args_map m_;

			return m_;
		}

		// Find by register id.
		static const XArgs<X>* Find(double regid)
		{
			args_map& m(Map());

			args_map::const_iterator i = m.find(regid);

			return i == m.end() ? 0 : i->second;
		}

		// Find by name.
		static const XArgs<X>* Find(const X& name)
		{
			args_map& m(Map());

			args_map::const_iterator i;
			
			for (i = m.begin(); i != m.end(); ++i) 
				if (i->second->FunctionText() == name)
					return i->second;

			return 0;
		}

		static void Insert(double regid, const XArgs<X>* pa)
		{
			Map().insert(std::make_pair(regid, pa));
		}
	};
	typedef XArgsMap<XLOPER>   ArgsMap;
	typedef XArgsMap<XLOPER12> ArgsMap12;
	typedef X_(ArgsMap)        ArgsMapX;

	// macro
	template<class X>
	inline OPERX Register(
		typename xll::traits<X>::xcstr procedure, 
		typename xll::traits<X>::xcstr function_text)
	{
		return Register<X>(XArgs<X>(procedure, function_text));
	}

	// function
	template<class X>
	inline XOPER<X> Register(
		typename xll::traits<X>::xcstr procedure, 
		typename xll::traits<X>::xcstr type_text, 
		typename xll::traits<X>::xcstr function_text, 
		typename xll::traits<X>::xcstr argument_text)
	{
		return Register<X>(XArgs<X>(procedure, type_text, function_text, argument_text));
	}

	// general case
	template<class X>
	inline XOPER<X> Register(const XArgs<X>& arg, size_t i = 0)
	{
		XOPER<X> x(true);

		if (arg.isMacro() || arg.isFunction()) {
			XArgs<X> a = arg;
			if (i > 0) {
				a.FunctionText(arg.Alias()[i - 1]);
			}

			int result = traits<X>::Excelv(xlfRegister, &x, a.size(), a.pointers());
			ensure (xll::type(x) != xltypeStr && xll::type(x) != xltypeMulti); 

			if (result != xlretSuccess || xll::type(x) == xltypeErr) {
				Excel<X>(xlcAlert
					, Excel<X>(xlfConcatenate
						, arg.Procedure()
						, XOPER<X>(_T(": failed to register.\nDid you forget #pragma XLLEXPORT?"))
					)
				);
			}
			else {
				// point at the real arg
				ensure (xll::type(x) == xltypeNum);
				XArgsMap<X>::Insert(x.val.num, &arg);
			}

			if (i == 0 && arg.Alias().size() > 0) {
				for (size_t i = 1; i <= arg.Alias().size(); ++i)
					Register(arg, i);
			}

		}

		return x;
	}

	template<class X>
	inline XOPER<X> Unregister(const XArgs<X>& arg)
	{
		XOPER<X> x;

		try {
			x = Excel<X>(xlfUnregister, Excel<X>(xlfRegisterId, Excel<X>(xlGetName), arg.Procedure()));

			// Workaround for name not disappearing from paste function dialog.
			Register<X>(XArgs<X>(arg).MacroType(0)); // hidden
		}
		catch (const std::exception&) {
			Excel<X>(xlcAlert
				,Excel<X>(xlfConcatenate
					,arg.Procedure()
					,XOPER<X>(": failed to unregister.\n")
				)
			);
		}

		return x;
	}

	template<class X>
	class XRequire {
	};

	typedef XRequire<XLOPER>   Require;
	typedef XRequire<XLOPER12> Require12;
	typedef XRequire<XLOPERX>  RequireX;
} // namespace xll