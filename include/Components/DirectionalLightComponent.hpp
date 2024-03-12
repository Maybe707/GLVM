// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#ifndef DIRECTIONAL_LIGHT_COMPONENT
#define DIRECTIONAL_LIGHT_COMPONENT

#include "VertexMath.hpp"

namespace GLVM::ecs::components
{
	struct directionalLight
	{
		vec3 position;
		vec3 direction;
		
		vec3 ambient;
		vec3 diffuse;
		vec3 specular;
	};
}

#endif
