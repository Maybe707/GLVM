// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#ifndef GUI_SYSTEM
#define GUI_SYSTEM

#include "ComponentManager.hpp"
#include "Event.hpp"
#include "ISystem.hpp"
#include "ShaderProgram.hpp"
#include "Constants.hpp"
#include "VertexMath.hpp"
#include <GL/gl.h>

namespace GLVM::ecs
{
    class CGUISystem : public ISystem
    {
        GLuint iVbo_Crosshair_;
		GLuint iVao_Crosshair_;

    public:
		CGUISystem();
        void Update() override;
		void RaycastringDebug();

        Shader* _Shader_Program;
		Shader* debugLines;
    };
}

#endif
