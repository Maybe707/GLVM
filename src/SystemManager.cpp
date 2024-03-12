// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#include "SystemManager.hpp"

namespace GLVM::ecs
{
    CSystemManager* CSystemManager::pInstance_ = nullptr;
    std::mutex CSystemManager::Mutex_;
    
    CSystemManager::CSystemManager() {}
    
    CSystemManager::~CSystemManager()
    {
        delete pInstance_;
        pInstance_ = nullptr;
    }
    
    CSystemManager* CSystemManager::GetInstance()
    {
        std::lock_guard<std::mutex> lock(Mutex_);
        if(pInstance_ == nullptr)
        {
            pInstance_ = new CSystemManager();
        }
        return pInstance_;
    }
    
    void CSystemManager::ActivateSystem(ISystem* _System)
    {
        tSystemContainer.Push(_System);
        ++s_iSystem_ID;
    }

    void CSystemManager::Update()
    {
        for(unsigned int i = 0; i < s_iSystem_ID; ++i)
            tSystemContainer[i]->Update();
    }
}
