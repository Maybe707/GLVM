// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#ifndef ENTITY_MANAGER
#define ENTITY_MANAGER

#include "Vector.hpp"
#include "ComponentManager.hpp"

typedef unsigned int Entity_ID;  

namespace GLVM::ecs
{
	class EntityManager
	{
        static EntityManager* pInstance_;
        static std::mutex  Mutex_;

		inline static Entity_ID u_iID = 0;		
 		core::vector<Entity_ID> tRemoved_Entity_Registry_;
		core::vector<Entity_ID> tActive_Entity_Registry_;
		
        EntityManager();
        ~EntityManager();
        
    public:                                                                   ///< !!!!!DELETE!!!!!!!!!!!!!!!11
        EntityManager(EntityManager& _entity_Manager) = delete;           ///< Dont need to make cope because of singleton property.
        void operator=(const EntityManager& _entity_Manager) = delete;     ///< Dont need assignment operator because of singleton property.
        static EntityManager* GetInstance();                      ///< It possibly to get only one instance of this class whith this method.
        
		[[nodiscard]] Entity_ID CreateEntity();

        void RemoveEntity(Entity_ID& _Entity_ID, ComponentManager* _ComponentManager);
	};
}

#endif
