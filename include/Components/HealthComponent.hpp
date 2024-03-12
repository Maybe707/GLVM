// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#ifndef HEALTH
#define HEALTH

namespace GAME_MECHANICS::ecs::components
{
	struct health
	{
		float maxHealth; 
        float currentHealth;
	};
}

#endif
