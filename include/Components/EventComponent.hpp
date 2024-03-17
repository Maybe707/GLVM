// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#ifndef EVENT_COMPONENT
#define EVENT_COMPONENT

#include "../Event.hpp"

namespace GLVM::ecs::components
{
	struct event
	{
		core::EEvents eEvent_;
	};
}

#endif
