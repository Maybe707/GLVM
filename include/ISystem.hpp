// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#ifndef ISYSTEM
#define ISYSTEM

#include "ComponentManager.hpp"
#include "Event.hpp"

namespace GLVM::ecs
{
	class ISystem
	{
	public:
		virtual ~ISystem() {}
		virtual void Update() = 0;
	};
}

#endif
