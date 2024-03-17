// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#include "Components/ColliderComponent.hpp"
#include "Components/ControllerComponent.hpp"
#include "Components/MaterialComponent.hpp"
#include "Components/PointLightComponent.hpp"
#include "Components/ProjectileComponent.hpp"
#include "Components/TransformComponent.hpp"
#include "Components/VertexComponent.hpp"
#include "Texture.hpp"
#include <Systems/ProjectileSystem.hpp>

namespace GLVM::ecs
{
    CProjectileSystem::CProjectileSystem(core::CStack& inputStack) : inputStack (inputStack)
    {}
    
    void CProjectileSystem::Update()
    {
		namespace cm = GLVM::ecs::components;
		
        ComponentManager* pComponent_Manager = GLVM::ecs::ComponentManager::GetInstance();
        EntityManager* pEntity_Manager       = GLVM::ecs::EntityManager::GetInstance();
    
        core::vector<unsigned int>* pEntity_Container_refMove =
			pComponent_Manager->GetEntityContainer<cm::controller>();
        unsigned int u_iVector_Move_Size = pEntity_Container_refMove->GetSize();

        core::vector<unsigned int>* pEntity_Container_refView =
			pComponent_Manager->GetEntityContainer<cm::beholder>();

		unsigned int iEntity_refView = 0;
		if ( pEntity_Container_refView->GetSize() > 0 )
			iEntity_refView = (*pEntity_Container_refView)[0];
		
        cm::beholder* view_Component = pComponent_Manager->GetComponent<cm::beholder>(iEntity_refView);
        
        float cameraSpeed = 5.5f * deltaFrameTime;            

        if(projectileCooldown > 0)
            projectileCooldown -= cameraSpeed;

        for(unsigned int i = 0; i < u_iVector_Move_Size; ++i) {
            for(int n = 0; n < 6; ++n) {
                unsigned int iEntity_refMove = (*pEntity_Container_refMove)[i];

                if(inputStack.SearchElement(core::EEvents::eMOUSE_LEFT_BUTTON) == core::EEvents::eMOUSE_LEFT_BUTTON) {
                    if(projectileCooldown <= 0) {
                        CalculateProjectile(pComponent_Manager,
                                            iEntity_refMove,
                                            *view_Component);
                        projectileCooldown = 2.0;
                    }
                }
            }
        }

        // core::vector<unsigned int>* pEntity_Container_refProjectile =
		// 	pComponent_Manager->GetEntityContainer<cm::projectile>();
        // unsigned int uiVector_Projectile_Size = pEntity_Container_refProjectile->GetSize();

        ComponentManager* componentManager       = ComponentManager::GetInstance();
		core::vector<Entity> linkedEntities      = componentManager->collectLinkedEntities<cm::projectile,
																						   cm::transform,
																						   cm::material,
																						   cm::mesh,
																						   cm::collider,
																						   cm::pointLight>();
		
        for(unsigned int x = 0; x < linkedEntities.GetSize(); ++x) {
            unsigned int uiEntity_refProjectile = linkedEntities[x];
            cm::transform* rTransformProjectile = pComponent_Manager->GetComponent<cm::transform>(uiEntity_refProjectile);
			rTransformProjectile->tPosition += rTransformProjectile->tForward * cameraSpeed;
			cm::pointLight* pointLightComponent = pComponent_Manager->GetComponent<cm::pointLight>(uiEntity_refProjectile);
			pointLightComponent->position += rTransformProjectile->tForward * cameraSpeed;
		}

        for(unsigned int i = 0; i < linkedEntities.GetSize(); ++i) {
			
            unsigned int uiEntity_refProjectile = linkedEntities[i];
            if(pComponent_Manager->GetComponent<cm::collider>(uiEntity_refProjectile)->bWall_Collision_ ||
               pComponent_Manager->GetComponent<cm::collider>(uiEntity_refProjectile)->bGround_Collision_) {
                pEntity_Manager->RemoveEntity(uiEntity_refProjectile, pComponent_Manager);
            }
//			std::cout << "Size: " << linkedEntities.GetSize() << std::endl;
//			pComponent_Manager->GetEntityContainer<cm::projectile>()->Print();
//			std::cout << "Colliders container size: " << pComponent_Manager->GetEntityContainer<cm::collider>()->GetSize() << std::endl;
//			std::cout << "Projectiles container size 1: " << linkedEntities.GetSize() << std::endl;
//			std::cout << "Projectiles container size 2: " << uiVector_Projectile_Size << std::endl;
//			std::cout << "entity: " << uiEntity_refProjectile << std::endl;
        }
    }

    void CProjectileSystem::CalculateProjectile(ecs::ComponentManager* componentManager,
												unsigned int entityRefMove,
												components::beholder& beholder) {
		namespace cm = GLVM::ecs::components;

        unsigned int uiEntity_Projectile = ecs::EntityManager::GetInstance()->CreateEntity();
        ecs::ComponentManager::GetInstance()->CreateComponent<cm::mesh, cm::collider,
															  cm::transform, cm::material,
															  cm::projectile, cm::pointLight>(uiEntity_Projectile);

        core::Sound::CSoundSample* pSound_Sample = new core::Sound::CSoundSample();
        pSound_Sample->kPath_to_File_ = "../laser2.wav";
        pSound_Sample->uiDuration_ = 5;
        pSound_Sample->uiRate_ = 22050;
        soundEngine->GetSoundContainer().Push(pSound_Sample);

		ecs::components::MeshHandle meshHandle{};
		if ( meshHandlers.GetSize() > 0 )
			meshHandle = meshHandlers[0];
		componentManager->GetComponent<cm::mesh>(uiEntity_Projectile)->handle = meshHandle;
		ecs::TextureHandle textureHandle{};
		if ( textureHandlers.GetSize() > 0 )
			textureHandle = textureHandlers[0];
		cm::material* rTextureProjectile = componentManager->GetComponent<cm::material>(uiEntity_Projectile);
		*rTextureProjectile = { .diffuseTextureID_ = textureHandle, .specularTextureID_ = textureHandle, .ambient = { 0.05f, 0.05f, 0.05f },
		.shininess = 128.0f * 0.078125f };
        cm::transform* rTransformProjectile = componentManager->GetComponent<cm::transform>(uiEntity_Projectile);
        rTransformProjectile->fScale = 0.1f;
		
		cm::transform* transform = componentManager->GetComponent<cm::transform>(entityRefMove);
		if ( transform != nullptr )
			rTransformProjectile->tPosition = transform->tPosition;

        rTransformProjectile->tForward   = GetDirectionVector(beholder);
		rTransformProjectile->yaw        = fYaw;
		rTransformProjectile->pitch      = fPitch;
        rTransformProjectile->tPosition += rTransformProjectile->tForward * 2.0;
		
		*(componentManager->GetComponent<cm::pointLight>(uiEntity_Projectile)) = { .position = rTransformProjectile->tPosition,
			.ambient = { 0.1f, 0.1f, 0.1f }, .diffuse = { 0.5f, 0.5f, 0.5f }, .specular = { 1.1f, 1.2f, 1.3f },
			.constant = 1.4f, .linear = 0.1f, .quadratic = 0.128f };
    }

    Vector<float, 3> CProjectileSystem::GetDirectionVector(components::beholder& beholder)
    {
        const float kSensitivity = 0.1f;

        fYaw = g_eEvent.mousePointerPosition.offset_X;
        fPitch = g_eEvent.mousePointerPosition.offset_Y;
        fYaw *= kSensitivity;
        fPitch *= kSensitivity;

        g_eEvent.mousePointerPosition.pitch = fPitch;
        g_eEvent.mousePointerPosition.yaw = fYaw;
        
        if(fPitch > 89.0f)
            fPitch = 89.0f;
        if(fPitch < -89.0f)
            fPitch = -89.0f;

		vec3 forward;
		float sinPitch = std::sin(Radians(fPitch / 2));
		float cosPitch = std::cos(Radians(fPitch / 2));
		float sinYaw = std::sin(Radians(-fYaw / 2));
		float cosYaw = std::cos(Radians(-fYaw / 2));
		
		Quaternion pitchQuat;
		Quaternion yawQuat;
		pitchQuat.w = cosPitch;
		pitchQuat.x = sinPitch;
		pitchQuat.y = 0.0f;
		pitchQuat.z = 0.0f;

		yawQuat.w = cosYaw;
		yawQuat.x = 0.0f;
		yawQuat.y = sinYaw;
		yawQuat.z = 0.0f;

		Quaternion result;
		result = multiplyQuaternion(yawQuat, pitchQuat);

		result = multiplyQuaternion(multiplyQuaternion(result, Quaternion{ .w = 0.0f, .x = 0.0f,
					.y = 0.0f, .z = -1.0f }), inverseQuaternion(result));

		forward[0] = result.x;
		forward[1] = result.y;
		forward[2] = result.z;
		
        // front[0] = std::cos(Radians(fYaw)) * std::cos(Radians(fPitch));
        // front[1] = std::sin(Radians(fPitch));
        // front[2] = std::sin(Radians(fYaw)) * std::cos(Radians(fPitch));
        beholder.forward = Normalize(forward);

        return beholder.forward;
    }
}
