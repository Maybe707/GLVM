// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#ifndef COLLIDER_COMPONENT
#define COLLIDER_COMPONENT

#include "Vector.hpp"

namespace GLVM::ecs::components
{
	class collider
	{
    public:
        bool bGround_Collision_ = false;
		bool roofCollision = false;
        bool bWall_Collision_ = false;
	};
}

#endif
