// on.h - xlcOnXXX functions
// Copyright (c) 2010 KALX, LLC. All rights reserved. No warranty is made.
#pragma once
#include "oper.h"

// used with On<Key>
#define ON_SHIFT   _T("+")
#define ON_CTRL    _T("^")
#define ON_ALT     _T("%")
#define ON_COMMAND _T("*")

namespace xll {

	struct Data {
		static const int On = xlcOnData;
	};
	struct Doubleclick {
		static const int On = xlcOnDoubleclick;
	};
	struct Entry {
		static const int On = xlcOnEntry;
	};
	struct Key {
		static const int On = xlcOnKey;
	};
	struct Recalc {
		static const int On = xlcOnRecalc;
	};
	struct Sheet {
		static const int On = xlcOnSheet;
	};
	struct Time {
		static const int On = xlcOnTime;
	};
	struct Window {
		static const int On = xlcOnWindow;
	};

	// On<Key> ok("shortcut", "MACRO");
	template<class K>
	class On {
	public:
		typedef traits<XLOPERX>::xcstr xcstr;
		On(xcstr text, xcstr macro)
		{
			OPERX x(3,1);

			x[0] = K::On;
			x[1] = text;
			x[2] = macro;

			callbacks().push_back(x);
		}
		On(xcstr text, xcstr macro, bool activate)
		{
			OPERX x(4,1);

			ensure (K::On == xlcOnSheet);
			x[0] = K::On;
			x[1] = text;
			x[2] = macro;
			x[3] = activate;

			callbacks().push_back(x);
		}
		On(const OPERX& time, xcstr macro, 
			const OPERX& tolerance = MissingX(), bool insert = true)
		{
			OPERX x(5,1);

			ensure (K::On == xlcOnTime);
			x[0] = K::On;
			x[1] = time;
			x[2] = macro;
			x[3] = tolerance;
			x[4] = insert;

			callbacks().push_back(x);
		}

		static int Open(void)
		{
			try {
				callback_list& l(callbacks());

				for (callback_iter i = l.begin(); i != l.end(); ++i) {
					int f = static_cast<int>((*i)[0].val.num);

					if ((*i).size() == 3)
						Excel<XLOPERX>(f, (*i)[1], (*i)[2]);
					else if ((*i).size() == 4)
						Excel<XLOPERX>(f, (*i)[1], (*i)[2], (*i)[3]);
					else if ((*i).size() == 5)
						Excel<XLOPERX>(f, (*i)[1], (*i)[2], (*i)[3], (*i)[4]);
				}
			}
			catch (const std::exception& ex) {
				XLL_ERROR(ex.what());

				return 0;
			}

			return 1;
		}
		static int Close(void)
		{
			try {
				callback_list& l(callbacks());

				for (callback_iter i = l.begin(); i != l.end(); ++i) {
					if ((*i).size() > 1) {
						int f = static_cast<int>((*i)[0].val.num);
						Excel<XLOPERX>(f, (*i)[1]); // unregister
					}
				}
			}
			catch (const std::exception& ex) {
				XLL_ERROR(ex.what());

				return 0;
			}

			return 1;
		}
	private:
		typedef std::list<OPERX> callback_list;
		typedef std::list<OPERX>::iterator callback_iter;
		static callback_list& callbacks(void)
		{
			static callback_list callbacks_;

			return callbacks_;
		}
	};
} // namespace xll
