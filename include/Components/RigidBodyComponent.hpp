// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#ifndef RIGIDBODY_COMPONENT
#define RIGIDBODY_COMPONENT

#include "VertexMath.hpp"

namespace GLVM::ecs::components
{
	class rigidBody
	{
    public:
		float gravityTime;
        float fMass_;
        bool bGravity_;
		vec3 jump{ 0.0f, 0.0f, 0.0f };
		float jumpAccumulator = 0.0f;
	};
}

#endif
