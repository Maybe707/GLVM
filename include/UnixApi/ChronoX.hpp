// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#ifndef CHRONOX
#define CHRONOX

#include <ctime>
#include "IChrono.hpp"

namespace GLVM::Time
{
	class CTimerX : public IChrono
	{
		timespec start_;
		timespec now_;
		double lFrequency_;
		double lSeconds_;
		double lNanoseconds_;

	public:
		CTimerX();
	
		double InitFrequency();
		double Reset();
		double GetElapsed();
	};
}

#endif
