// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#ifndef ITIMER
#define ITIMER

namespace GLVM::Time
{
	class IChrono
	{
	public:
	    virtual ~IChrono() {}
		
		virtual double InitFrequency() = 0;
		virtual double Reset() = 0;
		virtual double GetElapsed() = 0;
	};
}

#endif
