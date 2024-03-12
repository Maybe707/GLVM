// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#ifndef TRANSFORM_COMPONENT
#define TRANSFORM_COMPONENT

#include "VertexMath.hpp"

namespace GLVM::ecs::components
{    
	struct transform
	{
        vec3 tPosition{ 0.0f, 0.0f, 0.0f };
        vec3 tForward{ 0.0f, 0.0f, 0.0f };
        vec3 tRight{ 0.0f, 0.0f, 0.0f };
        vec3 tUp{ 0.0f, 0.0f, 0.0 };
        float yaw = 0.0f;
		float pitch = 0.0f;
        float fScale = 1.0f;
        bool hud = false;
		float GravityAccumulator = 0.0f;
		unsigned int currentAnimationFrame = 0;
		float frameAccumulator = 0.0f;
		bool gltf = true;
	};
}

#endif
