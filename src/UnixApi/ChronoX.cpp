// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#include "UnixApi/ChronoX.hpp"
#include <ctime>

namespace GLVM::Time
{
	CTimerX::CTimerX()
	{
		InitFrequency();
		Reset();
	}

	double CTimerX::InitFrequency()
	{
		return lFrequency_ = 1e+9;
	}

	double CTimerX::Reset()
	{
		clock_gettime(CLOCK_MONOTONIC, &start_);
		return (start_.tv_sec + start_.tv_nsec);
	}

	double CTimerX::GetElapsed()
	{
		clock_gettime(CLOCK_MONOTONIC, &now_);
		lSeconds_ = now_.tv_sec - start_.tv_sec;
		lNanoseconds_ = now_.tv_nsec - start_.tv_nsec;
		return lSeconds_ + lNanoseconds_/lFrequency_;
	}
}
