// timer.h - High resolution timer class
// Copyright (c) 2011 KALX, LLC. All rights reserved. No warranty is made.
#pragma once
#include <Windows.h>
#include "ensure.h"

namespace utility {

	class timer {
		LARGE_INTEGER  pc_, freq_;
		double elapsed_;
		bool running_;
	public:
		timer() 
			: elapsed_(0), running_(false)
		{
			ensure (QueryPerformanceFrequency(&freq_));

			ensure (freq_.QuadPart != 0);
		}
		void reset(void)
		{
			elapsed_ = 0;
			running_ = false;
		}
		void start(void)
		{
			running_ = true;
			QueryPerformanceCounter(&pc_);
		}
		void stop(void)
		{
			LARGE_INTEGER  pc;

			QueryPerformanceCounter(&pc);
			elapsed_ += 1.0*(pc.QuadPart - pc_.QuadPart) / freq_.QuadPart;
			running_ = false;
		}
		// time in seconds
		double elapsed(void)
		{
			if (running_) { // get the split
				stop();
				start();
			}

			return elapsed_;
		}
	};

} // namespace utility