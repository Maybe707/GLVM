// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#ifndef ENGINE
#define ENGINE

#include "ComponentsFullSet.hpp"
#include "GraphicAPI/Opengl.hpp"
#include "GraphicAPI/Vulkan.hpp"
#include "SystemsFullSet.hpp"
#include "GLPointer.h"
#include "IChrono.hpp"
#include "IWindow.hpp"
#include "ISoundEngine.hpp"
#include "ShaderProgram.hpp"
#include "EventsStack.hpp"
#include "Event.hpp"
#include "Texture.hpp"
#include "TimerCreator.hpp"
#include "Vector.hpp"
#include "EntityManager.hpp"
#include "ComponentManager.hpp"
#include "SystemManager.hpp"
#include "IContainer.hpp"
#include <GL/gl.h>
#include <GL/glext.h>
#include "Constants.hpp"
#include <mutex>
#include "TextureManager.hpp"

using Entity = unsigned int;

namespace GLVM::core
{
	enum RendererType {
		OPENGL_RENDERER,
		VULKAN_RENDERER
	};
	
	class Engine
	{
        static Engine*    pInstance_;
        static std::mutex  Mutex_;
        
		Time::IChrono       * chrono;
        Sound::ISoundEngine * soundEngine;

		float                deltaFrameTime;
		float                gravity;
		CStack               Input_Stack_;
		std::vector<ecs::Texture> textureVector;
		core::vector<ecs::TextureHandle> textureHandlers;
		std::vector<const char*> pathsArray_;
		core::vector<const char*> pathsGLTF_;
		uint32_t meshID = 0;
		core::vector<ecs::components::MeshHandle> meshHandlers;
		CVulkanRenderer*     vulkanRenderer;
		COpenglRenderer*     openglRenderer;
        
        ecs::CCollisionSystem  * collisionSystem;
		ecs::CMovementSystem   * movementSystem;
        ecs::CPhysicsSystem    * physicsSystem;
        ecs::CProjectileSystem * projectileSystem;

		/// For FPS counting
		unsigned int fpsCounter = 0;
		double fpsAccumulator   = 0;
		
        Engine();
        
	public:

        
        ~Engine();
        
        Engine(Engine& _engine) = delete;                   ///< Dont need to make copy because of singleton property.
        void operator=(const Engine& _engine) = delete;      ///< Dont need assignment operator because of singleton property.
        static Engine* GetInstance();                        ///< It possibly to get only one instance of this class whith this method
        
		void GameLoop(RendererType renderer);
		void EventQueueFlush();
		void RenderOpengl();
		void RenderVulkan();
		ecs::TextureHandle LoadTextureFromFile(const char* path_to_texture);
		ecs::TextureHandle LoadTextureFromAddress(unsigned int iWidth, unsigned int iHeight,
								  unsigned int dat_length, unsigned char* u_iData);
		ecs::components::MeshHandle LoadMeshFromFile_OBJ(const char* _pathToMesh);
		ecs::components::MeshHandle LoadMeshFromFile_GLTF(const char* pathToMesh);
		void FPScounter();
		void GameKill();
	};
}

#endif
