// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#include "Systems/MovementSystem.hpp"
#include "ComponentManager.hpp"
#include "Components/ColliderComponent.hpp"
#include "Components/ControllerComponent.hpp"
#include "Components/DirectionalLightComponent.hpp"
#include "Components/EventComponent.hpp"
#include "Components/MoveComponent.hpp"
#include "Components/MaterialComponent.hpp"
#include "Components/PointLightComponent.hpp"
#include "Components/RigidBodyComponent.hpp"
#include "Components/SpotLightComponent.hpp"
#include "Components/TransformComponent.hpp"
#include "Components/VertexComponent.hpp"
#include "Components/ProjectileComponent.hpp"
#include "Components/ViewComponent.hpp"
#include "Engine.hpp"
#include "EntityManager.hpp"
#include "Event.hpp"
#include "ISoundEngine.hpp"
#include "Vector.hpp"
#include "VertexMath.hpp"
#include <cstdio>

namespace GLVM::ecs
{
    CMovementSystem::CMovementSystem(core::CStack& inputStack) :
        inputStack(inputStack) {}
        
    void CMovementSystem::Update()
    {
		namespace cm = GLVM::ecs::components;
		
        ComponentManager* componentManager = GLVM::ecs::ComponentManager::GetInstance();
		core::vector<Entity> linkedEntities = componentManager->collectLinkedEntities<cm::controller,
																						cm::beholder,
																						cm::transform>();
		unsigned int linkedEntitiesVectorSize = linkedEntities.GetSize();
        float cameraSpeed = 5.5f * deltaFrameTime;            

        for(unsigned int i = 0; i < linkedEntitiesVectorSize; ++i) {
			// std::cout << "i: " << i << std::endl;
			// std::cout << "size: " << linkedEntitiesVectorSize << std::endl;
			Entity currentEntity                = linkedEntities[i];
			cm::beholder* beholderComponent     = componentManager->GetComponent<cm::beholder>(currentEntity);
//			cm::transform* transformComponent   = componentManager->GetComponent<cm::transform>(currentEntity);
//			vec3 result = { 0.0f, 0.0f, 0.0f };
			
            for(int n = 0; n < 6; ++n) {
				vec3 right;
				vec3 forward;
                switch(inputStack[n])
                {
                case core::EEvents::eMOVE_LEFT:
					right = CalculateVectorRL(*beholderComponent);
					componentManager->CreateComponent<cm::move>(currentEntity);
					componentManager->GetComponent<cm::move>(currentEntity)->frameMovement -=
						right * cameraSpeed;
//					result -= right * cameraSpeed;
                    break;
                case core::EEvents::eMOVE_RIGHT:
					right = CalculateVectorRL(*beholderComponent);
					componentManager->CreateComponent<cm::move>(currentEntity);
					componentManager->GetComponent<cm::move>(currentEntity)->frameMovement +=
						right * cameraSpeed;
//					result += right * cameraSpeed;
                    break;
                case core::EEvents::eMOVE_BACKWARD:
                    forward = CalculateVectorFB(*beholderComponent, g_eEvent);
					componentManager->CreateComponent<cm::move>(currentEntity);
					componentManager->GetComponent<cm::move>(currentEntity)->frameMovement -=
						forward * cameraSpeed;
//					result -= forward * cameraSpeed;
                    break;
                case core::EEvents::eMOVE_FORWARD:
					forward = CalculateVectorFB(*beholderComponent, g_eEvent);
					componentManager->CreateComponent<cm::move>(currentEntity);
					componentManager->GetComponent<cm::move>(currentEntity)->frameMovement +=
						forward * cameraSpeed;
//					result += forward * cameraSpeed;
                    break;
                case core::EEvents::eJUMP:
				{
					cm::collider* collider = componentManager->GetComponent<cm::collider>(currentEntity);
					if ( collider->bGround_Collision_ ) {
						cm::rigidBody* rigidBody = componentManager->GetComponent<cm::rigidBody>(currentEntity);
						rigidBody->jumpAccumulator = 1.5f;
					}
				}
                    break;
                default:
                    break;
                }
			  	// FIXME: FLASH LIGHT CRUTCH
				
				// core::vector<unsigned int>* pEntityContainerRefSpotLight = ecs::GetEntityContainer<ecs::spotLight>(*pComponent_Manager);
				// unsigned int spotLightComponentContainerSize = pEntityContainerRefSpotLight->GetSize();
				// for(int x = 0; x < spotLightComponentContainerSize; ++x) {
				// 	unsigned int uiSpotLightEntity = (*pEntityContainerRefSpotLight)[x];
				// 	ecs::spotLight& spotLightComponent = pComponent_Manager->GetComponent<ecs::spotLight>(uiSpotLightEntity);
				// 	spotLightComponent.direction = rTransformComponent.tForward;
				// 	spotLightComponent.position  = rTransformComponent.tPosition;
				// }
            }
        }
		// FIXME: NO NEED TO HAVE SPECIAL FIELD FOR GRAVITY FRAME MOVEMENT
        for(unsigned int n = 0; n < componentManager->GetEntityContainer<cm::rigidBody>()->GetSize(); ++n) {
//            int iEntity_refRigidBody = (*ecs::GetEntityContainer<ecs::rigidBody>(*pComponent_Manager))[n];
			int iEntity_refRigidBody = (*componentManager->GetEntityContainer<cm::rigidBody>())[n];
//            ecs::transform& rTransform_Component = pComponent_Manager->GetComponent<ecs::transform>(iEntity_refRigidBody);
			cm::transform* rTransform_Component = componentManager->GetComponent<cm::transform>(iEntity_refRigidBody);
			cm::rigidBody* rigidBodyComponennt = componentManager->GetComponent<cm::rigidBody>(iEntity_refRigidBody);
			componentManager->CreateComponent<cm::move>(iEntity_refRigidBody);
			cm::move* moveComponent = componentManager->GetComponent<cm::move>(iEntity_refRigidBody);
			rTransform_Component->GravityAccumulator += deltaFrameTime;
			float gravity = 9.8f * rTransform_Component->GravityAccumulator
				* rigidBodyComponennt->fMass_ * 0.0005;
			if ( gravity > 0.2f )
				gravity = 0.2;

			moveComponent->gravity[1] -= gravity;
        }
    }

    Vector<float, 3> CMovementSystem::CalculateVectorRL(components::beholder& beholder) {
//		std::cout << beholder.up << std::endl;
        Vector<float, 3> normalizedVector = Normalize(Cross(beholder.forward, beholder.up));
        return normalizedVector;
    }

    Vector<float, 3> CMovementSystem::CalculateVectorFB(components::beholder& beholder,
                                                        core::CEvent& event) {
        Vector<float, 3> forward(0.0f);
        // forward[0] = std::cos(Radians(event.mousePointerPosition.yaw * 2));
        // forward[2] = std::sin(Radians(event.mousePointerPosition.yaw * 2));

		float sinYaw = std::sin(Radians(-event.mousePointerPosition.yaw / 2));
		float cosYaw = std::cos(Radians(-event.mousePointerPosition.yaw / 2));
		
		Quaternion yawQuat;
		yawQuat.w = cosYaw;
		yawQuat.x = 0.0f;
		yawQuat.y = sinYaw;
		yawQuat.z = 0.0f;

		Quaternion result;
		result = multiplyQuaternion(multiplyQuaternion(yawQuat, Quaternion{ .w = 0.0f, .x = 0.0f,
					.y = 0.0f, .z = -1.0f }), inverseQuaternion(yawQuat));

		forward[0] = result.x;
		forward[1] = result.y;
		forward[2] = result.z;
		
        beholder.forward = Normalize(forward);
//		std::cout << beholder.forward << std::endl;
        return beholder.forward;
    }
}


