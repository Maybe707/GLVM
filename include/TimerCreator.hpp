// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#ifndef TIMER_CREATOR
#define TIMER_CREATOR

#include "IChrono.hpp"

/*! \class TimerCreator
    \brief Create timer interface

	This class creates a timer independent interface.
	Implemented by means of the factory method.
*/

namespace GLVM::Time
{
	class CTimerCreator
	{
	public:
        ~CTimerCreator() {}
        
		IChrono* Create();
	};
}

#endif
