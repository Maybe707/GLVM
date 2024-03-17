// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#ifndef CHRONOWIN
#define CHRONOWIN

#include <windows.h>
#include "IChrono.hpp"

namespace GLVM::Time
{    
	class CTimerWin : public IChrono
	{
		__int64 i64Freq_;
		__int64 i64Start_;
		__int64 i64Now_;
	
	public:
		CTimerWin();
	
		double InitFrequency();
		double Reset();
		double GetElapsed();
	};
}
	
#endif

