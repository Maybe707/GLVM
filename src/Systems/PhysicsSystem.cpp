// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#include "Systems/PhysicsSystem.hpp"
#include "ComponentManager.hpp"
#include "Components/ColliderComponent.hpp"
#include "Components/RigidBodyComponent.hpp"
#include "Components/MoveComponent.hpp"
#include "Components/TransformComponent.hpp"
#include "Components/ViewComponent.hpp"
#include "EntityManager.hpp"
#include "Event.hpp"
#include "Globals.hpp"
#include "VertexMath.hpp"

namespace GLVM::ecs
{
    /*! This update searching for refering to colliders entities and check their
     *  transform components for collision, and if collision detected check if
     *  backtracking entity had gravity component for call Gravity function.
     */
         
    void CPhysicsSystem::Update() 
    {
		namespace cm = GLVM::ecs::components;
		
        ComponentManager* componentManager = ComponentManager::GetInstance();
		core::vector<Entity> linkedEntities = componentManager->collectLinkedEntities<cm::collider,
																					  cm::move,
																					  cm::transform>();

		float deltaTime = 5.5f * fDelta_Time_;
		unsigned int linkedEntitiesVectorSize = linkedEntities.GetSize();
        for(unsigned int i = 0; i < linkedEntitiesVectorSize; ++i)
        {
                unsigned int entityRefMove = linkedEntities[i];
				cm::transform* transformComponent = componentManager->GetComponent<cm::transform>(entityRefMove);
				cm::move* move = componentManager->GetComponent<cm::move>(entityRefMove);
				cm::collider* collider = componentManager->GetComponent<cm::collider>(entityRefMove);
                if(collider->bGround_Collision_) {
					move->gravity = 0;
					transformComponent->GravityAccumulator = 0.0f;
                }
                if(collider->bWall_Collision_) {
					move->frameMovement = 0;
                    collider->bWall_Collision_ = false;
                }
				transformComponent->tPosition += move->frameMovement;
				transformComponent->tPosition += move->gravity;
				move->gravity       = 0.0f;
				move->frameMovement = 0.0f;
				componentManager->RemoveComponent<cm::move>(entityRefMove);

				cm::rigidBody* rigidBody = componentManager->GetComponent<cm::rigidBody>(entityRefMove);
				if ( rigidBody->jumpAccumulator > 0.0f ) {
					rigidBody->jumpAccumulator -= deltaTime;
					rigidBody->jump = vec3{ 0.0f, 5.0f, 0.0f } * deltaTime;
					transformComponent->tPosition += rigidBody->jump;
				}
        }
    }
}

