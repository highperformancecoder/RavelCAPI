// loper.h - XLOPER with memory handled by Excel
// Copyright (c) 2011 KALX, LLC. All rights reserved. No warranty is made.
#include "traits.h"

// LOPER o = Excel(xlfSomething, NumX(x), StrX(s), ...);
template <class X>
struct LXOPER : public X {
	LXOPER()
		: owner_(false)
	{
		xltype = xltypeNil;
	}
	LXOPER(const LXOPER& x_)
		: owner_(x_.owner_)
	{
		assign(rfree(x_));
	}
	// allows for explicit ownership
	LXOPER(const X& x, bool own = false)
		: owner_(own)
	{
		assign(x);
	}
	LXOPER& operator=(const LXOPER& x_)
	{
		if (this != &x_) {
			lfree();
			owner_ = x_.owner_;
			assign(rfree(x_));
		}

		return *this;
	}
	LXOPER& operator=(const X& x)
	{
		lfree();
		owner_ = false; // x not an LXOPER
		assign(x);

		return *this;
	}
	~LXOPER()
	{
		lfree();
		owner_ = false;
	}
	operator double() const
	{
		return xll::to_number<X>(*this);
	}
	const X& operator[](typename xll::traits<X>::xword i) const
	{
		return xll::index<X>(*this, i);
	}
	const X& operator()(typename xll::traits<X>::xword i, typename xll::traits<X>::xword j) const
	{
		return xll::index<X>(*this, i, j);
	}
	X* operator&()
	{
		return (X*)this;
	}
	const X* operator&() const
	{
		return (const X*)this;
	}
	X* XLFree(void)
	{
		if (owner_)
			xltype |= xlbitXLFree;

		owner_ = false;

		return this;
	}
	X* xlfree(void)
	{
		return XLFree();
	}
	bool owner(void) const
	{
		return owner_;
	}
private:
	void assign(const X& x)
	{
		xltype = x.xltype;
		val = x.val;
	}
	// l-value really gets freed
	void lfree(void)
	{
		if (owner_) {
			xll::traits<X>::Excel(xlFree, 0, 1, this);
		}
	}
	// r-value gets disowned
	const LXOPER& rfree(const LXOPER& x_) const
	{
		if (x_.owner_) {
			x_.owner_ = false;
		}

		return x_;
	}
	mutable bool owner_;
};

typedef LXOPER<XLOPER>   LOPER, *LPLOPER;
typedef LXOPER<XLOPER12> LOPER12, *LPLOPER12;
typedef X_(LOPER) LOPERX, *LPLOPERX;
