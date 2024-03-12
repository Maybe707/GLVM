// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#include "MeshManager.hpp"
#include "Components/VertexComponent.hpp"

namespace GLVM::core
{    
    MeshManager* MeshManager::pInstance_ = nullptr;
    std::mutex MeshManager::Mutex_;

    MeshManager::MeshManager() {}
    MeshManager::~MeshManager() {}

    void MeshManager::SetMesh(const char* _pathToMesh) {
        pathsArray_.push_back(_pathToMesh);
    }

	void MeshManager::SetMeshGLTF(const char* pathToMesh) {
		pathsGLTF_.Push(pathToMesh);
	}
	
    MeshManager* MeshManager::GetInstance()
    {
        
        std::lock_guard<std::mutex> lock(Mutex_);
        if(pInstance_ == nullptr)
        {
            pInstance_ = new MeshManager();
        }
        return pInstance_;
    }
}    
