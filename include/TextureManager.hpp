// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#ifndef TEXTURE_MANAGER
#define TEXTURE_MANAGER

#include <vector>
#include "Texture.hpp"
#include <mutex>
#include "Components/MaterialComponent.hpp"
#include "GLPointer.h"
#include "Constants.hpp"

typedef unsigned int Entity_ID;
typedef unsigned int Texture_ID;

namespace GLVM::ecs
{
    class TextureManager
    {
        static TextureManager* pInstance_;
        static std::mutex  Mutex_;

        std::vector<Texture> textureVector_;
    public:
        TextureManager();
		
        void SetTextureVector(std::vector<Texture> _textureVector);
        static TextureManager* GetInstance(); ///< It possibly to get only one instance of this class whith this method.
        static TextureManager* GetHUDInstance();
        void BindTexture(Entity_ID _entityID, Texture_ID _textureID);
		void LoadTextureData(GLVM::ecs::Texture& _Texture);
        std::vector<Texture>& GetTextureVector();
        void UnbindTexture(components::material _textureComponent, Entity _entity);
    };
}

#endif
