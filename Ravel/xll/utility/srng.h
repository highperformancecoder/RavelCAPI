// srng.h - simple random number generator
// Copyright (c) 2011 KALX, LLC. All rights reserved. No warranty is made.
#pragma once
#ifdef WIN32
#include <ctime>
#include "registry.h"
#define SRNG_SUBKEY _T("Software\\KALX\\xll")
#endif

static_assert(sizeof(unsigned int) == 4, "unsigned int must be 32 bits");

namespace utility {

	class srng { 
		unsigned int s_[2];
	public:
		static const unsigned int min = 0;
		static const unsigned int max = 0xFFFFFFFF;

#ifdef WIN32
		// store state in registry
		void save(void) const
		{
			Reg::CreateKey<TCHAR>(HKEY_CURRENT_USER, SRNG_SUBKEY).SetValue<DWORD>(_T("seed0"), s_[0]);
			Reg::CreateKey<TCHAR>(HKEY_CURRENT_USER, SRNG_SUBKEY).SetValue<DWORD>(_T("seed1"), s_[1]);
		}
		void load(void)
		{
			Reg::Object<TCHAR, DWORD> s0(HKEY_CURRENT_USER, SRNG_SUBKEY, _T("seed0"), 521288629);
			Reg::Object<TCHAR, DWORD> s1(HKEY_CURRENT_USER, SRNG_SUBKEY, _T("seed1"), 362436069);
			s_[0] = s0;
			s_[1] = s1;
		}
#else
		void save(void) const 
		{ }
		void load(void)
		{
			s_[0] = 521288629;
			s_[1] = 362436069;
		}
#endif // WIN32

		srng(bool stored = false)
		{
			if (stored) {
				load();
			}
			else {
				s_[0] = static_cast<unsigned int>(::time(0));
				s_[1] = ~s_[0];

				save();
			}
		}
		void seed(unsigned int s0, unsigned int s1)
		{ 
			s_[0] = s0;
			s_[1] = s1;

			save();
		}
		void seed(unsigned int s[2])
		{
			s_[0] = s[0];
			s_[1] = s[1];

			save();
		}
		const unsigned int* seed(void) const
		{
			return s_;
		}

		// uniform unsigned int in the range [0, 2^32)
		// See http://www.bobwheeler.com/statistics/Password/MarsagliaPost.txt
		unsigned int uint()
		{
			s_[1] = 36969 * (s_[1] & 0xFFFF) + (s_[1] >> 0x10);
			s_[0] = 18000 * (s_[0] & 0xFFFF) + (s_[0] >> 0x10);

			return (s_[1] << 0x10) + s_[0];
		}

		// uniform double in the range [0, 1)
		double real()
		{
			// 0 <= u < 2^32
			double u = uint();

			return u/max;
		}
		// uniform int in [a, b]
		int between(int a, int b)
		{
			return static_cast<int>(a + ((b - a + 1)*real()));
		}
		// uniform double in [a, b)
		double uniform(double a = 0, double b = 1)
		{
			return a + (b - a)*real();
		}


	};
} // namespace utility
