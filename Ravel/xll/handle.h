// handle.h - simple handle class
// Copyright (c) 2006 KALX, LLC. All rights reserved. No warranty is made.
// Use SET.NAME instead of list???
#pragma once
#include <cctype>
#include <cfloat>
#include <cstdint>
#include <set>
#include <memory>
#include "oper.h"

//#pragma comment(linker, "/section:.handleseg,RWS")

typedef double HANDLEX;

static_assert (sizeof(HANDLEX) >= sizeof(void*), "handles must be large enough to hold pointers");

namespace xll {

	// handle to pointer
	template<class T>
	inline T* h2p(HANDLEX x)
	{
		union { T* p_; HANDLEX x_; int64_t l_; } px;
		
		px.l_ = static_cast<long long>(x);

		return px.p_;
	}
	// pointer to handle
	template<class T>
	inline HANDLEX p2h(T* p)
	{
		union { T* p_; HANDLEX x_; long long l_; } px;
		
		px.l_ = 0;
		px.p_ = p;

		// 64-bit pointers might map to denormalized doubles
		int fpclass = _fpclass(static_cast<HANDLEX>(px.l_));
		ensure (fpclass == _FPCLASS_NN || fpclass == _FPCLASS_PN);

		return static_cast<HANDLEX>(px.l_);
	}

	// similar to std::unique_ptr
	template<class T, class D = std::default_delete<T>>
	class handle {
	public:
		typedef typename std::set<std::unique_ptr<T,D>> handle_set;
//#pragma bss_seg(".handleseg")
		static handle_set& handle_set_get(void)
		{
			static handle_set h_;

			return h_;
		}
//#pragma bss_seg()
	private:
		// get previous cell contents
		// caller must be uncalced
		static T* previous()
		{
			T* q(0);

			OPERX x = Excel<XLOPERX>(xlCoerce, Excel<XLOPERX>(xlfCaller));
			if (xll::type(x) == xltypeNum) {
				// otherwise it is junk
				q = h2p<T>(static_cast<HANDLEX>(x.val.num));
			}

			return q;
		}

		// return 0 if p not in handle set
		static typename handle_set::iterator find(T* p)
		{
			handle_set& h(handle_set_get());

			return std::find_if(h.begin(), h.end(), [p](const std::unique_ptr<T,D>& l) { return l.get() == p; });
		}

		static void insert(T* p, D d)
		{
			handle_set& h(handle_set_get());
			T* q = previous();
			if (q) {
				handle_set::iterator i = find(q);
				if (i != h.end()) {
					h.erase(i);
				}
			}
			// do after checking old handle
			// might have same handle in old spreadsheet
			h.insert(std::unique_ptr<T,D>(p,d));
		}

		T* p_;
		handle(const handle&);
		handle& operator=(const handle&);
	public:
		// Called as handle<T> h(new T(...));
		// in constructor. Must be uncalced.
		handle(T* p, D d = std::default_delete<T>())
			: p_(p)
		{
			if (p) {
				insert(p, d);
			}
		}
		// Called as handle<T> h(handle) in "member" function.
		// Need not be uncalced.
		handle(HANDLEX p, bool checked = true)
		{
			p_ = h2p<T>(p);
			if (checked)
				p_ = find(p_) == handle_set_get().end() ? 0 : p_;
		}
		~handle()
		{ }

		bool operator!() const
		{
			return !p_;
		}
		// handle returned to Excel
		HANDLEX get() const
		{
			return p_ ? p2h<T>(p_) : std::numeric_limits<HANDLEX>::quiet_NaN();
		}
		T* ptr()
		{
			return p_;
		}
		const T* ptr() const
		{
			p_;
		}
		operator T*()
		{
			return p_;
		}
		T* operator->()
		{
			return p_;
		}
		const T* operator->() const
		{
			return p_;
		}
		T& operator*()
		{
			return *p_;
		}
		const T& operator*() const
		{
			return *p_;
		}

		static size_t erase(T* p)
		{
			return handle_set_get().erase(p);
		}
	};

	// NaN returns as #NUM!
	class handlex {
		HANDLEX h_;
	public:
		handlex(HANDLEX h = std::numeric_limits<double>::quiet_NaN())
			: h_(h)
		{ }
		~handlex()
		{ }
		operator const HANDLEX&() const
		{
			return h_;
		}
	};

} // namespace xll

