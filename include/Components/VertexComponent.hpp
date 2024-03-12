// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#ifndef VERTEX_COMPONENT
#define VERTEX_COMPONENT

// #include <vector>
// #include <iostream>
// #include "GraphicAPI/Vulkan.hpp"

#include <cstdint>

namespace GLVM::ecs::components
{
	struct MeshHandle {
		uint32_t id;
	};
	
	struct mesh
	{
        MeshHandle handle;
	};
}

#endif
