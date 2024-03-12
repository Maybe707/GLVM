// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#include "WinApi/ChronoWin.hpp"

namespace GLVM::Time
{    
	CTimerWin::CTimerWin()
	{
		InitFrequency();
		Reset();
	}
 
	double CTimerWin::InitFrequency()
	{
		QueryPerformanceFrequency((PLARGE_INTEGER) &i64Freq_);
		return (double)i64Freq_;
	}
 
	double CTimerWin::Reset()
	{
		QueryPerformanceCounter((PLARGE_INTEGER) &i64Start_);
		return (double)i64Start_;
	}
 
	double CTimerWin::GetElapsed()
	{
		QueryPerformanceCounter((PLARGE_INTEGER) &i64Now_);
		return (double) (i64Now_ - i64Start_)/i64Freq_;
	}
}
