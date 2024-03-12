// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#ifndef CONTROLLER_SYSTEM
#define CONTROLLER_SYSTEM

#include "Event.hpp"
#include "Stack.hpp"

namespace GLVM::core
{
	class ControllerSystem
	{
		CStack& inputStack_;
		CEvent event_;
	public:
		ControllerSystem(CStack& inputStack, CEvent& event);
		void Update();
	};
}

#endif
