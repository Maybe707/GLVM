// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#ifndef TEXTURE
#define TEXTURE

#include <cstdint>
#include <vector>
#include <GL/gl.h>

typedef unsigned int Entity;

namespace GLVM::ecs
{
	struct TextureHandle {
		uint32_t id;
	};
	
    struct Texture
    {
        unsigned int vkAvailableInnerId_ = 0; ///< This field using to choose specific instance of texture image in Vulkan.
        unsigned int vkInnerIdLimit_ = 10;
        
        GLuint iTexture_;
		const char* path_to_image;
        std::vector<Entity> entitiesOwnsThisTypeOfTexture_;
        unsigned int id_;
		unsigned int iWidth_;
		unsigned int iHeight_;
        unsigned int dat_length_;
		unsigned char* u_iData_;
    };
}

#endif
