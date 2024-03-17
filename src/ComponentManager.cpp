// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#include "ComponentManager.hpp"

namespace GLVM::ecs
{
    ComponentManager* ComponentManager::pInstance_ = nullptr;
    std::mutex ComponentManager::Mutex_;

    ComponentManager::ComponentManager() = default;
    
    ComponentManager::~ComponentManager() {
        for(int i = 0, iSize_Main = worldComponentsContainer.GetSize(); i < iSize_Main; ++i) {
            delete worldComponentsContainer[i];
            worldComponentsContainer[i] = nullptr;
        }
        for(int j = 0, iSize_Ordered = worldSparseEntitiesMapToComponents.GetSize(); j < iSize_Ordered; ++j) {
            delete worldSparseEntitiesMapToComponents[j];
            worldSparseEntitiesMapToComponents[j] = nullptr;
        }
    }

	bool ComponentManager::checkAvailability( core::vector<Entity>& sparse,
											   core::vector<Entity>& dense,
											   Entity entity ) {
		return entity < sparse.GetSize() && sparse[entity] < dense.GetSize() && dense[sparse[entity]] == entity;
	}
	
    unsigned int ComponentManager::GetContainerID() {
        return componentsContainerID;
    }

    ComponentManager* ComponentManager::GetInstance() {
        std::lock_guard<std::mutex> lock(Mutex_);
        if(pInstance_ == nullptr) {
            pInstance_ = new ComponentManager();
        }
        return pInstance_;
    }
}
