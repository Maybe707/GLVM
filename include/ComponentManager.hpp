// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#ifndef COMPONENT_MANAGER
#define COMPONENT_MANAGER

#include "Components/AnimationMoveComponent.hpp"
#include "Components/AttackComponent.hpp"
#include "Components/ColliderComponent.hpp"
#include "Components/DirectionalLightComponent.hpp"
#include "Components/EventComponent.hpp"
#include "Components/MaterialComponent.hpp"
#include "Components/MoveComponent.hpp"
#include "Components/PointLightComponent.hpp"
#include "Components/ProjectileComponent.hpp"
#include "Components/SpotLightComponent.hpp"
#include "Components/TransformComponent.hpp"
#include "Components/VertexComponent.hpp"
#include "Components/ViewComponent.hpp"
#include "Vector.hpp"
#include <cassert>
#include <compare>
#include <concepts>
#include <iostream>
#include "IContainer.hpp"
//#include "Components/VertexComponent.hpp"
#include <mutex>
#include <assert.h>
#include "Components/ControllerComponent.hpp"
#include <cstdlib>

typedef unsigned int Entity;

namespace GLVM::ecs
{
	class ComponentManager
	{
        static ComponentManager* pInstance_;
        static std::mutex  Mutex_;
		unsigned int numberOfBaseComponents;
		
        ComponentManager();
        ~ComponentManager();

		template <typename componentType>
		unsigned int CreateComponentContainer() {
				static unsigned int localContainerID = 0;
				static bool existComponentContainerFlag = false;
				if(existComponentContainerFlag)  
					return localContainerID;
                                                                                                                
				localContainerID = componentsContainerID;    ///< Give a value of global component container ID's counter to local container ID of current component type.
				existComponentContainerFlag = true;
            
				core::vector<componentType>* componentContainer =
					new core::vector<componentType>;    ///< Create component container of current type.
				worldComponentsContainer.Push(componentContainer);
            
				core::vector<Entity>* sparseEntitiesMapToComponents =
					new core::vector<Entity>;    ///< Create ID's component container.
				worldSparseEntitiesMapToComponents.Push(sparseEntitiesMapToComponents);

				core::vector<Entity>* denseEntitiesMapToComponents =
					new core::vector<Entity>;    ///< Create ID's component container.
				worldDenseComponentsMapToEntities.Push(denseEntitiesMapToComponents);
				componentsTypes.Push(typeid(componentType).name());
//				std::cout << typeid(Component_Type).name() << std::endl;			
				++componentsContainerID;
				return localContainerID;
			}
        
	public:
		inline static unsigned int componentsContainerID = 0;
		core::vector<core::IContainer*> worldComponentsContainer;    ///< Contains all local containers for diferent types of components.
		core::vector<core::vector<Entity>*> worldSparseEntitiesMapToComponents;    ///< Contains all local container with IDs for diferent types of components.
		core::vector<core::vector<Entity>*> worldDenseComponentsMapToEntities;

		core::vector<const char*> componentsTypes;
		
        ComponentManager(ComponentManager& componentManager) = delete;         ///< Dont need to make cope because of singleton property.
        void operator=(const ComponentManager& componentManager) = delete;      ///< Dont need assignment operator because of singleton property.
       static ComponentManager* GetInstance();                          ///< It possibly to get only one instance of this class whith this method.

		template <typename componentType>
		void CreateComponent(const Entity& entity)
		{
			unsigned int localContainerID = 0; ///< Index for world components and world ID's containers.
			componentType Component;
			localContainerID = CreateComponentContainer<componentType>();

			core::vector<Entity>& sparse = *static_cast<core::vector<Entity>*>
				(worldSparseEntitiesMapToComponents[localContainerID]);
			core::vector<Entity>& dense = *static_cast<core::vector<Entity>*>
				(worldDenseComponentsMapToEntities[localContainerID]);
			core::vector<componentType>& components = *static_cast<core::vector<componentType>*>
				(worldComponentsContainer[localContainerID]);
//			std::cout << typeid(componentType).name() << std::endl;
			if ( checkAvailability( sparse, dense, entity ) ) {
				return;
			}
			
			if ( entity >= sparse.GetSize() ) {
				sparse.Resize(entity + 1);
			}

			assert( dense.GetSize() == components.GetSize() );
			
			sparse[entity] = dense.GetSize();
			dense.Push(entity);
			components.Push(Component);
		}

		bool checkAvailability( core::vector<Entity>& sparse,
								core::vector<Entity>& dense,
								Entity entity );
		
        /// Allow to give a various components to chosen entity.
        
        template <typename componentType1, typename componentType2, typename... Args>
		void CreateComponent(Entity& entity) {
            CreateComponent<componentType2, Args...>(entity);
            CreateComponent<componentType1>(entity);
        }

        template <typename componentType, typename... Args>
		core::vector<Entity> collectLinkedEntities() {
			numberOfBaseComponents = 0;
			unsigned int firstComponentArrayIndex = CreateComponentContainer<componentType>();
			core::vector<Entity>& dense = *static_cast<core::vector<Entity>*>
				( worldDenseComponentsMapToEntities[firstComponentArrayIndex] );

			if ( dense.GetSize() > 0 ) {
				++numberOfBaseComponents;
				numberOfBaseComponents += sizeof...(Args);
			}

			core::vector<Entity> returnVector;
			for ( unsigned int i = 0; i < dense.GetSize(); ++i ) {
				if ( multiCheckAvailability<Args...>(dense[i]) )
					returnVector.Push(dense[i]);
			}

			return returnVector;
		}

		template <typename componentType, typename... Args>
		core::vector<Entity> collectUniqueLinkedEntities() {
			core::vector<Entity> baseSubSetEntities;
			baseSubSetEntities = collectLinkedEntities<componentType, Args...>();

			unsigned int numberOfComponentArrays = 0;
			for ( unsigned int j = 0; j < baseSubSetEntities.GetSize(); ++j ) {
				numberOfComponentArrays = 0;
				for ( unsigned int i = 0; i < worldDenseComponentsMapToEntities.GetSize(); ++i ) {
					core::vector<Entity>& sparse = *static_cast<core::vector<Entity>*>
						(worldSparseEntitiesMapToComponents[i]);
					core::vector<Entity>& dense = *static_cast<core::vector<Entity>*>
						(worldDenseComponentsMapToEntities[i]);

						if ( checkAvailability(sparse, dense, baseSubSetEntities[j]) )
							++numberOfComponentArrays;
				}

				if ( numberOfComponentArrays > numberOfBaseComponents ) {
					baseSubSetEntities.Remove(baseSubSetEntities[j]);
					--j;
				}
			}
			
			return baseSubSetEntities;
		}
		
		template <typename... Args>
		bool multiCheckAvailability(Entity entity) {
			return (multiCheckAvailabilityBase<Args>(entity) && ...);
		}

		template <typename componentType>
		bool multiCheckAvailabilityBase(Entity entity) {
			unsigned int componentArrayIndex = CreateComponentContainer<componentType>();
			core::vector<Entity>& sparse = *static_cast<core::vector<Entity>*>
				(worldSparseEntitiesMapToComponents[componentArrayIndex]);
			core::vector<Entity>& dense = *static_cast<core::vector<Entity>*>
				(worldDenseComponentsMapToEntities[componentArrayIndex]);

			return checkAvailability(sparse, dense, entity);
		}
		
        template <typename componentType>
        componentType* GetComponent(const Entity& entity)
        {
            unsigned int localContainerID;
            localContainerID = CreateComponentContainer<componentType>();

			core::vector<Entity>& sparse = *static_cast<core::vector<Entity>*>
				(worldSparseEntitiesMapToComponents[localContainerID]);
			core::vector<Entity>& dense = *static_cast<core::vector<Entity>*>
				(worldDenseComponentsMapToEntities[localContainerID]);
			core::vector<componentType>& components =
				*static_cast<core::vector<componentType>*>(worldComponentsContainer[localContainerID]);

			if ( checkAvailability( sparse, dense, entity ) ) {
				Entity componentIndex = sparse[entity];
				return &components[componentIndex];
			} else {
				return nullptr;
			}
        }

        /**************************************************************************************
         * Dont need to delete real component in this method. Because systems dont work with
         * component without indices for that component in ordered container.
         **************************************************************************************/
        
		template <typename componentType>
		void RemoveComponent(Entity& entity) {
			unsigned int localContainerID;
			localContainerID = CreateComponentContainer<componentType>();

			core::vector<Entity>& sparse = *static_cast<core::vector<Entity>*>
				(worldSparseEntitiesMapToComponents[localContainerID]);
			core::vector<Entity>& dense = *static_cast<core::vector<Entity>*>
				(worldDenseComponentsMapToEntities[localContainerID]);
			core::vector<componentType>& components = *static_cast<core::vector<componentType>*>
				(worldComponentsContainer[localContainerID]);

			if ( checkAvailability( sparse, dense, entity ) ) {
				assert( dense.GetSize() == components.GetSize() );
//				std::cout << "DELETE: " << typeid(componentType).name() << std::endl;
				Entity indexInDenseOfRemovableEntity = sparse[entity];
				Entity indexInSparseOfSwapableEntity = dense.GetHead();
				const componentType& componentFromLastIndex = components.GetHead();
				dense[indexInDenseOfRemovableEntity] = indexInSparseOfSwapableEntity;
				dense.Pop();
				components[indexInDenseOfRemovableEntity] = componentFromLastIndex;
				components.Pop();
				sparse[indexInSparseOfSwapableEntity] = indexInDenseOfRemovableEntity;
			}
		}
		
		void RemoveAllComponents(Entity& entity) {
			// TODO: DYNAMIC CAST THAT CAN RETURN 0 IF CANT CAST
			
			// for ( unsigned int i = 0; i < worldComponentsContainer.GetSize(); ++i ) {
			// 	core::vector<Entity>& sparse = *static_cast<core::vector<Entity>*>
			// 		(worldSparseEntitiesMapToComponents[i]);
			// 	core::vector<Entity>& dense = *static_cast<core::vector<Entity>*>
			// 		(worldDenseComponentsMapToEntities[i]);
			// 	core::vector<core::IContainer>& components = *static_cast<core::vector<core::IContainer>*>
			// 		(worldComponentsContainer[i]);

			// 	if ( checkAvailability( sparse, dense, entity ) ) {
			// 		assert( dense.GetSize() == components.GetSize() );
			// 		std::cout << "DELETE!" << std::endl;
			// 		Entity indexInDenseOfRemovableEntity = sparse[entity];
			// 		Entity indexInSparseOfSwapableEntity = dense.GetHead();
			// 		const core::IContainer& componentFromLastIndex = components.GetHead();
			// 		dense[indexInDenseOfRemovableEntity] = indexInSparseOfSwapableEntity;
			// 		dense.Pop();
			// 		components[indexInDenseOfRemovableEntity] = componentFromLastIndex;
			// 		components.Pop();
			// 		sparse[indexInSparseOfSwapableEntity] = indexInDenseOfRemovableEntity;
			// 	}
			// }
			
			for ( unsigned int i = 0; i < worldComponentsContainer.GetSize(); ++i ) {
//				std::cout << "iteration: " << i << std::endl;
				if ( componentsTypes[i] == typeid(components::transform).name() ) {
					// std::cout << "transform from container: " << componentsTypes[i] << std::endl;
					// std::cout << "transform from typeid: " << typeid(components::transform).name() << std::endl;
					RemoveComponent<components::transform>(entity);
//					std::cout << "Delete transform" << std::endl;
				} else if ( componentsTypes[i] == typeid(components::beholder).name() ) {
					RemoveComponent<components::beholder>(entity);
//					std::cout << "Delete beholder" << std::endl;
				} else if ( componentsTypes[i] == typeid(components::animation).name() ) {
					RemoveComponent<components::animation>(entity);
//					std::cout << "Delete animation" << std::endl;
				} else if ( componentsTypes[i] == typeid(components::collider).name() ) {
					RemoveComponent<components::collider>(entity);
//					std::cout << "Delete collider" << std::endl;
				} else if ( componentsTypes[i] == typeid(components::directionalLight).name() ) {
					RemoveComponent<components::directionalLight>(entity);
//					std::cout << "Delete directional light" << std::endl;
				} else if ( componentsTypes[i] == typeid(components::pointLight).name() ) {
					RemoveComponent<components::pointLight>(entity);
//					std::cout << "Delete point light" << std::endl;
				} else if ( componentsTypes[i] == typeid(components::spotLight).name() ) {
					RemoveComponent<components::spotLight>(entity);
//					std::cout << "Delete spot light" << std::endl;
				} else if ( componentsTypes[i] == typeid(components::event).name() ) {
					RemoveComponent<components::event>(entity);
//					std::cout << "Delete event" << std::endl;
				} else if ( componentsTypes[i] == typeid(components::material).name() ) {
					RemoveComponent<components::material>(entity);
//					std::cout << "Delete material" << std::endl;
				} else if ( componentsTypes[i] == typeid(components::move).name() ) {
					RemoveComponent<components::move>(entity);
//					std::cout << "Delete move" << std::endl;
				} else if ( componentsTypes[i] == typeid(components::mesh).name() ) {
					RemoveComponent<components::mesh>(entity);
//					std::cout << "Delete vertex" << std::endl;
				} else if ( componentsTypes[i] == typeid(GLVM::ecs::components::controller).name() ) {
					RemoveComponent<GLVM::ecs::components::controller>(entity);
//					std::cout << "Delete controller" << std::endl;
				} else if ( componentsTypes[i] == typeid(GAME_MECHANICS::ECS::components::attack).name() ) {
					RemoveComponent<GAME_MECHANICS::ECS::components::attack>(entity);
//				 	std::cout << "Delete attack" << std::endl;
				} else if ( componentsTypes[i] == typeid(components::projectile).name() ) {
					RemoveComponent<components::projectile>(entity);
//				 	std::cout << "Delete projectile" << std::endl;
				} else {
					continue;
				}
			}
		}
		
		unsigned int GetContainerID();

		template <typename componentType>
		core::VectorIterator<componentType> GetComponentContainerTest() {
			core::vector<componentType>* componentVector = static_cast<core::vector<componentType>*>(worldComponentsContainer[CreateComponentContainer<componentType>()]);
			core::VectorIterator<componentType> iterator(*componentVector);
			return iterator;
		}

		// template <typename componentType>
		// core::VectorIterator<Entity> GetEntityContainer() {
		// 	core::vector<Entity>* entityVector = static_cast<core::vector<Entity>*>(worldDenseComponentsMapToEntities[CreateComponentContainer<componentType>()]);
		// 	core::VectorIterator<Entity> iterator(*entityVector);
		// 	return iterator;
		// }

		template <typename componentType>
		core::vector<componentType>* GetComponentContainer()
			{
				return static_cast<core::vector<componentType>*>(worldComponentsContainer[CreateComponentContainer<componentType>()]);
			}

		template <typename componentType>
		core::vector<Entity>* GetEntityContainer()
			{
				return static_cast<core::vector<Entity>*>(worldDenseComponentsMapToEntities[CreateComponentContainer<componentType>()]);
			}
	};


}

#endif
