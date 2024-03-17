// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#ifndef CAMERA_SYSTEM
#define CAMERA_SYSTEM

#include "ISystem.hpp"
#include "Components/TransformComponent.hpp"
#include "Components/ViewComponent.hpp"
#include "ShaderProgram.hpp"
#include "Globals.hpp"
#include "VertexMath.hpp"

namespace GLVM::ecs
{
    class CCameraSystem : public ISystem
    {
    public:
        Shader* Shader_Program_;
        Matrix<float, 4> tProjection_Matrix{1.0f};

        ///< Mouse parameters.
        float fYaw = -90.0f;
        float fPitch = 0.0f;
        float fLast_X = 1920.0f / 2.0f;
        float fLast_Y = 1080.0f / 2.0f;
        bool bFirst_Mouse = true;

        void Update() override;
        void SetViewMatrix(components::transform& _Player, components::beholder& _view_Component);
        void SetProjectionMatrix();
    };
}

#endif
