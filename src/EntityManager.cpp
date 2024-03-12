// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#include "EntityManager.hpp"
#include "ComponentManager.hpp"
#include "Vector.hpp"

namespace GLVM::ecs
{
    EntityManager* EntityManager::pInstance_ = nullptr;
    std::mutex EntityManager::Mutex_;
    
    EntityManager::EntityManager() {}
    EntityManager::~EntityManager() {}
    
    EntityManager* EntityManager::GetInstance()
    {
        std::lock_guard<std::mutex> lock(Mutex_);
        if(pInstance_ == nullptr)
        {
            pInstance_ = new EntityManager();
        }
        return pInstance_;
    }
    
    [[nodiscard]] Entity_ID EntityManager::CreateEntity()
    {
		Entity_ID _Entity_ID;
		
        if(tRemoved_Entity_Registry_.GetSize() > k_iNull)    ///< Check out wether or not free ID in removed entities registry.
        {
            _Entity_ID = tRemoved_Entity_Registry_.GetFirstItem();
            tActive_Entity_Registry_.Push(tRemoved_Entity_Registry_.GetFirstItem());
            tRemoved_Entity_Registry_.RemoveFirstItem();
        }
        else
        {
            tActive_Entity_Registry_.Push(u_iID);
            _Entity_ID = u_iID;
            ++u_iID;

        }
		return _Entity_ID;
    }

    /**************************************************************************************
     * Dont need to delete real component in this method. Because systems dont work with
     * component without indices for that component in ordered container.
     **************************************************************************************/
        
    void EntityManager::RemoveEntity(Entity_ID& _Entity_ID, ComponentManager* _ComponentManager)
    {
        // for(int i = 0, iSize = _ComponentManager->worldSparseEntitiesMapToComponents.GetSize(); i < iSize; ++i) {
        //     // static_cast<core::vector<unsigned int>*>(_ComponentManager->tWorld_IDs_Container[i])->RemoveItem(_Entity_ID);
		// 	core::vector<unsigned int>& vector =
		// 		*(static_cast<core::vector<unsigned int>*>
		// 		  (_ComponentManager->worldSparseEntitiesMapToComponents[i]));
		// 	core::VectorIterator<unsigned int> iterator = vector.Find(_Entity_ID);
		// 	if ( !iterator.ValidStatus() )
		// 		continue;

		// 	vector.Swap(iterator.Current(), vector.GetHead());
		// 	vector.Pop();
        // }

		_ComponentManager->RemoveAllComponents(_Entity_ID);
		tActive_Entity_Registry_[_Entity_ID] = k_iUint_Max;  
		tRemoved_Entity_Registry_.Push(_Entity_ID);
    }
}

