// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#include "Engine.hpp"
#include "Components/VertexComponent.hpp"
#include "ISoundEngine.hpp"
#include "GraphicAPI/Opengl.hpp"
#include "GraphicAPI/Vulkan.hpp"
#include "ShaderProgram.hpp"
#include "ISoundEngine.hpp"
#include "SoundEngineFactory.hpp"
#include "SystemManager.hpp"
#include "Systems/CameraSystem.hpp"
#include "Systems/CollisionSystem.hpp"
#include "Systems/GUISystem.hpp"
#include "Systems/MovementSystem.hpp"
#include "Systems/PhysicsSystem.hpp"
#include "Systems/ProjectileSystem.hpp"
#include "Texture.hpp"
#include <cstdint>
#include <limits>
#include <mutex>
#include <sys/types.h>
#include <thread>


/*******************************************************************
 * Legends never die...
 * You are about to face most terrifying data structures of all time.
 *    "Abandon hope all ye who enter here..." (c) Dante Alighieri.
 *******************************************************************
 *****************  👑  !!!  DESTRUCTOR_3000  !!!  👑  *************/

/*******************************************************************
*                                                                  *
*                             \_/                                  *
*                            (* *)                                 *
*                           __)#(__                                *
*                          ( )...( )(_)                            *
*                          || |_| ||//                             *
*                       >==() | | ()/                              *
*                           _(___)_                                *
*                          [-]   [-]                               *
*                                                                  *
********************************************************************/

#define DESTRUCTOR_3000													\
    std::cout << "You have been destructurized. [=]___[=]" << std::endl; \
    exit(1)

GLVM::core::CEvent g_eEvent;

namespace GLVM::core
{
    Engine* Engine::pInstance_ = nullptr;
    std::mutex Engine::Mutex_;

    void PlaybackSound(Sound::ISoundEngine* _sound_Engine) {
        while(1) {
			_sound_Engine->SoundStream();
		}
    }

    Engine::Engine() {
		chrono                   = Time::CTimerCreator().Create();
		soundEngine              = Sound::CSoundEngineFactory().CreateSoundEngine();

		collisionSystem          = new ecs::CCollisionSystem(Input_Stack_);
		movementSystem           = new ecs::CMovementSystem(Input_Stack_);
		physicsSystem            = new ecs::CPhysicsSystem(gravity, Input_Stack_);
		projectileSystem         = new ecs::CProjectileSystem(Input_Stack_);
        
		deltaFrameTime             = 0.0;
		g_eEvent.SetEvent(eDEFAULT);

		ecs::CSystemManager* pSystem_Manager = ecs::CSystemManager::GetInstance();

		///< Call of ActivateSystem function must be in this order.
		pSystem_Manager->ActivateSystem(movementSystem);
		pSystem_Manager->ActivateSystem(projectileSystem);
		pSystem_Manager->ActivateSystem(collisionSystem);
		pSystem_Manager->ActivateSystem(physicsSystem);

		std::thread sound_thread(PlaybackSound, std::ref(soundEngine));
		sound_thread.detach();
    }
	
    Engine::~Engine() {}
            
    Engine* Engine::GetInstance() {
		std::lock_guard<std::mutex> lock(Mutex_);
		if(pInstance_ == nullptr) {
			pInstance_ = new Engine();
		}
		return pInstance_;
    }

    void Engine::GameLoop(RendererType renderer) {
		if ( renderer == OPENGL_RENDERER ) {
			RenderOpengl();
			return;
		} else if ( renderer == VULKAN_RENDERER ) {
			RenderVulkan();
			return;
		}
    }

	void Engine::EventQueueFlush() {
	}
	
	void Engine::RenderOpengl() {
		ecs::CSystemManager* pSystem_Manager = ecs::CSystemManager::GetInstance();
		bool bGame_Loop_Active = true;

		projectileSystem->textureHandlers = textureHandlers;
		
		openglRenderer = new COpenglRenderer();
		openglRenderer->textureVector = textureVector;
		openglRenderer->pathsArray_            = pathsArray_;
		openglRenderer->pathsGLTF_             = pathsGLTF_;
		openglRenderer->run();
		openglRenderer->Window.Input_Stack_    = &Input_Stack_;
		
#ifdef __linux__
		// XEvent uXEvent;
		// while (XPending(openglRenderer->Window.GetDisplay())) {
		// 	XNextEvent(openglRenderer->Window.GetDisplay(), &uXEvent);
		// }

		// xcb_generic_event_t* event;
		// while (( event = xcb_poll_for_event ( openglRenderer->Window.GetConnection() ))) {
		// }
#endif

#ifdef _WIN32
		MSG msg;

		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				TranslateMessage( &msg );
				DispatchMessage( &msg );
		}
#endif
		
		while(bGame_Loop_Active) {
			deltaFrameTime = chrono->GetElapsed();
			chrono->Reset();
		    gravity += deltaFrameTime;

			openglRenderer->Window.ClearDisplay();
             
			openglRenderer->Window.HandleEvent(g_eEvent);
			if((Input_Stack_.SearchElement(EEvents::eGAME_LOOP_KILL)) == EEvents::eGAME_LOOP_KILL)
				bGame_Loop_Active = false;
						
//			Input_Stack_.PrintStack();
			g_eEvent.SetLastEvent(Input_Stack_);

			openglRenderer->Window.CursorLock(g_eEvent.mousePointerPosition.position_X,
								  g_eEvent.mousePointerPosition.position_Y,
								  &g_eEvent.mousePointerPosition.offset_X,
								  &g_eEvent.mousePointerPosition.offset_Y);

			movementSystem->deltaFrameTime            = deltaFrameTime;
			movementSystem->gravity                   = gravity;
			collisionSystem->fDelta_Time_             = deltaFrameTime;
			collisionSystem->gravity                  = gravity;
			projectileSystem->deltaFrameTime          = deltaFrameTime;
			projectileSystem->soundEngine             = soundEngine;
			physicsSystem->fDelta_Time_               = deltaFrameTime;
			physicsSystem->fAcceleration_of_Gravity_ += (deltaFrameTime / 20);
			physicsSystem->gravity                    = gravity;
			openglRenderer->EnlargeFrameAccumulator(deltaFrameTime);
			pSystem_Manager->Update();
			openglRenderer->draw();
			openglRenderer->Window.SwapBuffers();
		}

		openglRenderer->Window.Close();
	}
	
	void Engine::RenderVulkan() {
		ecs::CSystemManager* pSystem_Manager = ecs::CSystemManager::GetInstance();
		bool bGame_Loop_Active = true;

		projectileSystem->textureHandlers = textureHandlers;
		projectileSystem->meshHandlers    = meshHandlers;
		
		vulkanRenderer = new CVulkanRenderer();
		vulkanRenderer->initializeTextureData_ = textureVector;
		vulkanRenderer->pathsArray_            = pathsArray_;
		vulkanRenderer->pathsGLTF_             = pathsGLTF_;
		vulkanRenderer->run();
		vulkanRenderer->Window.Input_Stack_    = &Input_Stack_;		

#ifdef __linux__
		// XEvent uXEvent;
		// while (XPending(vulkanRenderer->Window.GetDisplay())) {
		// 	XNextEvent(vulkanRenderer->Window.GetDisplay(), &uXEvent);
		// }

		// xcb_generic_event_t* event;
		// while (( event = xcb_poll_for_event ( vulkanRenderer->Window.GetConnection() ))) {
		// }
#endif

#ifdef _WIN32
		MSG msg;

		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				TranslateMessage( &msg );
				DispatchMessage( &msg );
		}

		// while(GetMessageA(&msg, vulkanRenderer->Window.GetModernWindowHWND(), WM_KEYFIRST, WM_KEYLAST)) {
		// 		// TranslateMessage( &msg );
		// 		// DispatchMessage( &msg );
		// }
#endif

		while(bGame_Loop_Active) {
			deltaFrameTime = chrono->GetElapsed();
			chrono->Reset();
		    gravity += deltaFrameTime;

			vulkanRenderer->Window.ClearDisplay();
             
			vulkanRenderer->Window.HandleEvent(g_eEvent);
			// 	Input_Stack_.ControlInput(g_eEvent);
			if((Input_Stack_.SearchElement(EEvents::eGAME_LOOP_KILL)) == EEvents::eGAME_LOOP_KILL)
				bGame_Loop_Active = false;
			// }
			g_eEvent.SetLastEvent(Input_Stack_);
            
			vulkanRenderer->Window.CursorLock(g_eEvent.mousePointerPosition.position_X,
								  g_eEvent.mousePointerPosition.position_Y,
								  &g_eEvent.mousePointerPosition.offset_X,
								  &g_eEvent.mousePointerPosition.offset_Y);

			movementSystem->deltaFrameTime            = deltaFrameTime;
			movementSystem->gravity                   = gravity;
			collisionSystem->fDelta_Time_             = deltaFrameTime;
			collisionSystem->gravity                  = gravity;
			projectileSystem->deltaFrameTime          = deltaFrameTime;
			projectileSystem->soundEngine             = soundEngine;
			physicsSystem->fDelta_Time_               = deltaFrameTime;
			physicsSystem->fAcceleration_of_Gravity_ += (deltaFrameTime / 20);
			physicsSystem->gravity                    = gravity;
			vulkanRenderer->EnlargeFrameAccumulator(deltaFrameTime);
			pSystem_Manager->Update();
			vulkanRenderer->draw();
			vulkanRenderer->Window.SwapBuffers();
		}

		vulkanRenderer->Window.Close();
	}

	ecs::TextureHandle Engine::LoadTextureFromFile(const char* path_to_texture) {
		uint32_t textureID = textureVector.size();
		ecs::TextureHandle textureHandle;
		textureHandle.id = textureID;
		textureVector.push_back({ .path_to_image = path_to_texture });
		textureHandlers.Push(textureHandle);

		return textureHandle;
	}
	
	ecs::TextureHandle Engine::LoadTextureFromAddress(unsigned int iWidth, unsigned int iHeight,
								  unsigned int dat_length, unsigned char* u_iData) {
		uint32_t textureID = textureVector.size();
		ecs::TextureHandle textureHandle;
		textureHandle.id = textureID;
		textureVector.push_back({ .iWidth_ = iWidth, .iHeight_ = iHeight, .dat_length_ = dat_length, .u_iData_ = u_iData});
		textureHandlers.Push(textureHandle);

		return textureHandle;
    }

	ecs::components::MeshHandle Engine::LoadMeshFromFile_OBJ(const char* _pathToMesh) {
		ecs::components::MeshHandle meshHandle;
		meshHandle.id = meshID;
        pathsArray_.push_back(_pathToMesh);
		meshHandlers.Push(meshHandle);
		++meshID;

		return meshHandle;
    }

	ecs::components::MeshHandle Engine::LoadMeshFromFile_GLTF(const char* pathToMesh) {
		ecs::components::MeshHandle meshHandle;
		meshHandle.id = meshID;
        pathsGLTF_.Push(pathToMesh);
		meshHandlers.Push(meshHandle);
		++meshID;

		return meshHandle;
	}
	
	void Engine::FPScounter() {
		++fpsCounter;
		fpsAccumulator += deltaFrameTime;
		if (fpsAccumulator > 1.0f) {
			std::cout << "FPS: " << fpsCounter << std::endl;
			fpsCounter = 0;
			fpsAccumulator = 0;
		}
	}
	
    void Engine::GameKill()
    {
		// delete soundEngine;
		// soundEngine = nullptr;
		delete chrono;
		chrono = nullptr;
		delete collisionSystem;
		collisionSystem = nullptr;
		delete movementSystem;
		movementSystem = nullptr;
		delete physicsSystem;
		physicsSystem = nullptr;
		delete projectileSystem;
		projectileSystem = nullptr;
    }
} // namespace GLVM::core
