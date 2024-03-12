// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#include "ComponentManager.hpp"
#include "GraphicAPI/Vulkan.hpp"
#include "Components/ControllerComponent.hpp"
#include "Components/MaterialComponent.hpp"
#include "Components/TransformComponent.hpp"
#include "Components/VertexComponent.hpp"
#include "Components/ViewComponent.hpp"
#include "Texture.hpp"
#include "Vector.hpp"
#include "WavefrontObjParser.hpp"
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <thread>
#include <vulkan/vulkan_core.h>


namespace GLVM::core
{    
    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
        static auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        } else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void CreateBeginDebugUtilsLabelEXT([[maybe_unused]] VkInstance instance, [[maybe_unused]] VkCommandBuffer commandBuffer, [[maybe_unused]] const VkDebugUtilsLabelEXT* labelInfo) {
#ifndef NDEBUG
        static auto func = (PFN_vkCmdBeginDebugUtilsLabelEXT) vkGetInstanceProcAddr(instance, "vkCmdBeginDebugUtilsLabelEXT");
            func(commandBuffer, labelInfo);
#endif
    }

    void CreateEndDebugUtilsLabelEXT([[maybe_unused]] VkInstance instance, [[maybe_unused]] VkCommandBuffer commandBuffer) {
#ifndef NDEBUG
        static auto func = (PFN_vkCmdEndDebugUtilsLabelEXT) vkGetInstanceProcAddr(instance, "vkCmdEndDebugUtilsLabelEXT");
            func(commandBuffer);
#endif
    }
	
	VkResult SetDebugObjectName(VkDevice device, const VkDebugUtilsObjectNameInfoEXT* objectNameInfo) {
		static auto func = (PFN_vkSetDebugUtilsObjectNameEXT) vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectNameEXT");
		if (func != nullptr) {
			return func(device, objectNameInfo);
		} else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}
	
    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
        static auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance, debugMessenger, pAllocator);
        }
    }

    CVulkanRenderer::CVulkanRenderer() {
    }
    
    CVulkanRenderer::~CVulkanRenderer() {
        cleanup();
    }
    
    void CVulkanRenderer::draw() {
		namespace cm = GLVM::ecs::components;
		
		ecs::ComponentManager* componentManager = GLVM::ecs::ComponentManager::GetInstance();
		core::vector<Entity> linkedEntities = componentManager->collectLinkedEntities<cm::beholder>();
		unsigned int linkedEntitiesVectorSize = linkedEntities.GetSize();
		for(unsigned int i = 0; i < linkedEntitiesVectorSize; ++i) {
			Entity currentEntity                = linkedEntities[i];
			cm::beholder* beholderComponent     = componentManager->GetComponent<cm::beholder>(currentEntity);
			cm::transform* transformComponent   = componentManager->GetComponent<cm::transform>(currentEntity);
			SetViewMatrix(*transformComponent, *beholderComponent);
		}
		
		SetProjectionMatrix();
		// mutex0.lock();
		// mutex1.lock();
		// mutex2.lock();
		// std::thread directionalLightShadowMapThread(&CVulkanRenderer::directionalLightShadowMapDrawFrame, this);
		// std::thread spotLightShadowMapThread(&CVulkanRenderer::spotLightShadowMapDrawFrame, this);
		// std::thread pointLightShadowMapThread(&CVulkanRenderer::pointLightShadowMapDrawFrame, this);
		// std::thread mainRenderThread(&CVulkanRenderer::mainRenderDrawFrame, this);

		directionalLightShadowMapDrawFrame();
		spotLightShadowMapDrawFrame();
		pointLightShadowMapDrawFrame();
		mainRenderDrawFrame();
		
		// #ifdef VK_USE_PLATFORM_XCB_KHR
        // vkDeviceWaitIdle(device);
		// #endif
		// directionalLightShadowMapThread.join();
		// spotLightShadowMapThread.join();
		// pointLightShadowMapThread.join();
		// mainRenderThread.join();
    }

    void CVulkanRenderer::loadWavefrontObj() {
        for (unsigned int m = 0; m < pathsArray_.size(); ++m) {
            CWaveFrontObjParser parser;
            CWaveFrontObjParser* wavefrontObjParser = &parser;
            
            wavefrontObjParser->ReadFile(pathsArray_[m]);
            wavefrontObjParser->ParseFile();

            aIndices_.emplace_back();
            aVertices_.emplace_back();

			frames.Push({});
			jointMatricesPerMesh.Push({});
            
            unsigned int vertexIndex  = 0;
            unsigned int textureIndex = 0;
			unsigned int normalIndex  = 0;
            unsigned int faceVerticesSize = wavefrontObjParser->getFaces().GetSize();

            for (unsigned int i = 0; i < faceVerticesSize; ++i)
                for (int j = 0; j < 3; ++j) {
                    vertexIndex     = wavefrontObjParser->getFaces()[i][0][j] - 1;
					aIndices_[m].push_back(i * 3 + j);
                    SVertex vertex  = wavefrontObjParser->getCoordinateVertices()[vertexIndex];
                    textureIndex    = wavefrontObjParser->getFaces()[i][1][j] - 1;
                    SVertex texture = wavefrontObjParser->getTextureVertices()[textureIndex];
					normalIndex     = wavefrontObjParser->getFaces()[i][2][j] - 1;
					SVertex normal  = wavefrontObjParser->getNormals()[normalIndex];

					vec4 jointIndices;
					vec4 weights;

					jointIndices[0] = -1;
					jointIndices[1] = -1;
					jointIndices[2] = -1;
					jointIndices[3] = -1;

					weights[0] = 1.0f;
					weights[1] = 1.0f;
					weights[2] = 1.0f;
					weights[2] = 1.0f;
					
                    aVertices_[m].push_back({{vertex[0], vertex[1], vertex[2]},
											 {normal[0], normal[1], normal[2]},
											 {texture[0], texture[1]},
											 {jointIndices[0], jointIndices[1], jointIndices[2]},
											 {weights[0], weights[1], weights[2]}});
                }
			
            vertexBufferContainer.emplace_back();
            vertexBufferMemoryContainer.emplace_back();
            createVertexBuffer(vertexBufferContainer[m], vertexBufferMemoryContainer[m], aVertices_[m]);

            indexBufferContainer.emplace_back();
            indexBufferMemoryContaner.emplace_back();
            createIndexBuffer(indexBufferContainer[m], indexBufferMemoryContaner[m], aIndices_[m]);
			++wavefrontObjCounter;
        }
    }

	void CVulkanRenderer::EnlargeFrameAccumulator(float value) {
		namespace cm = GLVM::ecs::components;
		
		ecs::ComponentManager* componentManager = GLVM::ecs::ComponentManager::GetInstance();
		core::vector<Entity> linkedEntities = componentManager->collectLinkedEntities<cm::transform>();
		unsigned int linkedEntitiesVectorSize = linkedEntities.GetSize();
		for(unsigned int i = 0; i < linkedEntitiesVectorSize; ++i) {
			Entity currentEntity                = linkedEntities[i];
			cm::transform* transformComponent   = componentManager->GetComponent<cm::transform>(currentEntity);
			unsigned int mesh_id                = componentManager->GetComponent<cm::mesh>(currentEntity)->handle.id;

			if ( jointMatricesPerMesh.GetSize() > 0 && jointMatricesPerMesh[mesh_id].GetSize() > 0 )
				transformComponent->frameAccumulator += value;
		}
	}
	
    void CVulkanRenderer::SetViewMatrix(mat4 _viewMatrix) {
        viewMatrix = _viewMatrix; // 
    }
    
    void CVulkanRenderer::SetProjectionMatrix(mat4 _projectionMatrix) {
        projectionMatrix = _projectionMatrix;
    }

    void CVulkanRenderer::SetViewMatrix(ecs::components::transform& _Player, ecs::components::beholder& cameraComponent)
    {
		Matrix<float, 4> viewMatrix_(1.0f);
        const float kSensitivity = 0.1f;

        fYaw = g_eEvent.mousePointerPosition.offset_X;
        fPitch = g_eEvent.mousePointerPosition.offset_Y;
        fYaw *= kSensitivity;
        fPitch *= kSensitivity;

        g_eEvent.mousePointerPosition.pitch = fPitch;
        g_eEvent.mousePointerPosition.yaw = fYaw;
        
        if(fPitch > 89.0f)
            fPitch = 89.0f;
        if(fPitch < -89.0f)
            fPitch = -89.0f;

		vec3 forward;
		float sinPitch = std::sin(Radians(fPitch / 2));
		float cosPitch = std::cos(Radians(fPitch / 2));
		float sinYaw = std::sin(Radians(-fYaw / 2));
		float cosYaw = std::cos(Radians(-fYaw / 2));
		
		Quaternion pitchQuat;
		Quaternion yawQuat;
		pitchQuat.w = cosPitch;
		pitchQuat.x = sinPitch;
		pitchQuat.y = 0.0f;
		pitchQuat.z = 0.0f;

		yawQuat.w = cosYaw;
		yawQuat.x = 0.0f;
		yawQuat.y = sinYaw;
		yawQuat.z = 0.0f;
		
		Quaternion result;
		result = multiplyQuaternion(yawQuat, pitchQuat);

		result = multiplyQuaternion(multiplyQuaternion(result, Quaternion{ .w = 0.0f, .x = 0.0f,
					.y = 0.0f, .z = -1.0f }), inverseQuaternion(result));

		forward[0] = result.x;
		forward[1] = result.y;
		forward[2] = result.z;
        cameraComponent.forward = Normalize(forward);
        viewMatrix_ = LookAtMain(_Player.tPosition,
								_Player.tPosition + cameraComponent.forward,
								cameraComponent.up);

		viewMatrix = viewMatrix_;
    }

    void CVulkanRenderer::SetProjectionMatrix()
	{
		mat4 tProjection_Matrix = Perspective(Radians(90.0f), (float)1920 / (float)1080, 0.1f, 100.0f);
		projectionMatrix = tProjection_Matrix;
		projectionMatrix[1][1] *= -1.0f;
	}
	
    void CVulkanRenderer::createTextureImage() {
		uint32_t texWidth, texHeight;
		[[maybe_unused]] uint32_t texChannels;

        for(unsigned int i = 0; i < initializeTextureData_.size(); ++i)
        {
			VkDeviceSize imageSize{};
			unsigned char* pixels;
			[[maybe_unused]] const char* path_to_stb_image = nullptr;

			#ifndef STB_IMAGE_IMPLEMENTATION
            imageSize = initializeTextureData_[i].dat_length_;
            pixels = initializeTextureData_[i].u_iData_;
            texWidth = initializeTextureData_[i].iWidth_;
            texHeight = initializeTextureData_[i].iHeight_;
			#endif

			#ifdef STB_IMAGE_IMPLEMENTATION
			path_to_stb_image = initializeTextureData_[i].path_to_image;
			pixels = stbi_load(path_to_stb_image, reinterpret_cast<int*>(&texWidth), reinterpret_cast<int*>(&texHeight),
							   reinterpret_cast<int*>(&texChannels), STBI_rgb_alpha);
			imageSize = texWidth * texHeight * 4;
			#endif

			if (!pixels) {
                throw std::runtime_error("failed to load texture image!");
            }

            VkBuffer stagingBuffer;
            VkDeviceMemory stagingBufferMemory;
            createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

            void* data;
            vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
            memcpy(data, pixels, static_cast<size_t>(imageSize));
            vkUnmapMemory(device, stagingBufferMemory);
			VK_Image textureImage = {
				.image = VkImage{},
				.deviceMemory = VkDeviceMemory{},
				.viewType = VK_IMAGE_VIEW_TYPE_2D,
				.createFlags  = 0,
				.memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				.usageFlags = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
				.aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT,
				.format = VK_FORMAT_R8G8B8A8_SRGB,
				.tiling = VK_IMAGE_TILING_OPTIMAL,
				.arrayLayers = 1,
				.width = texWidth,
				.height = texHeight
			};

            createImage(textureImage);

            transitionImageLayout(textureImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            copyBufferToImage(stagingBuffer, textureImage.image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
            transitionImageLayout(textureImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

			textureImages.push_back(textureImage);
			
            vkDestroyBuffer(device, stagingBuffer, nullptr);
            vkFreeMemory(device, stagingBufferMemory, nullptr);
        }
    }

    void CVulkanRenderer::recreateSwapChain() {
        vkDeviceWaitIdle(device);

        cleanupSwapChain();
		
        createSwapChain();
        createImageViews();
        createDepthResources();
		createDirectionalLightShadowMapDepthResources();
		createSpotLightShadowMapDepthResources();
		createPointLightShadowMapDepthResources();
		createFramebuffers();

		createDirectionalLightShadowMapTextureSamplers();
		createSpotLightShadowMapTextureSamplers();
		createPointLightShadowMapTextureSamplers();
		
		updateDirectionalLightShadowMapDescriptorSets();
		updateSpotLightShadowMapDescriptorSets();
		updatePointLightShadowMapDescriptorSets();
		updateDescriptorSets();
    }
    
    void CVulkanRenderer::SetTextureData(std::vector<ecs::Texture>& _texture_data) {
        texture_load_data_ = _texture_data;
    }

    void CVulkanRenderer::SetMeshData(std::vector<const char*> _pathsArray, core::vector<const char*> pathsGLTF) {
        for (unsigned int i = 0; i < _pathsArray.size(); ++i)
            pathsArray_.push_back(_pathsArray[i]);

		for (unsigned int i = 0; i < pathsGLTF.GetSize(); ++i)
			pathsGLTF_.Push(pathsGLTF[i]);
    }
    
    void CVulkanRenderer::run() {
        GLVM::core::MeshManager*   meshManager = GLVM::core::MeshManager::GetInstance();

		SetMeshData(meshManager->pathsArray_, meshManager->pathsGLTF_);

		namespace cm = GLVM::ecs::components;
		ecs::ComponentManager* componentManager   = ecs::ComponentManager::GetInstance();
		core::vector<Entity> directionalLightLinkedEntities = componentManager->collectLinkedEntities<cm::transform,
																									  cm::directionalLight,
																									  cm::mesh>();

		directionalLightNumber = directionalLightLinkedEntities.GetSize();
		directionalLightShadowMapTextureSamplers.resize(directionalLightNumber);

		core::vector<u32> DS_0_binding;
		core::vector<u32> DS_0_count;
		DS_0_binding.Push(0);
		DS_0_count.Push(1);
		
		directionalLightPipeline.addDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
											   DescriptorsTypes::DIRECTIONAL_LIGHT_SHADOW_MAP_MATRIX_UBO, VK_SHADER_STAGE_VERTEX_BIT, DS_0_count, DS_0_binding);
		
		directionalLightPipeline.vertShader = vertShaderFlatShadowMap;
		directionalLightPipeline.bindingDescription = Vertex::getBindingDescription();
		directionalLightPipeline.attributeDescriptions = Vertex::getAttributeDescriptions();

		core::vector<Entity> spotLightLinkedEntities = componentManager->collectLinkedEntities<cm::transform,
																							   cm::spotLight,
																							   cm::mesh>();

		spotLightNumber = spotLightLinkedEntities.GetSize();
		spotLightShadowMapTextureSamplers.resize(spotLightNumber);
		spotLightPipeline.addDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
										DescriptorsTypes::SPOT_LIGHT_SHADOW_MAP_MATRIX_UBO, VK_SHADER_STAGE_VERTEX_BIT, DS_0_count, DS_0_binding);
		
		spotLightPipeline.vertShader = vertShaderFlatShadowMap;
		spotLightPipeline.bindingDescription = Vertex::getBindingDescription();
		spotLightPipeline.attributeDescriptions = Vertex::getAttributeDescriptions();

		core::vector<Entity> pointLightLinkedEntities = componentManager->collectLinkedEntities<cm::transform,
																							   cm::pointLight,
																							   cm::mesh>();

		pointLightNumber = pointLightLinkedEntities.GetSize();
		pointLightShadowMapTextureSamplers.resize(pointLightNumber);
		pointLightPipeline.addDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
										 DescriptorsTypes::POINT_LIGHT_SHADOW_MAP_MATRIX_UBO, VK_SHADER_STAGE_VERTEX_BIT, DS_0_count, DS_0_binding);
		pointLightPipeline.vertShader = vertShaderCubeShadowMap;
		pointLightPipeline.fragShader = fragShaderCubeShadowMap;
		
		pointLightPipeline.bindingDescription = Vertex::getBindingDescription();
		pointLightPipeline.attributeDescriptions = Vertex::getAttributeDescriptions();


		core::vector<Entity> actorsLinkedEntities = componentManager->collectLinkedEntities<cm::transform,
																								cm::material,
																								cm::mesh>();

		actorsNumber = actorsLinkedEntities.GetSize();
		

		mainRenderScenePipeline.addDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, DescriptorsTypes::MODEL_MATRIX_UBO, VK_SHADER_STAGE_VERTEX_BIT, DS_0_count, DS_0_binding);
		mainRenderScenePipeline.addDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, DescriptorsTypes::LIGHT_DATA, VK_SHADER_STAGE_FRAGMENT_BIT, DS_0_count, DS_0_binding);
		mainRenderScenePipeline.addDescriptor(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, DescriptorsTypes::SPECULAR_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, DS_0_count, DS_0_binding);

		core::vector<u32> DS_0_3_bindigs;
		core::vector<u32> DS_0_3_count;

		DS_0_3_count.Push(1);
		DS_0_3_count.Push(4);
		DS_0_3_count.Push(32);
		DS_0_3_count.Push(8);

		DS_0_3_bindigs.Push(0);
		DS_0_3_bindigs.Push(1);
		DS_0_3_bindigs.Push(5);
		DS_0_3_bindigs.Push(37);
			
		mainRenderScenePipeline.addDescriptor(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, DescriptorsTypes::LIGHT_SAMPLERS, VK_SHADER_STAGE_FRAGMENT_BIT, DS_0_3_count, DS_0_3_bindigs);

		mainRenderScenePipeline.vertShader = vertShaderMain_;
		mainRenderScenePipeline.fragShader = fragShaderMain_;

		mainRenderScenePipeline.bindingDescription = Vertex::getBindingDescription();
		mainRenderScenePipeline.attributeDescriptions = Vertex::getAttributeDescriptions();
		
        initWindow();
        initVulkan();
    }
    
    void CVulkanRenderer::initWindow() {
#ifdef VK_USE_PLATFORM_XLIB_KHR
		Window.Close();
		Window = GLVM::core::WindowXVulkan();
        createXlibSurfaceInfo.dpy = Window.GetDisplay();
        createXlibSurfaceInfo.window = Window.GetWindow();

        createXlibSurfaceInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
        createXlibSurfaceInfo.pNext = nullptr;
        createXlibSurfaceInfo.flags = 0;
#endif

#ifdef VK_USE_PLATFORM_XCB_KHR
		Window.Disconnect();
		Window = GLVM::core::WindowXCBVulkan();
		createXcbSurfaceInfo.window = Window.GetWindow();
		createXcbSurfaceInfo.connection = Window.GetConnection();

		createXcbSurfaceInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
		createXcbSurfaceInfo.pNext = nullptr;
		createXcbSurfaceInfo.flags = 0;
#endif
		
#ifdef VK_USE_PLATFORM_WIN32_KHR
        createWin32SurfaceInfo.hwnd = Window.GetModernWindowHWND();
        
        createWin32SurfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        createWin32SurfaceInfo.pNext = nullptr;
        createWin32SurfaceInfo.flags = 0;
#endif
    }

	void CVulkanRenderer::initializeGLTF() {
		core::vector<bool> animationFlags;
		for (unsigned int m = 0; m < pathsGLTF_.GetSize(); ++m) {
			Core::CJsonParser jsonParser;
			aVertexesTemp_.emplace_back();
			aIndices_.emplace_back();
			frames.Push({});
			jointMatricesPerMesh.Push({});
			animationFlags.Push({});
			uint32_t nextIndexGLTF = wavefrontObjCounter + m;
			jsonParser.LoadGLTF(pathsGLTF_[m], aVertexesTemp_[m], aIndices_[nextIndexGLTF], jointMatricesPerMesh[nextIndexGLTF], frames[nextIndexGLTF], animationFlags[m]);
		}

		for (unsigned int m = 0; m < pathsGLTF_.GetSize(); ++m) {
//            aIndices_.emplace_back();
//            aVertices_.emplace_back();
			aVertices_.emplace_back();

			int stepOffset = 0;
			if ( animationFlags[m] )
				stepOffset = 8;
			else
				stepOffset = 16;
			for ( unsigned int n = 0; n < aVertexesTemp_[m].size(); n += stepOffset ) {
				SVertex vertex;
				vertex[0] = aVertexesTemp_[m][n];
			    vertex[1] = aVertexesTemp_[m][n + 1];
				vertex[2] = aVertexesTemp_[m][n + 2];
				SVertex normal;
				normal[0] = aVertexesTemp_[m][n + 3];
				normal[1] = aVertexesTemp_[m][n + 4];
				normal[2] = aVertexesTemp_[m][n + 5];
				SVertex texture;
				texture[0] = aVertexesTemp_[m][n + 6];
				texture[1] = aVertexesTemp_[m][n + 7];
				vec4 joinIndices;
				vec4 weights;
				if ( animationFlags[m] ) {
					joinIndices[0] = -1;
					joinIndices[1] = -1;
					joinIndices[2] = -1;
					joinIndices[3] = -1;

					weights[0] = 1;
					weights[1] = 1;
					weights[2] = 1;
					weights[3] = 1;
					
				} else {
					joinIndices[0] = aVertexesTemp_[m][n + 8];
					joinIndices[1] = aVertexesTemp_[m][n + 9];
					joinIndices[2] = aVertexesTemp_[m][n + 10];
					joinIndices[3] = aVertexesTemp_[m][n + 11];

					weights[0] = aVertexesTemp_[m][n + 12];
					weights[1] = aVertexesTemp_[m][n + 13];
					weights[2] = aVertexesTemp_[m][n + 14];
					weights[3] = aVertexesTemp_[m][n + 15];
				}

				uint32_t nextIndexGLTF = wavefrontObjCounter + m;
				aVertices_[nextIndexGLTF].push_back({{vertex[0], vertex[1], vertex[2]},
										 {normal[0], normal[1], normal[2]},
										 {texture[0], texture[1]},
										 {joinIndices[0], joinIndices[1], joinIndices[2], joinIndices[3]},
										 {weights[0], weights[1], weights[2], weights[3]}});
			}
			uint32_t nextIndexGLTF = wavefrontObjCounter + m;

            vertexBufferContainer.emplace_back();
            vertexBufferMemoryContainer.emplace_back();
            createVertexBuffer(vertexBufferContainer[nextIndexGLTF], vertexBufferMemoryContainer[nextIndexGLTF], aVertices_[nextIndexGLTF]);

            indexBufferContainer.emplace_back();
            indexBufferMemoryContaner.emplace_back();
            createIndexBuffer(indexBufferContainer[nextIndexGLTF], indexBufferMemoryContaner[nextIndexGLTF], aIndices_[nextIndexGLTF]);
		}
	}
	
    void CVulkanRenderer::initVulkan() {
        createInstance();
        setupDebugMessenger();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createSwapChain();
        createImageViews();
        createMainRenderPass();
		createDirectionalLightShadowMapRenderPass();
		createSpotLightShadowMapRenderPass();
		createPointLightShadowMapRenderPass();
        createDescriptorSetLayout(directionalLightPipeline.descriptors);
		createDescriptorSetLayout(spotLightPipeline.descriptors);
		createDescriptorSetLayout(pointLightPipeline.descriptors);
        createDescriptorSetLayout(mainRenderScenePipeline.descriptors);
        createGraphicsPipeline(directionalLightPipeline, directionalLightShadowMapRenderPass);
		createGraphicsPipeline(spotLightPipeline, spotLightShadowMapRenderPass);
		createGraphicsPipeline(pointLightPipeline, pointLightShadowMapRenderPass);
		createGraphicsPipeline(mainRenderScenePipeline, renderPass);
        createCommandPool(directionalLightCommandPool);
		createCommandPool(spotLightCommandPool);
		createCommandPool(pointLightCommandPool);
		createCommandPool(mainRenderCommandPool);
        createDepthResources();
		createDirectionalLightShadowMapDepthResources();
		createSpotLightShadowMapDepthResources();
		createPointLightShadowMapDepthResources();
        createFramebuffers();
        createTextureImage();
        createTextureImageView();
        createTextureSampler();
		createDirectionalLightShadowMapTextureSamplers();
		createSpotLightShadowMapTextureSamplers();
		createPointLightShadowMapTextureSamplers();
        loadWavefrontObj();
		initializeGLTF();
		
        createMainRenderUniformBuffers();
        createMainRenderDescriptorPool();
		createDirectionalLightShadowMapDescriptorSets();
		createSpotLightShadowMapDescriptorSets();
		createPointLightShadowMapDescriptorSets();
        createMainRenderDescriptorSets();
		setDebugObjectNames();
        createCommandBuffers(directionalLightCommandPool, directionalLightCommandBuffers);
		createCommandBuffers(spotLightCommandPool, spotLightCommandBuffers);
		createCommandBuffers(pointLightCommandPool, pointLightCommandBuffers);
		createCommandBuffers(mainRenderCommandPool, mainRenderCommandBuffers);
		createSyncObjects(directionalLightShadowMapImageAvailableSemaphores,
						  directionalLightShadowMapRenderFinishedSemaphores,
						  directionalLightShadowMapInFlightFences);
		createSyncObjects(spotLightShadowMapImageAvailableSemaphores,
						  spotLightShadowMapRenderFinishedSemaphores,
						  spotLightShadowMapInFlightFences);
		createSyncObjects(pointLightShadowMapImageAvailableSemaphores,
						  pointLightShadowMapRenderFinishedSemaphores,
						  pointLightShadowMapInFlightFences);
        createSyncObjects(imageAvailableSemaphores, renderFinishedSemaphores, inFlightFences);
    }

    void CVulkanRenderer::cleanupSwapChain() {
        vkDestroyImageView(device, depthImageView, nullptr);

		directionalLightPipeline.descriptors[0].textureImages.clear();
		spotLightPipeline.descriptors[0].textureImages.clear();
		pointLightPipeline.descriptors[0].textureImages.clear();
		
        for (VkFramebuffer& framebuffer : directionalLightShadowMapFrameBuffers) {
            vkDestroyFramebuffer(device, framebuffer, nullptr);
        }

		for (VkFramebuffer& framebuffer : spotLightShadowMapFrameBuffers) {
            vkDestroyFramebuffer(device, framebuffer, nullptr);
        }

		for (std::vector<VkFramebuffer>& inner_vector : pointLightShadowMapFrameBuffers) {
			for (VkFramebuffer& framebuffer : inner_vector) {
				vkDestroyFramebuffer(device, framebuffer, nullptr);
			} 
        }
		
        for (VkFramebuffer& framebuffer : swapChainFramebuffers) {
            vkDestroyFramebuffer(device, framebuffer, nullptr);
        }

        for (VkImageView& imageView : swapChainImageViews) {
            vkDestroyImageView(device, imageView, nullptr);
        }

        vkDestroySwapchainKHR(device, swapChain, nullptr);
    }

    void CVulkanRenderer::cleanup() {
        cleanupSwapChain();

        vkDestroyPipeline(device, graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
        vkDestroyRenderPass(device, renderPass, nullptr);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroyBuffer(device, modelMatrixUniformBuffers[i], nullptr);
            vkFreeMemory(device, modelMatrixUniformBuffersMemory[i], nullptr);
			vkDestroyBuffer(device, spotLightsUniformBuffers[i], nullptr);
            vkFreeMemory(device, spotLightsUniformBuffersMemory[i], nullptr);
        }

        vkDestroyDescriptorPool(device, descriptorPool, nullptr);

        for(unsigned int i = 0; i < textureImages.size(); ++i)
        {
            vkDestroySampler(device, textureImages[i].sampler, nullptr);
			for ( unsigned int j = 0; j < textureImages[i].views.size(); ++j )
				vkDestroyImageView(device, textureImages[i].views[j], nullptr);
			
			vkDestroyImage(device, textureImages[i].image, nullptr);
            vkFreeMemory(device, textureImages[i].deviceMemory, nullptr);
        }

		for ( unsigned int i = 0; i < directionalLightPipeline.descriptors.GetSize(); ++i ) 
			vkDestroyDescriptorSetLayout(device, directionalLightPipeline.descriptors[i].setLayout, nullptr);

		for ( unsigned int i = 0; i < spotLightPipeline.descriptors.GetSize(); ++i ) 
			vkDestroyDescriptorSetLayout(device, spotLightPipeline.descriptors[i].setLayout, nullptr);

		for ( unsigned int i = 0; i < pointLightPipeline.descriptors.GetSize(); ++i ) 
			vkDestroyDescriptorSetLayout(device, pointLightPipeline.descriptors[i].setLayout, nullptr);
		
		for ( unsigned int i = 0; i < mainRenderScenePipeline.descriptors.GetSize(); ++i ) 
			vkDestroyDescriptorSetLayout(device, mainRenderScenePipeline.descriptors[i].setLayout, nullptr);
		
        for (size_t i = 0; i < vertexBufferContainer.size(); ++i) {
            vkDestroyBuffer(device, indexBufferContainer[i], nullptr);
            vkFreeMemory(device, indexBufferMemoryContaner[i], nullptr);

            vkDestroyBuffer(device, vertexBufferContainer[i], nullptr);
            vkFreeMemory(device, vertexBufferMemoryContainer[i], nullptr);
        }

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(device, directionalLightShadowMapImageAvailableSemaphores[i], nullptr);
            vkDestroySemaphore(device, directionalLightShadowMapRenderFinishedSemaphores[i], nullptr);
            vkDestroyFence(device, directionalLightShadowMapInFlightFences[i], nullptr);

			vkDestroySemaphore(device, spotLightShadowMapImageAvailableSemaphores[i], nullptr);
            vkDestroySemaphore(device, spotLightShadowMapRenderFinishedSemaphores[i], nullptr);
            vkDestroyFence(device, spotLightShadowMapInFlightFences[i], nullptr);

			vkDestroySemaphore(device, pointLightShadowMapImageAvailableSemaphores[i], nullptr);
            vkDestroySemaphore(device, pointLightShadowMapRenderFinishedSemaphores[i], nullptr);
            vkDestroyFence(device, pointLightShadowMapInFlightFences[i], nullptr);
			
            vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(device, inFlightFences[i], nullptr);
        }

        vkDestroyCommandPool(device, directionalLightCommandPool, nullptr);
		vkDestroyCommandPool(device, spotLightCommandPool, nullptr);
		vkDestroyCommandPool(device, pointLightCommandPool, nullptr);
		vkDestroyCommandPool(device, mainRenderCommandPool, nullptr);

        vkDestroyDevice(device, nullptr);

        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        }

        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);
//        Window.Close();
    }

    void CVulkanRenderer::createInstance() {
        if (enableValidationLayers && !checkValidationLayerSupport()) {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Grey Lane Vertex Machine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        std::vector<const char*> extensions = getRequiredExtensions();
		
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
			
            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
        } else {
            createInfo.enabledLayerCount = 0;

            createInfo.pNext = nullptr;
        }

        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!");
        }
    }

    void CVulkanRenderer::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }

    void CVulkanRenderer::setupDebugMessenger() {
        if (!enableValidationLayers) return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }

    void CVulkanRenderer::createSurface() {
#ifdef VK_USE_PLATFORM_XLIB_KHR
        if (vkCreateXlibSurfaceKHR(instance, &createXlibSurfaceInfo, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
#endif

#ifdef VK_USE_PLATFORM_XCB_KHR
        if (vkCreateXcbSurfaceKHR(instance, &createXcbSurfaceInfo, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
#endif
		
#ifdef VK_USE_PLATFORM_WIN32_KHR
        if (vkCreateWin32SurfaceKHR(instance, &createWin32SurfaceInfo, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
#endif
    }

    void CVulkanRenderer::pickPhysicalDevice() {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        if (deviceCount == 0) {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        for (const VkPhysicalDevice& device : devices) {
            if (isDeviceSuitable(device)) {
                physicalDevice = device;
                break;
            }
        }

        if (physicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
    }

    void CVulkanRenderer::createLogicalDevice() {
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE;

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        createInfo.pEnabledFeatures = &deviceFeatures;

        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();

        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        } else {
            createInfo.enabledLayerCount = 0;
        }

        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!");
        }

        vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
        vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
    }

    void CVulkanRenderer::createSwapChain() {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D  extent = chooseSwapExtent(swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;

        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
        uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
            throw std::runtime_error("failed to create swap chain!");
        }

        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent;
    }

    void CVulkanRenderer::createImageViews() {
        swapChainImageViews.resize(swapChainImages.size());

        for (uint32_t i = 0; i < swapChainImages.size(); i++) {
			VK_Image swapChainImage		 = {
				.image				 = swapChainImages[i],
				.viewType			 = VK_IMAGE_VIEW_TYPE_2D,
				.aspectFlags         = VK_IMAGE_ASPECT_COLOR_BIT,
				.format				 = swapChainImageFormat,
				.red                 = VK_COMPONENT_SWIZZLE_IDENTITY,
				.green               = VK_COMPONENT_SWIZZLE_IDENTITY,
				.blue                = VK_COMPONENT_SWIZZLE_IDENTITY,
				.alpha               = VK_COMPONENT_SWIZZLE_IDENTITY,
				.arrayLayers         = 1,
				.width               = swapChainExtent.width,
				.height              = swapChainExtent.height
			};

            swapChainImageViews[i] = createImageView(swapChainImage, 0, 1);
        }
    }

    void CVulkanRenderer::createMainRenderPass() {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = swapChainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        
        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = findDepthFormat();
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = 0;
        dependency.dstSubpass = VK_SUBPASS_EXTERNAL;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
//        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT ;
        
        std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }
    }

    void CVulkanRenderer::createDirectionalLightShadowMapRenderPass() {
        VkAttachmentDescription attachmentDescription{};
        
		attachmentDescription.format = findDepthFormat();
        attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
        attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

		/// Attachment references form subpasses
        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 0;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		/// Subpass 0: shadow map rendering
        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 0;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

		VkSubpassDependency dependencies[2];
		dependencies[0].srcSubpass		= VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass		= 0;
		dependencies[0].srcStageMask	= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[0].dstStageMask	= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependencies[0].srcAccessMask	= VK_ACCESS_SHADER_READ_BIT;
		dependencies[0].dstAccessMask	= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		dependencies[1].srcSubpass		= 0;
		dependencies[1].dstSubpass		= VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask	= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependencies[1].dstStageMask	= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[1].srcAccessMask	= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask	= VK_ACCESS_SHADER_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &attachmentDescription;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 2;
        renderPassInfo.pDependencies = dependencies;

        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &directionalLightShadowMapRenderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }
    }

    void CVulkanRenderer::createSpotLightShadowMapRenderPass() {
        VkAttachmentDescription attachmentDescription{};
        
		attachmentDescription.format = findDepthFormat();
        attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
        attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

		/// Attachment references form subpasses
        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 0;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		/// Subpass 0: shadow map rendering
        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 0;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

		VkSubpassDependency dependencies[2];
		dependencies[0].srcSubpass		= VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass		= 0;
		dependencies[0].srcStageMask	= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[0].dstStageMask	= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependencies[0].srcAccessMask	= VK_ACCESS_SHADER_READ_BIT;
		dependencies[0].dstAccessMask	= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		dependencies[1].srcSubpass		= 0;
		dependencies[1].dstSubpass		= VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask	= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependencies[1].dstStageMask	= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[1].srcAccessMask	= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask	= VK_ACCESS_SHADER_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &attachmentDescription;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 2;
        renderPassInfo.pDependencies = dependencies;

        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &spotLightShadowMapRenderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }
    }

    void CVulkanRenderer::createPointLightShadowMapRenderPass() {
        VkAttachmentDescription attachmentDescription{};
        
		attachmentDescription.format = findDepthFormat();
        attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
        attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

		/// Attachment references form subpasses
        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 0;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		/// Subpass 0: shadow map rendering
        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 0;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

		VkSubpassDependency dependencies[2];
		dependencies[0].srcSubpass		= VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass		= 0;
		dependencies[0].srcStageMask	= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[0].dstStageMask	= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependencies[0].srcAccessMask	= VK_ACCESS_SHADER_READ_BIT;
		dependencies[0].dstAccessMask	= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		dependencies[1].srcSubpass		= 0;
		dependencies[1].dstSubpass		= VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask	= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependencies[1].dstStageMask	= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[1].srcAccessMask	= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask	= VK_ACCESS_SHADER_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		std::array<VkAttachmentDescription, 1> attachments = {attachmentDescription};
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 2;
        renderPassInfo.pDependencies = dependencies;

        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &pointLightShadowMapRenderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }
    }
	
    void CVulkanRenderer::createDescriptorSetLayout(core::vector<Descriptor>& descriptors) {
		for ( u32 i = 0; i < descriptors.GetSize(); ++i ) {
			u32 bindigs_size = descriptors[i].binding.GetSize();
			std::vector<VkDescriptorSetLayoutBinding> bindings;
			for ( u32 j = 0; j < bindigs_size; ++j ) {
				VkDescriptorSetLayoutBinding modelMatrixUboLayout{};
				modelMatrixUboLayout.binding = descriptors[i].binding[j];
				modelMatrixUboLayout.descriptorCount = descriptors[i].descriptorsNumber[j];
				modelMatrixUboLayout.descriptorType = descriptors[i].vkType;
				modelMatrixUboLayout.pImmutableSamplers = nullptr;
				modelMatrixUboLayout.stageFlags = descriptors[i].shaderStageFlag;

				bindings.push_back(modelMatrixUboLayout);
			}

			VkDescriptorSetLayoutCreateInfo layoutInfo{};
			layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutInfo.flags = 0;
			layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
			layoutInfo.pBindings = bindings.data();

			if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptors[i].setLayout) != VK_SUCCESS) {
				throw std::runtime_error("failed to create descriptor set layout!");
			}
		}
    }

    void CVulkanRenderer::createGraphicsPipeline(Pipeline& pipeline, VkRenderPass& renderPass) {
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

		VkShaderModule vertShaderModule;
		VkShaderModule fragShaderModule;
		if (pipeline.vertShader != nullptr) {
			std::vector<char> vertShaderCode = readFile(pipeline.vertShader);
			vertShaderModule = createShaderModule(vertShaderCode);

			VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
			vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
			vertShaderStageInfo.module = vertShaderModule;
			vertShaderStageInfo.pName = "main";

			shaderStages.push_back(vertShaderStageInfo);
		}

		if (pipeline.fragShader != nullptr) {
			std::vector<char> fragShaderCode = readFile(pipeline.fragShader);
			fragShaderModule = createShaderModule(fragShaderCode);

			VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
			fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			fragShaderStageInfo.module = fragShaderModule;
			fragShaderStageInfo.pName = "main";

			shaderStages.push_back(fragShaderStageInfo);
		}

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(pipeline.attributeDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = &pipeline.bindingDescription;
        vertexInputInfo.pVertexAttributeDescriptions = pipeline.attributeDescriptions.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = 0;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.stencilTestEnable = VK_FALSE;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

		unsigned int descriptorLayoutsNumber = pipeline.descriptors.GetSize();
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
		for (unsigned int i = 0; i < descriptorLayoutsNumber; ++i) {
			descriptorSetLayouts.push_back(pipeline.descriptors[i].setLayout);
		}
			
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = descriptorLayoutsNumber;
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();

        if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipeline.pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = shaderStages.size();
        pipelineInfo.pStages = shaderStages.data();
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = pipeline.pipelineLayout;
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline.pipeline) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline!");
        }

		if (pipeline.vertShader != nullptr)
			vkDestroyShaderModule(device, vertShaderModule, nullptr);

		if (pipeline.fragShader != nullptr)
			vkDestroyShaderModule(device, fragShaderModule, nullptr);
    }

    void CVulkanRenderer::createFramebuffers() {
		/// Main renderer frame buffers initialization
		swapChainFramebuffers.resize(swapChainImageViews.size());
        for (size_t i = 0; i < swapChainImageViews.size(); ++i) {
			std::vector<VkImageView> mainRenderAttachments;
			mainRenderAttachments.push_back(swapChainImageViews[i]);
			mainRenderAttachments.push_back(depthImageView);

			createRenderPassFramebuffers(mainRenderAttachments, renderPass, swapChainFramebuffers[i],
										 swapChainExtent.width, swapChainExtent.height);
		}

		/// Directional lights shadow map renderer frame buffers initialization
		directionalLightShadowMapFrameBuffers.resize(DIRECTIONAL_LIGHTS_NUMBER);
        for (size_t i = 0; i < DIRECTIONAL_LIGHTS_NUMBER; ++i) {
			std::vector<VkImageView> directionalLightsRenderAttachments;
			directionalLightsRenderAttachments.push_back(directionalLightPipeline.descriptors[0].textureImages[i].views[0]);
			createRenderPassFramebuffers(directionalLightsRenderAttachments, directionalLightShadowMapRenderPass, directionalLightShadowMapFrameBuffers[i], swapChainExtent.width, swapChainExtent.height);
		}

		/// Spot lights shadow map renderer frame buffers initialization
		spotLightShadowMapFrameBuffers.resize(SPOT_LIGHTS_NUMBER);
        for (size_t i = 0; i < SPOT_LIGHTS_NUMBER; ++i) {
			std::vector<VkImageView> spotLightsRenderAttachments;
			spotLightsRenderAttachments.push_back(spotLightPipeline.descriptors[0].textureImages[i].views[0]);
			
			createRenderPassFramebuffers(spotLightsRenderAttachments, spotLightShadowMapRenderPass, spotLightShadowMapFrameBuffers[i], swapChainExtent.width, swapChainExtent.height);
		}

		/// Point lights shadow map renderer frame buffers initialization
		pointLightShadowMapFrameBuffers.resize(POINT_LIGHTS_NUMBER);
		for ( size_t j = 0; j < POINT_LIGHTS_NUMBER; ++j ) {
			for ( size_t m = 0; m < 6; ++m ) {
				std::vector<VkImageView> pointLightsRenderAttachments;
				pointLightsRenderAttachments.push_back(pointLightPipeline.descriptors[0].textureImages[j].views[m]);
				pointLightShadowMapFrameBuffers[j].push_back({});
				createRenderPassFramebuffers(pointLightsRenderAttachments, pointLightShadowMapRenderPass, pointLightShadowMapFrameBuffers[j][m], SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
			}
		}
    }

    void CVulkanRenderer::createRenderPassFramebuffers(std::vector<VkImageView>& attachments, VkRenderPass& renderPass_, VkFramebuffer& swapChainFramebuffer, uint32_t width, uint32_t height) {
            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass_;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = width;
            framebufferInfo.height = height;
            framebufferInfo.layers = 1;
			
            if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffer) != VK_SUCCESS) {
                throw std::runtime_error("failed to create framebuffer!");
            }
    }
	
    void CVulkanRenderer::createCommandPool(VkCommandPool& commandPool) {
        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

        if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics command pool!");
        }
    }

    void CVulkanRenderer::createDepthResources() {
        VkFormat depthFormat = findDepthFormat();

		VK_Image depthImage		 = {
			.image				 = VkImage{},
			.deviceMemory		 = VkDeviceMemory{},
			.viewType			 = VK_IMAGE_VIEW_TYPE_2D,
			.createFlags		 = 0,
			.memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			.usageFlags			 = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			.aspectFlags         = VK_IMAGE_ASPECT_DEPTH_BIT,
			.format				 = depthFormat,
			.tiling				 = VK_IMAGE_TILING_OPTIMAL,
			.arrayLayers		 = 1,
			.width				 = swapChainExtent.width,
			.height				 = swapChainExtent.height,
		};

		createImage(depthImage);
        depthImageView = createImageView(depthImage, 0, 1);
    }

	void CVulkanRenderer::createDirectionalLightShadowMapDepthResources() {
		for ( unsigned int i = 0; i < DIRECTIONAL_LIGHTS_NUMBER; ++i ) {
			VK_Image depthImage		 = {
				.image				 = VkImage{},
				.deviceMemory		 = VkDeviceMemory{},
				.viewType			 = VK_IMAGE_VIEW_TYPE_2D,
				.createFlags		 = 0,
				.memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				.usageFlags			 = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
				.aspectFlags         = VK_IMAGE_ASPECT_DEPTH_BIT,
				.format				 = findDepthFormat(),
				.tiling				 = VK_IMAGE_TILING_OPTIMAL,
				.arrayLayers		 = 1,
				.width				 = swapChainExtent.width,
				.height				 = swapChainExtent.height,
			};

			createImage(depthImage);
			
			VkCommandBuffer commandBuffer = beginSingleTimeCommands(directionalLightCommandPool);

			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = depthImage.image;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;

			VkPipelineStageFlags sourceStage;
			VkPipelineStageFlags destinationStage;

			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = 0;

			sourceStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

			vkCmdPipelineBarrier(
				commandBuffer,
				sourceStage, destinationStage,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier
				);

			endSingleTimeCommands(directionalLightCommandPool, commandBuffer);
			
			depthImage.views.push_back(createImageView(depthImage, 0, 1));
			setImageDebugObjectName(depthImage);
			directionalLightPipeline.descriptors[0].textureImages.push_back(depthImage);
		}
	}

	void CVulkanRenderer::createSpotLightShadowMapDepthResources() {
		for ( unsigned int i = 0; i < SPOT_LIGHTS_NUMBER; ++i ) {
			VK_Image depthImage		 = {
				.image				 = VkImage{},
				.deviceMemory		 = VkDeviceMemory{},
				.viewType			 = VK_IMAGE_VIEW_TYPE_2D,
				.createFlags		 = 0,
				.memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				.usageFlags			 = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
				.aspectFlags         = VK_IMAGE_ASPECT_DEPTH_BIT,
				.format				 = findDepthFormat(),
				.tiling				 = VK_IMAGE_TILING_OPTIMAL,
				.arrayLayers		 = 1,
				.width				 = swapChainExtent.width,
				.height				 = swapChainExtent.height,
			};

			createImage(depthImage);

			VkCommandBuffer commandBuffer = beginSingleTimeCommands(spotLightCommandPool);

			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = depthImage.image;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;

			VkPipelineStageFlags sourceStage;
			VkPipelineStageFlags destinationStage;

			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = 0;

			sourceStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

			vkCmdPipelineBarrier(
				commandBuffer,
				sourceStage, destinationStage,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier
				);

			endSingleTimeCommands(spotLightCommandPool, commandBuffer);
			
			depthImage.views.push_back(createImageView(depthImage, 0, 1));
			setImageDebugObjectName(depthImage);
			spotLightPipeline.descriptors[0].textureImages.push_back(depthImage);
		}
	}

	void CVulkanRenderer::createPointLightShadowMapDepthResources() {
		for ( unsigned int i = 0; i < POINT_LIGHTS_NUMBER; ++i ) {
			VK_Image depthImage		 = {
				.image				 = VkImage{},
				.deviceMemory		 = VkDeviceMemory{},
				.viewType			 = VK_IMAGE_VIEW_TYPE_2D,
				.createFlags		 = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
				.memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				.usageFlags			 = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
				.aspectFlags         = VK_IMAGE_ASPECT_DEPTH_BIT,
				.format				 = findDepthFormat(),
				.tiling				 = VK_IMAGE_TILING_OPTIMAL,
				.arrayLayers		 = 6,
				.width				 = SHADOW_MAP_SIZE,
				.height				 = SHADOW_MAP_SIZE
			};
			
			createImage(depthImage);

			for ( unsigned int j = 0; j < 6; ++j ) {
				VkCommandBuffer commandBuffer = beginSingleTimeCommands(pointLightCommandPool);

				VkImageMemoryBarrier barrier{};
				barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
				barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				barrier.image = depthImage.image;
				barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
				barrier.subresourceRange.baseMipLevel = 0;
				barrier.subresourceRange.levelCount = 1;
				barrier.subresourceRange.baseArrayLayer = 0;
				barrier.subresourceRange.layerCount = 6;

				VkPipelineStageFlags sourceStage;
				VkPipelineStageFlags destinationStage;

				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = 0;

				sourceStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
				destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

				vkCmdPipelineBarrier(
					commandBuffer,
					sourceStage, destinationStage,
					0,
					0, nullptr,
					0, nullptr,
					1, &barrier
					);

				endSingleTimeCommands(pointLightCommandPool, commandBuffer);

				
				depthImage.views.push_back(createImageView(depthImage, j, 1));
			}

			setImageDebugObjectName(depthImage);
			pointLightPipeline.descriptors[0].textureImages.push_back(depthImage);
		}

		for ( unsigned int i = 0; i < POINT_LIGHTS_NUMBER; ++i ) {
			pointLightPipeline.descriptors[0].textureImages[i].viewType = VK_IMAGE_VIEW_TYPE_CUBE;

			setImageDebugObjectName(pointLightPipeline.descriptors[0].textureImages[i]);
			pointLightPipeline.descriptors[0].textureImages[i].views.push_back(createImageView(pointLightPipeline.descriptors[0].textureImages[i], 0, 6));
		}
	}
	
    VkFormat CVulkanRenderer::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
        for (VkFormat format : candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }

        throw std::runtime_error("failed to find supported format!");
    }

    VkFormat CVulkanRenderer::findDepthFormat() {
        return findSupportedFormat(
            {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
            );
    }

    bool CVulkanRenderer::hasStencilComponent(VkFormat format) {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

    void CVulkanRenderer::createTextureImageView() {
        for(unsigned int i = 0; i < initializeTextureData_.size(); ++i)
            textureImages[i].views.push_back(createImageView(textureImages[i], 0, 1));
    }

    void CVulkanRenderer::createShadowMapTextureImageView() {
        for(unsigned int i = 0; i < pointLightImages.size(); ++i)
            pointLightImages[i].views.push_back(createImageView(pointLightImages[i], 0, 1));
    }
	
    void CVulkanRenderer::createTextureSampler() {
        for(unsigned int i = 0; i < initializeTextureData_.size(); ++i)
        {
            VkPhysicalDeviceProperties properties{};
            vkGetPhysicalDeviceProperties(physicalDevice, &properties);

            VkSamplerCreateInfo samplerInfo{};
            samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            samplerInfo.magFilter = VK_FILTER_NEAREST;
            samplerInfo.minFilter = VK_FILTER_NEAREST;
            samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerInfo.anisotropyEnable = VK_TRUE;
            samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
            samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
            samplerInfo.unnormalizedCoordinates = VK_FALSE;
            samplerInfo.compareEnable = VK_FALSE;
            samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
            samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

			textureImages[i].sampler = {};              /// TODO: Is it realy need here?
            if (vkCreateSampler(device, &samplerInfo, nullptr, &textureImages[i].sampler) != VK_SUCCESS) {
                throw std::runtime_error("failed to create texture sampler!");
            }
        }
    }

    void CVulkanRenderer::createShadowMapTextureSampler() {
        for(unsigned int i = 0; i < pointLightImages.size(); ++i)
        {
            VkPhysicalDeviceProperties properties{};
            vkGetPhysicalDeviceProperties(physicalDevice, &properties);

            VkSamplerCreateInfo samplerInfo{};
            samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            samplerInfo.magFilter = VK_FILTER_NEAREST;
            samplerInfo.minFilter = VK_FILTER_NEAREST;
            samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerInfo.anisotropyEnable = VK_TRUE;
            samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
            samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
            samplerInfo.unnormalizedCoordinates = VK_FALSE;
            samplerInfo.compareEnable = VK_FALSE;
            samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
            samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

			pointLightImages[i].sampler = {};              /// TODO: Is it realy need here?
            if (vkCreateSampler(device, &samplerInfo, nullptr, &pointLightImages[i].sampler) != VK_SUCCESS) {
                throw std::runtime_error("failed to create texture sampler!");
            }
        }
    }
	
    void CVulkanRenderer::createDirectionalLightShadowMapTextureSamplers() {
        for(unsigned int i = 0; i < directionalLightPipeline.descriptors[0].textureImages.size(); ++i)
			createRenderPassShadowMapTextureSamplers(directionalLightPipeline.descriptors[0].textureImages[i].sampler);
    }

    void CVulkanRenderer::createSpotLightShadowMapTextureSamplers() {
        for(unsigned int i = 0; i < spotLightPipeline.descriptors[0].textureImages.size(); ++i)
			createRenderPassShadowMapTextureSamplers(spotLightPipeline.descriptors[0].textureImages[i].sampler);
    }

    void CVulkanRenderer::createPointLightShadowMapTextureSamplers() {
        for(unsigned int i = 0; i < pointLightPipeline.descriptors[0].textureImages.size(); ++i)
			createRenderPassShadowMapTextureSamplers(pointLightPipeline.descriptors[0].textureImages[i].sampler);
    }
	
	void CVulkanRenderer::createRenderPassShadowMapTextureSamplers(VkSampler& shadowMapTextureSampler) {
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(physicalDevice, &properties);

		VkFilter shadowMapFilter = VK_FILTER_LINEAR;
			
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = shadowMapFilter;
		samplerInfo.minFilter = shadowMapFilter;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeV = samplerInfo.addressModeU;
		samplerInfo.addressModeW = samplerInfo.addressModeU;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 1.0f;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE;

		if (vkCreateSampler(device, &samplerInfo, nullptr, &shadowMapTextureSampler) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture sampler!");
		}
	}
	
    VkImageView CVulkanRenderer::createImageView(VK_Image image, uint32_t baseArrayLayers, uint32_t layerCount) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image.image;
        viewInfo.viewType = image.viewType;
        viewInfo.format = image.format;
		viewInfo.components.r = image.red;
		viewInfo.components.g = image.green;
		viewInfo.components.b = image.blue;
		viewInfo.components.a = image.alpha;
        viewInfo.subresourceRange.aspectMask = image.aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = baseArrayLayers;
        viewInfo.subresourceRange.layerCount = layerCount;

        VkImageView imageView;
        if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
        }

        return imageView;
    }

    void CVulkanRenderer::createImage(VK_Image& image) {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = image.width;
        imageInfo.extent.height = image.height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = image.arrayLayers;
        imageInfo.format = image.format;
        imageInfo.tiling = image.tiling;
        imageInfo.usage = image.usageFlags;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.flags = image.createFlags;

        if (vkCreateImage(device, &imageInfo, nullptr, &image.image) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(device, image.image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, image.memoryPropertyFlags);

        if (vkAllocateMemory(device, &allocInfo, nullptr, &image.deviceMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate image memory!");
        }

        vkBindImageMemory(device, image.image, image.deviceMemory, 0);
    }

    void CVulkanRenderer::transitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout) {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands(mainRenderCommandPool);

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else {
            throw std::invalid_argument("unsupported layout transition!");
        }

        vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
            );

        endSingleTimeCommands(mainRenderCommandPool, commandBuffer);
    }

    void CVulkanRenderer::transitionShadowMapImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout) {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands(directionalLightCommandPool);

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else {
            throw std::invalid_argument("unsupported layout transition!");
        }

        vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
            );

        endSingleTimeCommands(directionalLightCommandPool, commandBuffer);
    }
	
    void CVulkanRenderer::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands(mainRenderCommandPool);

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {
            width,
            height,
            1
        };

        vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        endSingleTimeCommands(mainRenderCommandPool, commandBuffer);
    }

    void CVulkanRenderer::createVertexBuffer(VkBuffer& _vertexBuffer, VkDeviceMemory& _vertexBufferMemory, const std::vector<Vertex>& _vertices) {
        VkDeviceSize bufferSize = sizeof(_vertices[0]) * _vertices.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, _vertices.data(), (size_t) bufferSize);
        vkUnmapMemory(device, stagingBufferMemory);

        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _vertexBuffer, _vertexBufferMemory);

        copyBuffer(stagingBuffer, _vertexBuffer, bufferSize);

        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
    }

    void CVulkanRenderer::createIndexBuffer(VkBuffer& _indexBuffer, VkDeviceMemory& _indexBufferMemory, const std::vector<uint32_t>& _indices) {
        VkDeviceSize bufferSize = sizeof(_indices[0]) * _indices.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, _indices.data(), (size_t) bufferSize);
        vkUnmapMemory(device, stagingBufferMemory);

        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _indexBuffer, _indexBufferMemory);

        copyBuffer(stagingBuffer, _indexBuffer, bufferSize);

        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
    }

    void CVulkanRenderer::createMainRenderUniformBuffers() {
        VkDeviceSize modelMatrixBufferSize = sizeof(ModelMatrixUBO);
		VkDeviceSize lightDataBufferSize = sizeof(LightData);
		VkDeviceSize lightSpaceMatrixSize = sizeof(LightSpaceMatrixUBO);
		VkDeviceSize modelShadowMapMatrixBufferSize = sizeof(ShadowMapMatrixUBO);
		VkDeviceSize modelCubeShadowMapMatrixBufferSize = sizeof(PointLightShadowMapMatrixUBO);
		namespace cm = GLVM::ecs::components;
		ecs::ComponentManager* componentManager   = ecs::ComponentManager::GetInstance();

		core::vector<Entity> matrixLinkedEntities = componentManager->collectLinkedEntities<cm::transform,
																							cm::material,
																							cm::mesh>();

		u32 memory = 0;
		constexpr u32 UBO_multiplier = 2;
		matrixUboDescriptorsNumber = matrixLinkedEntities.GetSize() * UBO_multiplier;
		modelMatrixUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		modelMatrixUniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT * matrixUboDescriptorsNumber; i++) {
			// createBuffer(modelMatrixBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			// 			 modelMatrixUniformBuffers[i], modelMatrixUniformBuffersMemory[i]);

			memory += modelMatrixBufferSize;
		}

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			createBuffer(memory, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						 modelMatrixUniformBuffers[i], modelMatrixUniformBuffersMemory[i]);

//				memory += modelMatrixBufferSize;
		}
		memory = 0;
		
		lightSpaceMatrixBuffer.resize(MAX_FRAMES_IN_FLIGHT);
        lightSpaceMatrixMemory.resize(MAX_FRAMES_IN_FLIGHT);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            createBuffer(lightSpaceMatrixSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						 lightSpaceMatrixBuffer[i], lightSpaceMatrixMemory[i]);

			memory += lightDataBufferSize;
		}
		memory = 0;

		core::vector<Entity> actorsLinkedEntities = componentManager->collectLinkedEntities<cm::transform,
																								cm::material,
																								cm::mesh>();
		
		core::vector<Entity> directionalLightLinkedEntities = componentManager->collectLinkedEntities<cm::directionalLight>();
		directionalLightUboDescriptorsNumber = (actorsLinkedEntities.GetSize() * UBO_multiplier) * directionalLightLinkedEntities.GetSize();

		if ( directionalLightUboDescriptorsNumber > 0 ) {
			shadowMapDirectionalLightModelMatrixUniformBuffers.resize(1);
			shadowMapDirectionalLightModelMatrixUniformBuffersMemory.resize(1);

			for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT * directionalLightUboDescriptorsNumber; i++) {
				memory += modelShadowMapMatrixBufferSize;
			}

			createBuffer(memory, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						 shadowMapDirectionalLightModelMatrixUniformBuffers[0], shadowMapDirectionalLightModelMatrixUniformBuffersMemory[0]);
		} else {
			shadowMapDirectionalLightModelMatrixUniformBuffers.resize(1);
			shadowMapDirectionalLightModelMatrixUniformBuffersMemory.resize(1);

			createBuffer(2 * modelShadowMapMatrixBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						 shadowMapDirectionalLightModelMatrixUniformBuffers[0], shadowMapDirectionalLightModelMatrixUniformBuffersMemory[0]);

		}
		memory = 0;

		core::vector<Entity> spotLightLinkedEntities = componentManager->collectLinkedEntities<cm::spotLight>();
		spotLightUboDescriptorsNumber = (actorsLinkedEntities.GetSize() * UBO_multiplier) * spotLightLinkedEntities.GetSize();

		if ( spotLightUboDescriptorsNumber > 0 ) {
			shadowMapSpotLightModelMatrixUniformBuffers.resize(1);
			shadowMapSpotLightModelMatrixUniformBuffersMemory.resize(1);

			for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT * spotLightUboDescriptorsNumber; i++) {
				memory += modelShadowMapMatrixBufferSize;
			}

			createBuffer(memory, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						 shadowMapSpotLightModelMatrixUniformBuffers[0], shadowMapSpotLightModelMatrixUniformBuffersMemory[0]);
		} else {
			shadowMapSpotLightModelMatrixUniformBuffers.resize(1);
			shadowMapSpotLightModelMatrixUniformBuffersMemory.resize(1);

			createBuffer(2 * modelShadowMapMatrixBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						 shadowMapSpotLightModelMatrixUniformBuffers[0], shadowMapSpotLightModelMatrixUniformBuffersMemory[0]);
		}
		memory = 0;

		core::vector<Entity> pointLightsLinkedEntities = componentManager->collectLinkedEntities<cm::transform,
																								 cm::material,
																								 cm::mesh>();

		
		pointLightUboDescriptorsNumber = (actorsLinkedEntities.GetSize() * UBO_multiplier) * pointLightsLinkedEntities.GetSize();

		if ( pointLightUboDescriptorsNumber > 0 ) {
			shadowMapPointLightModelMatrixUniformBuffers.resize(1);
			shadowMapPointLightModelMatrixUniformBuffersMemory.resize(1);

			for (size_t i = 0; i < 6 * MAX_FRAMES_IN_FLIGHT * pointLightUboDescriptorsNumber; i++) {
				memory += modelCubeShadowMapMatrixBufferSize;
			}

			createBuffer(memory, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
							 shadowMapPointLightModelMatrixUniformBuffers[0], shadowMapPointLightModelMatrixUniformBuffersMemory[0]);
		} else {
			shadowMapPointLightModelMatrixUniformBuffers.resize(1);
			shadowMapPointLightModelMatrixUniformBuffersMemory.resize(1);

			createBuffer(6 * 2 * modelCubeShadowMapMatrixBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						 shadowMapPointLightModelMatrixUniformBuffers[0], shadowMapPointLightModelMatrixUniformBuffersMemory[0]);
		}
		memory += modelCubeShadowMapMatrixBufferSize;			
		
		core::vector<Entity> viewPositionLinkedEntities = componentManager->collectLinkedEntities<cm::transform,
																								  cm::beholder,
																								  cm::mesh>();
		
		lightDataUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		lightDataUniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			createBuffer(lightDataBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						 lightDataUniformBuffers[i], lightDataUniformBuffersMemory[i]);

			memory += lightDataBufferSize;
		}
    }

    void CVulkanRenderer::createMainRenderDescriptorPool() {
        std::array<VkDescriptorPoolSize, 2> poolSizes{};

		uint32_t descriptorCount = 50000;
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(descriptorCount);
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = static_cast<uint32_t>(descriptorCount);

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = static_cast<uint32_t>(descriptorCount);

        if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    void CVulkanRenderer::createDirectionalLightShadowMapDescriptorSets() {
		core::vector<u32> dirLightBindings = directionalLightPipeline.getBindingOfDescriptor(DescriptorsTypes::DIRECTIONAL_LIGHT_SHADOW_MAP_MATRIX_UBO);

		int directionalLightShadowMapMatrixUboBinding = dirLightBindings[0];
		
		unsigned int actual_size = directionalLightUboDescriptorsNumber ? directionalLightUboDescriptorsNumber : 1;
		
		std::vector<VkDescriptorSetLayout> matrixUboLayouts(MAX_FRAMES_IN_FLIGHT * actual_size,
															directionalLightPipeline.descriptors[directionalLightShadowMapMatrixUboBinding].setLayout);
		VkDescriptorSetAllocateInfo matrixUboAllocInfo{};
		matrixUboAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		matrixUboAllocInfo.descriptorPool = descriptorPool;
		matrixUboAllocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT *
																	  actual_size);
		matrixUboAllocInfo.pSetLayouts = matrixUboLayouts.data();
			
		shadowMapDirectionalLightDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT * actual_size);
		if (vkAllocateDescriptorSets(device, &matrixUboAllocInfo, shadowMapDirectionalLightDescriptorSets.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT * actual_size; ++i) {
			VkDescriptorBufferInfo modelMatrixBufferInfo{};
			modelMatrixBufferInfo.buffer = shadowMapDirectionalLightModelMatrixUniformBuffers[0];
			modelMatrixBufferInfo.offset = i * sizeof(ShadowMapMatrixUBO);
			modelMatrixBufferInfo.range = sizeof(ShadowMapMatrixUBO);
			
			std::array<VkWriteDescriptorSet, 1> descriptorWrites{};
			
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = shadowMapDirectionalLightDescriptorSets[i];
			descriptorWrites[0].dstBinding = directionalLightShadowMapMatrixUboBinding;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &modelMatrixBufferInfo;

			vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
	}

    void CVulkanRenderer::createSpotLightShadowMapDescriptorSets() {
		core::vector<u32> spotLightsBindings = spotLightPipeline.getBindingOfDescriptor(DescriptorsTypes::SPOT_LIGHT_SHADOW_MAP_MATRIX_UBO);
 
		int spotLightShadowMapMatrixUboBinding = spotLightsBindings[0];
		
		unsigned int actual_size = spotLightUboDescriptorsNumber ? spotLightUboDescriptorsNumber : 1;
		
		std::vector<VkDescriptorSetLayout> matrixUboLayouts(MAX_FRAMES_IN_FLIGHT * actual_size,
															spotLightPipeline.descriptors[spotLightShadowMapMatrixUboBinding].setLayout);
		VkDescriptorSetAllocateInfo matrixUboAllocInfo{};
		matrixUboAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		matrixUboAllocInfo.descriptorPool = descriptorPool;
		matrixUboAllocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT *
																	  actual_size);
		matrixUboAllocInfo.pSetLayouts = matrixUboLayouts.data();
			
		shadowMapSpotLightDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT * actual_size);
		if (vkAllocateDescriptorSets(device, &matrixUboAllocInfo, shadowMapSpotLightDescriptorSets.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT * actual_size; ++i) {
			VkDescriptorBufferInfo modelMatrixBufferInfo{};
			modelMatrixBufferInfo.buffer = shadowMapSpotLightModelMatrixUniformBuffers[0];
			modelMatrixBufferInfo.offset = i * sizeof(ShadowMapMatrixUBO);
			modelMatrixBufferInfo.range = sizeof(ShadowMapMatrixUBO);
			
			std::array<VkWriteDescriptorSet, 1> descriptorWrites{};
			
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = shadowMapSpotLightDescriptorSets[i];
			descriptorWrites[0].dstBinding = spotLightShadowMapMatrixUboBinding;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &modelMatrixBufferInfo;

			vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
	}

	void CVulkanRenderer::createPointLightShadowMapDescriptorSets() {
		core::vector<u32> pointLightBindings = pointLightPipeline.getBindingOfDescriptor(DescriptorsTypes::POINT_LIGHT_SHADOW_MAP_MATRIX_UBO);

		int pointLightShadowMapMatrixUboBinding = pointLightBindings[0];
		
		unsigned int point_light_shadow_map_actual_size = pointLightUboDescriptorsNumber ? pointLightUboDescriptorsNumber : 1; 
		
		if ( pointLightShadowMapMatrixUboBinding != -1 ) {
			std::vector<VkDescriptorSetLayout> matrixUboLayouts(6 * MAX_FRAMES_IN_FLIGHT * point_light_shadow_map_actual_size,
																pointLightPipeline.descriptors[pointLightShadowMapMatrixUboBinding].setLayout);
			VkDescriptorSetAllocateInfo matrixUboAllocInfo{};
			matrixUboAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			matrixUboAllocInfo.descriptorPool = descriptorPool;
			matrixUboAllocInfo.descriptorSetCount = static_cast<uint32_t>(6 * MAX_FRAMES_IN_FLIGHT *
																		  point_light_shadow_map_actual_size);
			matrixUboAllocInfo.pSetLayouts = matrixUboLayouts.data();

			shadowMapPointLightDescriptorSets.resize(6 * MAX_FRAMES_IN_FLIGHT * point_light_shadow_map_actual_size);
			if (vkAllocateDescriptorSets(device, &matrixUboAllocInfo, shadowMapPointLightDescriptorSets.data()) != VK_SUCCESS) {
				throw std::runtime_error("failed to allocate descriptor sets!");
			}

			for (size_t i = 0; i < 6 * MAX_FRAMES_IN_FLIGHT * point_light_shadow_map_actual_size; ++i) {
				VkDescriptorBufferInfo modelMatrixBufferInfo{};
				modelMatrixBufferInfo.buffer = shadowMapPointLightModelMatrixUniformBuffers[0];
				modelMatrixBufferInfo.offset = i * sizeof(PointLightShadowMapMatrixUBO);
				modelMatrixBufferInfo.range = sizeof(PointLightShadowMapMatrixUBO);
			
				std::array<VkWriteDescriptorSet, 1> descriptorWrites{};
			
				descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[0].dstSet = shadowMapPointLightDescriptorSets[i];
				descriptorWrites[0].dstBinding = pointLightShadowMapMatrixUboBinding;
				descriptorWrites[0].dstArrayElement = 0;
				descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrites[0].descriptorCount = 1;
				descriptorWrites[0].pBufferInfo = &modelMatrixBufferInfo;

				vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
			}
		}
	}
	
    void CVulkanRenderer::createMainRenderDescriptorSets() {
		core::vector<u32> modelMatrixBindings = mainRenderScenePipeline.getBindingOfDescriptor(DescriptorsTypes::MODEL_MATRIX_UBO);

		int modelMatrixUboBinding = modelMatrixBindings[0];
		
		unsigned int model_matrix_ubo_actual_size = matrixUboDescriptorsNumber ? matrixUboDescriptorsNumber : 1; 
		
		std::vector<VkDescriptorSetLayout> matrixUboLayouts(MAX_FRAMES_IN_FLIGHT * model_matrix_ubo_actual_size,
															mainRenderScenePipeline.descriptors[0].setLayout);
		VkDescriptorSetAllocateInfo matrixUboAllocInfo{};
		matrixUboAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		matrixUboAllocInfo.descriptorPool = descriptorPool;
		matrixUboAllocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * model_matrix_ubo_actual_size);
		matrixUboAllocInfo.pSetLayouts = matrixUboLayouts.data();
			
		matrixUboDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT * model_matrix_ubo_actual_size);
		if (vkAllocateDescriptorSets(device, &matrixUboAllocInfo, matrixUboDescriptorSets.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT * model_matrix_ubo_actual_size; ++i) {
			VkDescriptorBufferInfo modelMatrixBufferInfo{};
			uint32_t uboIndex = 0;
			if ( i > MAX_FRAMES_IN_FLIGHT * model_matrix_ubo_actual_size / 2 )
				uboIndex = 1;
			modelMatrixBufferInfo.buffer = modelMatrixUniformBuffers[uboIndex];
			modelMatrixBufferInfo.offset = i * sizeof(ModelMatrixUBO);
			modelMatrixBufferInfo.range = sizeof(ModelMatrixUBO);
			
			std::array<VkWriteDescriptorSet, 1> descriptorWrites{};
			
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = matrixUboDescriptorSets[i];
			descriptorWrites[0].dstBinding = modelMatrixUboBinding;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &modelMatrixBufferInfo;

			vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}

		core::vector<u32> lightDataBindings = mainRenderScenePipeline.getBindingOfDescriptor(DescriptorsTypes::LIGHT_DATA);

		int lightDataUboBinding = lightDataBindings[0];
		
		std::vector<VkDescriptorSetLayout> lightDataUboLayouts(MAX_FRAMES_IN_FLIGHT,
																  mainRenderScenePipeline.descriptors[1].setLayout);
		VkDescriptorSetAllocateInfo lightDataUboAllocInfo{};
		lightDataUboAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		lightDataUboAllocInfo.descriptorPool = descriptorPool;
		lightDataUboAllocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		lightDataUboAllocInfo.pSetLayouts = lightDataUboLayouts.data();
			
		lightDataUboDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
		if (vkAllocateDescriptorSets(device, &lightDataUboAllocInfo, lightDataUboDescriptorSets.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
			VkDescriptorBufferInfo modelMatrixBufferInfo{};
			modelMatrixBufferInfo.buffer = lightDataUniformBuffers[i];
			modelMatrixBufferInfo.offset = 0;
			modelMatrixBufferInfo.range = sizeof(LightData);
			
			std::array<VkWriteDescriptorSet, 1> descriptorWrites{};
			
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = lightDataUboDescriptorSets[i];
			descriptorWrites[0].dstBinding = lightDataUboBinding;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &modelMatrixBufferInfo;

			vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}

		core::vector<u32> specularSamplerBindigs = mainRenderScenePipeline.getBindingOfDescriptor(DescriptorsTypes::SPECULAR_SAMPLER);
 
		int specularSamplerBinding = specularSamplerBindigs[0];

		if ( initializeTextureData_.size() > 0 ) {
			u32 DS_specular_number = initializeTextureData_.size();
			std::vector<VkDescriptorSetLayout> specularSamplerUboLayouts(MAX_FRAMES_IN_FLIGHT * DS_specular_number,
																		 mainRenderScenePipeline.descriptors[2].setLayout);
			VkDescriptorSetAllocateInfo specularSamplerUboAllocInfo{};
			specularSamplerUboAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			specularSamplerUboAllocInfo.descriptorPool = descriptorPool;
			specularSamplerUboAllocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * DS_specular_number);
			specularSamplerUboAllocInfo.pSetLayouts = specularSamplerUboLayouts.data();
			
			specularSamplerDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT * DS_specular_number);
			if (vkAllocateDescriptorSets(device, &specularSamplerUboAllocInfo, specularSamplerDescriptorSets.data()) != VK_SUCCESS) {
				throw std::runtime_error("failed to allocate descriptor sets!");
			}

			for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT * DS_specular_number; ++i) {
				VkDescriptorImageInfo imageInfo{};
				imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				unsigned int textureIndex = i / 2;
				imageInfo.imageView = textureImages[textureIndex].views[0];
				imageInfo.sampler = textureImages[textureIndex].sampler;

				std::array<VkWriteDescriptorSet, 1> descriptorWrites{};
			
				descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[0].dstSet = specularSamplerDescriptorSets[i];
				descriptorWrites[0].dstBinding = specularSamplerBinding;
				descriptorWrites[0].dstArrayElement = 0;
				descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrites[0].descriptorCount = 1;
				descriptorWrites[0].pImageInfo = &imageInfo;

				vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
			}
		}
		
		core::vector<u32> lightsSamplersBindigs = mainRenderScenePipeline.getBindingOfDescriptor(DescriptorsTypes::LIGHT_SAMPLERS);

		int diffuseCisBinding = lightsSamplersBindigs[0];
		int directionalLightShadowMapsCisBinding = lightsSamplersBindigs[1];
		int pointLightShadowMapsCisBinding = lightsSamplersBindigs[2];
		int spotLightShadowMapsCisBinding = lightsSamplersBindigs[3];

		for (size_t i = 0; i < DIRECTIONAL_LIGHTS_NUMBER; ++i) {
			directionalLightsImageInfo[i] = {};
			directionalLightsImageInfo[i].imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			directionalLightsImageInfo[i].imageView = directionalLightPipeline.descriptors[0].textureImages[i].views[0];
			directionalLightsImageInfo[i].sampler = directionalLightPipeline.descriptors[0].textureImages[i].sampler;
		}

		for (size_t i = 0; i < POINT_LIGHTS_NUMBER; ++i) {
			pointLightsImageInfo[i] = {};
			pointLightsImageInfo[i].imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			pointLightsImageInfo[i].imageView = pointLightPipeline.descriptors[0].textureImages[i].views[6];
			pointLightsImageInfo[i].sampler = pointLightPipeline.descriptors[0].textureImages[i].sampler;
		}

		for (size_t i = 0; i < SPOT_LIGHTS_NUMBER; ++i) {
			spotLightsImageInfo[i] = {};
			spotLightsImageInfo[i].imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			spotLightsImageInfo[i].imageView = spotLightPipeline.descriptors[0].textureImages[i].views[0];
			spotLightsImageInfo[i].sampler = spotLightPipeline.descriptors[0].textureImages[i].sampler;
		}

		if ( initializeTextureData_.size() > 0 ) {
			u32 DS_number = initializeTextureData_.size();
			std::vector<VkDescriptorSetLayout> diffuseSamplerUboLayouts(MAX_FRAMES_IN_FLIGHT * DS_number,
																		mainRenderScenePipeline.descriptors[3].setLayout);
			VkDescriptorSetAllocateInfo diffuseSamplerUboAllocInfo{};
			diffuseSamplerUboAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			diffuseSamplerUboAllocInfo.descriptorPool = descriptorPool;
			diffuseSamplerUboAllocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * DS_number);
			diffuseSamplerUboAllocInfo.pSetLayouts = diffuseSamplerUboLayouts.data();
		
			diffuseSamplerDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT * DS_number);
			if (vkAllocateDescriptorSets(device, &diffuseSamplerUboAllocInfo, diffuseSamplerDescriptorSets.data()) != VK_SUCCESS) {
				throw std::runtime_error("failed to allocate descriptor sets!");
			}

			constexpr u32 DS_writes_size = 4;
			for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT * DS_number; ++i) {
				VkDescriptorImageInfo imageInfo{};
				imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				unsigned int textureIndex = i / 2;
				imageInfo.imageView = textureImages[textureIndex].views[0];
				imageInfo.sampler = textureImages[textureIndex].sampler;
			
				std::array<VkWriteDescriptorSet, DS_writes_size> descriptorWrites{};
				descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[0].dstSet = diffuseSamplerDescriptorSets[i];
				descriptorWrites[0].dstBinding = diffuseCisBinding;
				descriptorWrites[0].dstArrayElement = 0;
				descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrites[0].descriptorCount = 1;
				descriptorWrites[0].pImageInfo = &imageInfo;

				descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[1].dstSet = diffuseSamplerDescriptorSets[i];
				descriptorWrites[1].dstBinding = directionalLightShadowMapsCisBinding;
				descriptorWrites[1].dstArrayElement = 0;
				descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrites[1].descriptorCount = DIRECTIONAL_LIGHTS_NUMBER;
				descriptorWrites[1].pImageInfo = directionalLightsImageInfo;

				descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[2].dstSet = diffuseSamplerDescriptorSets[i];
				descriptorWrites[2].dstBinding = pointLightShadowMapsCisBinding;
				descriptorWrites[2].dstArrayElement = 0;
				descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrites[2].descriptorCount = POINT_LIGHTS_NUMBER;
				descriptorWrites[2].pImageInfo = pointLightsImageInfo;

				descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[3].dstSet = diffuseSamplerDescriptorSets[i];
				descriptorWrites[3].dstBinding = spotLightShadowMapsCisBinding;
				descriptorWrites[3].dstArrayElement = 0;
				descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrites[3].descriptorCount = SPOT_LIGHTS_NUMBER;
				descriptorWrites[3].pImageInfo = spotLightsImageInfo;

				vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
			}
		}
	}

	void CVulkanRenderer::updateSamplersDescriptroSets(uint32_t diffuse_id, uint32_t specular_id) {
		core::vector<u32> lightsSamplersBindigs = mainRenderScenePipeline.getBindingOfDescriptor(DescriptorsTypes::LIGHT_SAMPLERS);

		int diffuseCisBinding = lightsSamplersBindigs[0];
		int specularCisBinding = lightsSamplersBindigs[1];
		int directionalLightShadowMapsCisBinding = lightsSamplersBindigs[2];
		int pointLightShadowMapsCisBinding = lightsSamplersBindigs[3];
		int spotLightShadowMapsCisBinding = lightsSamplersBindigs[4];

		for (size_t i = 0; i < DIRECTIONAL_LIGHTS_NUMBER; ++i) {
			directionalLightsImageInfo[i] = {};
			directionalLightsImageInfo[i].imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			directionalLightsImageInfo[i].imageView = directionalLightPipeline.descriptors[0].textureImages[i].views[0];
			directionalLightsImageInfo[i].sampler = directionalLightPipeline.descriptors[0].textureImages[i].sampler;
		}

		for (size_t i = 0; i < POINT_LIGHTS_NUMBER; ++i) {
			pointLightsImageInfo[i] = {};
			pointLightsImageInfo[i].imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			pointLightsImageInfo[i].imageView = pointLightPipeline.descriptors[0].textureImages[i].views[6];
			pointLightsImageInfo[i].sampler = pointLightPipeline.descriptors[0].textureImages[i].sampler;
		}

		for (size_t i = 0; i < SPOT_LIGHTS_NUMBER; ++i) {
			spotLightsImageInfo[i] = {};
			spotLightsImageInfo[i].imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			spotLightsImageInfo[i].imageView = spotLightPipeline.descriptors[0].textureImages[i].views[0];
			spotLightsImageInfo[i].sampler = spotLightPipeline.descriptors[0].textureImages[i].sampler;
		}
		
		constexpr u32 DS_writes_size = 5;
		
		VkDescriptorImageInfo diffuseImageInfo{};
		diffuseImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		diffuseImageInfo.imageView = textureImages[diffuse_id].views[0];
		diffuseImageInfo.sampler = textureImages[diffuse_id].sampler;

		VkDescriptorImageInfo specularImageInfo{};
		specularImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		specularImageInfo.imageView = textureImages[specular_id].views[0];
		specularImageInfo.sampler = textureImages[specular_id].sampler;
		
		std::array<VkWriteDescriptorSet, DS_writes_size> descriptorWrites{};
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = diffuseSamplerDescriptorSets[currentFrame];
		descriptorWrites[0].dstBinding = diffuseCisBinding;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pImageInfo = &diffuseImageInfo;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = diffuseSamplerDescriptorSets[currentFrame];
		descriptorWrites[1].dstBinding = specularCisBinding;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &specularImageInfo;

		descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[2].dstSet = diffuseSamplerDescriptorSets[currentFrame];
		descriptorWrites[2].dstBinding = directionalLightShadowMapsCisBinding;
		descriptorWrites[2].dstArrayElement = 0;
		descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[2].descriptorCount = DIRECTIONAL_LIGHTS_NUMBER;
		descriptorWrites[2].pImageInfo = directionalLightsImageInfo;

		descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[3].dstSet = diffuseSamplerDescriptorSets[currentFrame];
		descriptorWrites[3].dstBinding = pointLightShadowMapsCisBinding;
		descriptorWrites[3].dstArrayElement = 0;
		descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[3].descriptorCount = POINT_LIGHTS_NUMBER;
		descriptorWrites[3].pImageInfo = pointLightsImageInfo;

		descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[4].dstSet = diffuseSamplerDescriptorSets[currentFrame];
		descriptorWrites[4].dstBinding = spotLightShadowMapsCisBinding;
		descriptorWrites[4].dstArrayElement = 0;
		descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[4].descriptorCount = SPOT_LIGHTS_NUMBER;
		descriptorWrites[4].pImageInfo = spotLightsImageInfo;

		vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
	
	void CVulkanRenderer::updateDirectionalLightShadowMapDescriptorSets() {
		core::vector<u32> directionalLightBindigs = directionalLightPipeline.getBindingOfDescriptor(DescriptorsTypes::DIRECTIONAL_LIGHT_SHADOW_MAP_MATRIX_UBO);
		int directionalLightShadowMapMatrixUboBinding = directionalLightBindigs[0];
		
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT * directionalLightUboDescriptorsNumber; ++i) {
				VkDescriptorBufferInfo modelMatrixBufferInfo{};
				modelMatrixBufferInfo.buffer = shadowMapDirectionalLightModelMatrixUniformBuffers[0];
				modelMatrixBufferInfo.offset = i * sizeof(ShadowMapMatrixUBO);
				modelMatrixBufferInfo.range = sizeof(ShadowMapMatrixUBO);
			
				std::array<VkWriteDescriptorSet, 1> descriptorWrites{};
			
				descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[0].dstSet = shadowMapDirectionalLightDescriptorSets[i];
				descriptorWrites[0].dstBinding = directionalLightShadowMapMatrixUboBinding;
				descriptorWrites[0].dstArrayElement = 0;
				descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrites[0].descriptorCount = 1;
				descriptorWrites[0].pBufferInfo = &modelMatrixBufferInfo;

				vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
			}
	}

	void CVulkanRenderer::updateSpotLightShadowMapDescriptorSets() {
		core::vector<u32> spotLightBindings = spotLightPipeline.getBindingOfDescriptor(DescriptorsTypes::SPOT_LIGHT_SHADOW_MAP_MATRIX_UBO);
		int spotLightShadowMapMatrixUboBinding = spotLightBindings[0];
		
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT * spotLightUboDescriptorsNumber; ++i) {
			VkDescriptorBufferInfo modelMatrixBufferInfo{};
			modelMatrixBufferInfo.buffer = shadowMapSpotLightModelMatrixUniformBuffers[0];
			modelMatrixBufferInfo.offset = i * sizeof(ShadowMapMatrixUBO);
			modelMatrixBufferInfo.range = sizeof(ShadowMapMatrixUBO);
			
			std::array<VkWriteDescriptorSet, 1> descriptorWrites{};
			
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = shadowMapSpotLightDescriptorSets[i];
			descriptorWrites[0].dstBinding = spotLightShadowMapMatrixUboBinding;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &modelMatrixBufferInfo;

			vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
	}

	void CVulkanRenderer::updatePointLightShadowMapDescriptorSets() {
		core::vector<u32> pointLightBindings = pointLightPipeline.getBindingOfDescriptor(DescriptorsTypes::POINT_LIGHT_SHADOW_MAP_MATRIX_UBO);
		int pointLightShadowMapMatrixUboBinding = pointLightBindings[0];
		
		for (size_t i = 0; i < 6 * MAX_FRAMES_IN_FLIGHT * pointLightUboDescriptorsNumber; ++i) {
			VkDescriptorBufferInfo modelMatrixBufferInfo{};
			modelMatrixBufferInfo.buffer = shadowMapPointLightModelMatrixUniformBuffers[0];
			modelMatrixBufferInfo.offset = i * sizeof(PointLightShadowMapMatrixUBO);
			modelMatrixBufferInfo.range = sizeof(PointLightShadowMapMatrixUBO);
			
			std::array<VkWriteDescriptorSet, 1> descriptorWrites{};
			
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = shadowMapPointLightDescriptorSets[i];
			descriptorWrites[0].dstBinding = pointLightShadowMapMatrixUboBinding;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &modelMatrixBufferInfo;

			vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
	}
	
    void CVulkanRenderer::updateDescriptorSets() {
		core::vector<u32> modelMatrixBindings = mainRenderScenePipeline.getBindingOfDescriptor(DescriptorsTypes::MODEL_MATRIX_UBO);
		int modelMatrixUboBinding = modelMatrixBindings[0];
		
		if ( modelMatrixUboBinding != -1 ) {
			for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT * matrixUboDescriptorsNumber; ++i) {
				VkDescriptorBufferInfo modelMatrixBufferInfo{};
				uint32_t uboIndex = 0;
				if ( i > MAX_FRAMES_IN_FLIGHT * matrixUboDescriptorsNumber / 2 )
					uboIndex = 1;
				modelMatrixBufferInfo.buffer = modelMatrixUniformBuffers[uboIndex];
				modelMatrixBufferInfo.offset = i * sizeof(ModelMatrixUBO);
				modelMatrixBufferInfo.range = sizeof(ModelMatrixUBO);
			
				std::array<VkWriteDescriptorSet, 1> descriptorWrites{};
			
				descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[0].dstSet = matrixUboDescriptorSets[i];
				descriptorWrites[0].dstBinding = modelMatrixUboBinding;
				descriptorWrites[0].dstArrayElement = 0;
				descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrites[0].descriptorCount = 1;
				descriptorWrites[0].pBufferInfo = &modelMatrixBufferInfo;

				vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
			}
		}

		core::vector<u32> specularSamplerBindigs = mainRenderScenePipeline.getBindingOfDescriptor(DescriptorsTypes::SPECULAR_SAMPLER);
 		int specularSamplerBinding = specularSamplerBindigs[0];
		u32 DS_specular_number = initializeTextureData_.size();
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT * DS_specular_number; ++i) {
			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			unsigned int textureIndex = i / 2;
			imageInfo.imageView = textureImages[textureIndex].views[0];
			imageInfo.sampler = textureImages[textureIndex].sampler;

			std::array<VkWriteDescriptorSet, 1> descriptorWrites{};
			
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = specularSamplerDescriptorSets[i];
			descriptorWrites[0].dstBinding = specularSamplerBinding;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pImageInfo = &imageInfo;

			vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}

		core::vector<u32> lightDataBindigs = mainRenderScenePipeline.getBindingOfDescriptor(DescriptorsTypes::LIGHT_DATA);
		int lightDataUboBinding = lightDataBindigs[0];
		
		if ( lightDataUboBinding != -1 ) {
			for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT * lightDataSize; ++i) {
				VkDescriptorBufferInfo modelMatrixBufferInfo{};
				modelMatrixBufferInfo.buffer = lightDataUniformBuffers[i];
				modelMatrixBufferInfo.offset = 0;
				modelMatrixBufferInfo.range = sizeof(LightData);
			
				std::array<VkWriteDescriptorSet, 1> descriptorWrites{};
			
				descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[0].dstSet = lightDataUboDescriptorSets[i];
				descriptorWrites[0].dstBinding = lightDataUboBinding;
				descriptorWrites[0].dstArrayElement = 0;
				descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrites[0].descriptorCount = 1;
				descriptorWrites[0].pBufferInfo = &modelMatrixBufferInfo;

				vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
			}
		}

		core::vector<u32> lightsSamplersBindings = mainRenderScenePipeline.getBindingOfDescriptor(DescriptorsTypes::LIGHT_SAMPLERS);
		int diffuseCisBinding = lightsSamplersBindings[0];
		int directionalLightShadowMapsCisBinding = lightsSamplersBindings[1];
		int pointLightShadowMapsCisBinding = lightsSamplersBindings[2];
		int spotLightShadowMapsCisBinding = lightsSamplersBindings[3];

		VkDescriptorImageInfo directionalLightsImageInfo[DIRECTIONAL_LIGHTS_NUMBER];
		for (size_t i = 0; i < DIRECTIONAL_LIGHTS_NUMBER; ++i) {
			directionalLightsImageInfo[i] = {};
			directionalLightsImageInfo[i].imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			directionalLightsImageInfo[i].imageView = directionalLightPipeline.descriptors[0].textureImages[i].views[0];
			directionalLightsImageInfo[i].sampler = directionalLightPipeline.descriptors[0].textureImages[i].sampler;
		}

		VkDescriptorImageInfo pointLightsImageInfo[POINT_LIGHTS_NUMBER];
		for (size_t i = 0; i < POINT_LIGHTS_NUMBER; ++i) {
			pointLightsImageInfo[i] = {};
			pointLightsImageInfo[i].imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			pointLightsImageInfo[i].imageView = pointLightPipeline.descriptors[0].textureImages[i].views[6];
			pointLightsImageInfo[i].sampler = pointLightPipeline.descriptors[0].textureImages[i].sampler;
		}

		VkDescriptorImageInfo spotLightsImageInfo[SPOT_LIGHTS_NUMBER];
		for (size_t i = 0; i < SPOT_LIGHTS_NUMBER; ++i) {
			spotLightsImageInfo[i] = {};
			spotLightsImageInfo[i].imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			spotLightsImageInfo[i].imageView = spotLightPipeline.descriptors[0].textureImages[i].views[0];
			spotLightsImageInfo[i].sampler = spotLightPipeline.descriptors[0].textureImages[i].sampler;
		}

		u32 DS_number = initializeTextureData_.size();
		constexpr u32 DS_writes_size = 4;
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT * DS_number; ++i) {
			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			unsigned int textureIndex = i / 2;
			imageInfo.imageView = textureImages[textureIndex].views[0];
			imageInfo.sampler = textureImages[textureIndex].sampler;
			
			std::array<VkWriteDescriptorSet, DS_writes_size> descriptorWrites{};
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = diffuseSamplerDescriptorSets[i];
			descriptorWrites[0].dstBinding = diffuseCisBinding;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pImageInfo = &imageInfo;

			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = diffuseSamplerDescriptorSets[i];
			descriptorWrites[1].dstBinding = directionalLightShadowMapsCisBinding;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[1].descriptorCount = DIRECTIONAL_LIGHTS_NUMBER;
			descriptorWrites[1].pImageInfo = directionalLightsImageInfo;

			descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[2].dstSet = diffuseSamplerDescriptorSets[i];
			descriptorWrites[2].dstBinding = pointLightShadowMapsCisBinding;
			descriptorWrites[2].dstArrayElement = 0;
			descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[2].descriptorCount = POINT_LIGHTS_NUMBER;
			descriptorWrites[2].pImageInfo = pointLightsImageInfo;

			descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[3].dstSet = diffuseSamplerDescriptorSets[i];
			descriptorWrites[3].dstBinding = spotLightShadowMapsCisBinding;
			descriptorWrites[3].dstArrayElement = 0;
			descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[3].descriptorCount = SPOT_LIGHTS_NUMBER;
			descriptorWrites[3].pImageInfo = spotLightsImageInfo;
			
			vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
	}
	
	void CVulkanRenderer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create buffer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

		i32 result = vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory);
        if (result != VK_SUCCESS) {
			std::cout << "result" << result << std::endl;
            throw std::runtime_error("failed to allocate buffer memory!");
        }

        vkBindBufferMemory(device, buffer, bufferMemory, 0);
    }

    VkCommandBuffer CVulkanRenderer::beginSingleTimeCommands(VkCommandPool& commandPool) {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }

    void CVulkanRenderer::endSingleTimeCommands(VkCommandPool& commandPool, VkCommandBuffer& commandBuffer) {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(graphicsQueue);

        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
    }

    void CVulkanRenderer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands(mainRenderCommandPool);

        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        endSingleTimeCommands(mainRenderCommandPool, commandBuffer);
    }

    uint32_t CVulkanRenderer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties && memProperties.memoryTypes[i].heapIndex == 0) {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }

    void CVulkanRenderer::createCommandBuffers(VkCommandPool& commandPool, std::vector<VkCommandBuffer>& commandBuffers) {
        commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();

        if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    void CVulkanRenderer::recordCommandBuffer(VkCommandBuffer& commandBuffer, uint32_t imageIndex) {
		ecs::ComponentManager* componentManager  = ecs::ComponentManager::GetInstance();
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

		namespace cm = GLVM::ecs::components;
		core::vector<Entity> linkedEntities      = componentManager->collectLinkedEntities<cm::transform,
																						   cm::material,
																						   cm::mesh>();
		
		CreateEndDebugUtilsLabelEXT(instance, commandBuffer);
		
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent.height = swapChainExtent.height;
		renderPassInfo.renderArea.extent.width = swapChainExtent.width;

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {{0.2f, 0.2f, 0.2f, 1.0f}};
        clearValues[1].depthStencil = {1.0f, 0};

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mainRenderScenePipeline.pipeline);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float) swapChainExtent.width;
        viewport.height = (float) swapChainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = swapChainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		core::vector<Entity> viewPositionLinkedEntities = componentManager->collectLinkedEntities<cm::beholder>();

		cm::transform* playerTransformComponent = nullptr;

		if ( viewPositionLinkedEntities.GetSize() > 0 )
			playerTransformComponent = componentManager->GetComponent<cm::transform>(viewPositionLinkedEntities[0]);

		for ( unsigned int i = 0; i < linkedEntities.GetSize(); ++i ) {
			unsigned int uiEntity = linkedEntities[i];
			unsigned int uiVertexId = componentManager->GetComponent<ecs::components::mesh>(uiEntity)->handle.id;
			cm::transform* transformComponent = componentManager->GetComponent<cm::transform>(uiEntity);
			unsigned int diffuseTextureIndex = componentManager->GetComponent<cm::material>(uiEntity)->diffuseTextureID_.id;
			unsigned int specularTextureIndex = componentManager->GetComponent<cm::material>(uiEntity)->specularTextureID_.id;
			cm::material* materialComponent = componentManager->GetComponent<cm::material>(uiEntity);
				
			unsigned int uboIndex = i;
			updateMatrixUniformBuffer(currentFrame, uboIndex, transformComponent, uiVertexId, materialComponent);
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mainRenderScenePipeline.pipelineLayout,
									0, 1, &matrixUboDescriptorSets[uboIndex], 0, nullptr);

			updateViewPositionUniformBuffer(currentFrame, playerTransformComponent);
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mainRenderScenePipeline.pipelineLayout,
									1, 1, &lightDataUboDescriptorSets[currentFrame], 0, nullptr);

			VkBuffer vertexBuffers[] = {vertexBufferContainer[uiVertexId]};
			VkDeviceSize offsets[] = {0};
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

			vkCmdBindIndexBuffer(commandBuffer, indexBufferContainer[uiVertexId], 0, VK_INDEX_TYPE_UINT32);

			unsigned int indicesContainerSize = aIndices_[uiVertexId].size();

//			updateSamplersDescriptroSets(diffuseTextureIndex, specularTextureIndex);
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mainRenderScenePipeline.pipelineLayout, 2, 1,
									&specularSamplerDescriptorSets[MAX_FRAMES_IN_FLIGHT * specularTextureIndex + currentFrame], 0, nullptr);
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mainRenderScenePipeline.pipelineLayout, 3, 1,
									&diffuseSamplerDescriptorSets[MAX_FRAMES_IN_FLIGHT * diffuseTextureIndex + currentFrame], 0, nullptr);
			
			vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indicesContainerSize), 1, 0, 0, 0);
		}

        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

    void CVulkanRenderer::createSyncObjects(std::vector<VkSemaphore>& imageAvailableSemaphores,
											std::vector<VkSemaphore>& renderFinishedSemaphores,
											std::vector<VkFence>& inFlightFences) {
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
		
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		
        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    }

	void CVulkanRenderer::updateDirectionalLightSpaceMatrixShadowMapUBO(ecs::components::directionalLight* directionalLightComponent,
																		uint32_t currentLight) {
		float nearPlaneFlatShadowMap = 1.5f;
		float farPlaneFlatShadowMap = 100.0f;
		mat4 directionalProjectionMatrixLight = ortho(-15.0f, 15.0f, -15.0f, 15.0f,
													  nearPlaneFlatShadowMap, farPlaneFlatShadowMap);

		vec3 positionVectorLight = directionalLightComponent->position;
		vec3 directionVectorLight = directionalLightComponent->direction;

		mat4 viewMatrixLight = LookAtMain(positionVectorLight,
										  directionVectorLight,
										  { 0.0f, 1.0f, 0.0f });

		directionalProjectionMatrixLight[1][1] *= -1;
		dirLightSpaceMatrix[currentLight] = viewMatrixLight * directionalProjectionMatrixLight;
	}
	
    void CVulkanRenderer::updateDirectionalLightShadowMapMatrixUBO(uint32_t currentImage, [[maybe_unused]] ecs::components::transform* _transformComponent,
																   [[maybe_unused]] uint32_t currentLight, [[maybe_unused]] u32 meshID) {
		ShadowMapMatrixUBO modelMatrixUBO{};

        modelMatrixUBO.model = computeModelMatrix(_transformComponent);
		modelMatrixUBO.lightSpaceMatrix = dirLightSpaceMatrix[currentLight];

		mat4* jointMatricesData = updateAnimationFrames(_transformComponent, meshID);

		for ( unsigned int j = 0; j < MAX_JOINTS_NUMBER; ++j ) {
			modelMatrixUBO.jointMatrices[j] = jointMatricesData[j];
		}

		delete [] jointMatricesData;
		jointMatricesData = nullptr;
		
        void* modelMatrixData = nullptr;
        vkMapMemory(device, shadowMapDirectionalLightModelMatrixUniformBuffersMemory[0], currentImage * sizeof(modelMatrixUBO),
					sizeof(modelMatrixUBO), 0, &modelMatrixData);
        memcpy(modelMatrixData, &modelMatrixUBO, sizeof(modelMatrixUBO));
        vkUnmapMemory(device, shadowMapDirectionalLightModelMatrixUniformBuffersMemory[0]);
    }

	void CVulkanRenderer::updateSpotLightSpaceMatrixShadowMapUBO(ecs::components::spotLight* spotLightComponent,
																		uint32_t currentLight) {
		float nearPlaneFlatShadowMap = 1.0f;
		float farPlaneFlatShadowMap = 100.0f;
		mat4 spotProjectionMatrixLight = Perspective(Radians(90.0f), (float)SHADOW_MAP_SIZE / (float)SHADOW_MAP_SIZE,
														 nearPlaneFlatShadowMap, farPlaneFlatShadowMap);
		
		vec3 positionVectorLight  = spotLightComponent->position;
		vec3 directionVectorLight = spotLightComponent->direction;
		mat4 viewMatrixLight = LookAtMain(positionVectorLight,
										  directionVectorLight,
										  { 0.0f, 1.0f, 0.0f });

		spotProjectionMatrixLight[1][1] *= -1;
		spotLightSpaceMatrix[currentLight] = viewMatrixLight * spotProjectionMatrixLight;
	}
	
    void CVulkanRenderer::updateSpotLightShadowMapMatrixUBO(uint32_t currentImage, [[maybe_unused]] ecs::components::transform* _transformComponent, [[maybe_unused]] uint32_t currentLight, [[maybe_unused]] u32 meshID) {
		ShadowMapMatrixUBO modelMatrixUBO{};
		
        modelMatrixUBO.model = computeModelMatrix(_transformComponent);
		modelMatrixUBO.lightSpaceMatrix = spotLightSpaceMatrix[currentLight];

		mat4* jointMatricesData = updateAnimationFrames(_transformComponent, meshID);
		
		for ( unsigned int j = 0; j < MAX_JOINTS_NUMBER; ++j ) {
			modelMatrixUBO.jointMatrices[j] = jointMatricesData[j];
		}

		delete [] jointMatricesData;
		jointMatricesData = nullptr;
		
        void* modelMatrixData;
        vkMapMemory(device, shadowMapSpotLightModelMatrixUniformBuffersMemory[0], currentImage * sizeof(modelMatrixUBO),
					sizeof(modelMatrixUBO), 0, &modelMatrixData);
        memcpy(modelMatrixData, &modelMatrixUBO, sizeof(modelMatrixUBO));
        vkUnmapMemory(device, shadowMapSpotLightModelMatrixUniformBuffersMemory[0]);
    }

    void CVulkanRenderer::updatePointLightShadowMapMatrixUBO([[maybe_unused]] uint32_t currentImage, ecs::components::transform* _transformComponent, ecs::components::pointLight* pointLightComponent, uint32_t layer, unsigned int meshID) {
		PointLightShadowMapMatrixUBO modelMatrixUBO{};

		vec3 positionVectorLight  = pointLightComponent->position;
		vec3 directionalVectorLight = vec3(0.0f, 0.0f, 0.0f);
		vec3 upVector = { 0.0, 0.0, 0.0 };

		switch(layer) {
		case 0:
			/// Positive X
			directionalVectorLight = positionVectorLight + vec3( 1.0f,  0.0f, 0.0f);
			upVector = vec3(0.0f, -1.0f,  0.0f);
			break;
		case 1:
			/// Negative X
			directionalVectorLight = positionVectorLight + vec3( -1.0f,  0.0f,  0.0f);
			upVector = vec3(0.0f, -1.0f,  0.0f);
			break;
		case 2:
			/// Positive Y
			directionalVectorLight = positionVectorLight + vec3( 0.0f,  1.0f,  0.0f);
			upVector = vec3(0.0f, 0.0f,  1.0f);
			break;
		case 3:
			/// Negative Y
			directionalVectorLight = positionVectorLight + vec3( 0.0f,  -1.0f,  0.0f);
			upVector = vec3(0.0f, 0.0f,  -1.0f);
			break;
		case 4:
			/// Positive Z
			directionalVectorLight = positionVectorLight + vec3( 0.0f,  0.0f,  1.0f);
			upVector = vec3(0.0f, -1.0f,  0.0f);
			break;
			/// Negative Z
		case 5:
			directionalVectorLight = positionVectorLight + vec3( 0.0f,  0.0f,  -1.0f);
			upVector = vec3(0.0f, -1.0f,  0.0f);
			break;
		default:
			break;
		}
		
		mat4 projectionMatrixCubeShadowMap = Perspective(Radians(90.0f), (float)SHADOW_MAP_SIZE / (float)SHADOW_MAP_SIZE, 0.3f, 100.0f);

		mat4 viewMatrixLight = LookAtMain(positionVectorLight,
										  directionalVectorLight,
										  upVector);

        modelMatrixUBO.model = computeModelMatrix(_transformComponent);
		
//		projectionMatrixCubeShadowMap[1][1] *= -1;
		
		modelMatrixUBO.lightSpaceMatrix = viewMatrixLight * projectionMatrixCubeShadowMap;
		modelMatrixUBO.farPlane = 100.0f;
		modelMatrixUBO.lightPosition = positionVectorLight;

		mat4* jointMatricesData = updateAnimationFrames(_transformComponent, meshID);
		
		for ( unsigned int j = 0; j < MAX_JOINTS_NUMBER; ++j ) {
			modelMatrixUBO.jointMatrices[j] = jointMatricesData[j];
		}
		
		delete [] jointMatricesData;
		jointMatricesData = nullptr;
		
        void* modelMatrixData;
        vkMapMemory(device, shadowMapPointLightModelMatrixUniformBuffersMemory[0], currentImage * sizeof(modelMatrixUBO),
					sizeof(modelMatrixUBO), 0, &modelMatrixData);
        memcpy(modelMatrixData, &modelMatrixUBO, sizeof(modelMatrixUBO));
        vkUnmapMemory(device, shadowMapPointLightModelMatrixUniformBuffersMemory[0]);
    }

    void CVulkanRenderer::updatePointLightShadowMapDataUBO(uint32_t currentImage, ecs::components::pointLight* pointLightComponent, float farPlane) {
		UniformBufferObjectLightUBO dataMatrixUBO{};

		dataMatrixUBO.lightPosition = pointLightComponent->position;
		dataMatrixUBO.farPlane = farPlane;
		
        void* dataMatrixData;
        vkMapMemory(device, shadowMapPointLightDataUniformBuffersMemory[currentImage], 0,
					sizeof(dataMatrixUBO), 0, &dataMatrixData);
        memcpy(dataMatrixData, &dataMatrixUBO, sizeof(dataMatrixUBO));
        vkUnmapMemory(device, shadowMapPointLightDataUniformBuffersMemory[currentImage]);
	}
	
    void CVulkanRenderer::updateMatrixUniformBuffer(uint32_t currentImage, uint32_t offset, ecs::components::transform* _transformComponent,
													unsigned int meshID, ecs::components::material* materialComponent) {
        ModelMatrixUBO modelMatrixUBO{};
		
        modelMatrixUBO.model = computeModelMatrix(_transformComponent);
		
        modelMatrixUBO.view = viewMatrix;
        modelMatrixUBO.proj = projectionMatrix;

		/// Start of animation logic
		mat4* jointMatricesData = updateAnimationFrames(_transformComponent, meshID);

		for ( unsigned int j = 0; j < MAX_JOINTS_NUMBER; ++j ) {
			modelMatrixUBO.jointMatrices[j] = jointMatricesData[j];
		}
		
		delete [] jointMatricesData;
		jointMatricesData = nullptr;
		/// End of animation logic

		modelMatrixUBO.ambient = materialComponent->ambient;
		modelMatrixUBO.shininess = materialComponent->shininess;

		for ( uint32_t i = 0; i < directionalLightNumber; ++i )
			modelMatrixUBO.dirSpaceMatrix[i] = dirLightSpaceMatrix[i];

		for ( uint32_t i = 0; i < spotLightNumber; ++i )
			modelMatrixUBO.spotSpaceMatrix[i] = spotLightSpaceMatrix[i];
		
		modelMatrixUBO.directionalLightsNumber = directionalLightNumber;
		modelMatrixUBO.spotLightsNumber        = spotLightNumber;
		
        void* modelMatrixData;
        vkMapMemory(device, modelMatrixUniformBuffersMemory[currentImage], sizeof(modelMatrixUBO) * offset,
					sizeof(modelMatrixUBO), 0, &modelMatrixData);
        memcpy(modelMatrixData, &modelMatrixUBO, sizeof(modelMatrixUBO));
        vkUnmapMemory(device, modelMatrixUniformBuffersMemory[currentImage]);
    }

	void CVulkanRenderer::updateViewPositionUniformBuffer(uint32_t currentImage, ecs::components::transform* transformComponent) {
		namespace cm = GLVM::ecs::components;
		ecs::ComponentManager* componentManager  = ecs::ComponentManager::GetInstance();
		LightData lightDataUBO{};
		core::vector<Entity> pointLightEntities = componentManager->collectLinkedEntities<GLVM::ecs::components::controller>();
		if ( pointLightEntities.GetSize() > 0 )

			lightDataUBO.viewPosition = transformComponent->tPosition;

		DirectionalLight directionalLight{};

		core::vector<Entity> linkedEntitiesDirLight      = componentManager->collectLinkedEntities<cm::transform,
																						   cm::directionalLight,
																						   cm::mesh>();

		directionalLightNumber = linkedEntitiesDirLight.GetSize();
		assert(directionalLightNumber <= 4 && "Directional lights number greater then 4");

		for ( unsigned int i = 0; i < directionalLightNumber; ++i ) {
			cm::directionalLight* directionalLightComponent = componentManager->GetComponent<cm::directionalLight>(linkedEntitiesDirLight[i]);
			
			directionalLight.position  = vec4(directionalLightComponent->position[0],
											  directionalLightComponent->position[1],
											  directionalLightComponent->position[2], 0.0f);
			directionalLight.direction = vec4(directionalLightComponent->direction[0],
											  directionalLightComponent->direction[1],
											  directionalLightComponent->direction[2], 0.0f);
			directionalLight.ambient   = vec4(directionalLightComponent->ambient[0],
											  directionalLightComponent->ambient[1],
											  directionalLightComponent->ambient[2], 0.0f);
			directionalLight.diffuse   = vec4(directionalLightComponent->diffuse[0],
											  directionalLightComponent->diffuse[1],
											  directionalLightComponent->diffuse[2], 0.0f);
			directionalLight.specular  = vec4(directionalLightComponent->specular[0],
											  directionalLightComponent->specular[1],
											  directionalLightComponent->specular[2], 0.0f);

			lightDataUBO.directionalLights[i] = directionalLight;			
		}

		lightDataUBO.directionalLightsArraySize = directionalLightNumber;

		core::vector<Entity> linkedEntitiesPointLight      = componentManager->collectLinkedEntities<cm::transform,
																						   cm::pointLight,
																						   cm::mesh>();

		pointLightNumber = linkedEntitiesPointLight.GetSize();
		assert(pointLightNumber <= POINT_LIGHTS_NUMBER && "Point lights number greater than 32");
		for ( unsigned int i = 0; i < pointLightNumber; ++i ) {
			cm::pointLight* pointLightComponent = componentManager->GetComponent<cm::pointLight>(linkedEntitiesPointLight[i]);
			PointLight pointLightUBO{};

 			pointLightUBO.position  = vec3(pointLightComponent->position[0],
										   pointLightComponent->position[1],
										   pointLightComponent->position[2]);
			pointLightUBO.ambient   = vec3(pointLightComponent->ambient[0],
										   pointLightComponent->ambient[1],
										   pointLightComponent->ambient[2]);
			pointLightUBO.diffuse   = vec3(pointLightComponent->diffuse[0],
										   pointLightComponent->diffuse[1],
										   pointLightComponent->diffuse[2]);
			pointLightUBO.specular  = pointLightComponent->specular;

			pointLightUBO.constant  = pointLightComponent->constant;

			pointLightUBO.linear    = pointLightComponent->linear;

			pointLightUBO.quadratic = pointLightComponent->quadratic;

			lightDataUBO.pointLights[i] = pointLightUBO;
		}
		
		lightDataUBO.pointLightsArraySize = pointLightNumber;
		lightDataUBO.farPlane = 100.0f;

		SpotLight spotLight{};
		core::vector<Entity> linkedEntitiesSpotLight      = componentManager->collectLinkedEntities<cm::transform,
																						   cm::spotLight,
																						   cm::mesh>();

		spotLightNumber = linkedEntitiesSpotLight.GetSize();
		assert(spotLightNumber <= 8 && "Spot light number greater then 8");
		for ( unsigned int i = 0; i < spotLightNumber; ++i ) {
			cm::spotLight* spotLightComponent = componentManager->GetComponent<cm::spotLight>(linkedEntitiesSpotLight[i]);
			
			spotLight.position    = spotLightComponent->position;
			spotLight.direction   = spotLightComponent->direction;
			spotLight.cutOff      = std::cos(Radians(spotLightComponent->cutOff));
			spotLight.outerCutOff = std::cos(Radians(spotLightComponent->outerCutOff));
			spotLight.ambient     = spotLightComponent->ambient;
			spotLight.diffuse     = spotLightComponent->diffuse;
			spotLight.specular    = spotLightComponent->specular;
			spotLight.constant    = spotLightComponent->constant;
			spotLight.linear      = spotLightComponent->linear;
			spotLight.quadratic   = spotLightComponent->quadratic; 

			lightDataUBO.spotLights[i] = spotLight;
		}

		lightDataUBO.spotLightArraySize = spotLightNumber;

        void* data;
        vkMapMemory(device, lightDataUniformBuffersMemory[currentImage], 0,
					sizeof(lightDataUBO), 0, &data);
        memcpy(data, &lightDataUBO, sizeof(lightDataUBO));
        vkUnmapMemory(device, lightDataUniformBuffersMemory[currentImage]);
	}

	void CVulkanRenderer::updateDirSpaceMatrix(uint32_t currentImage) {
		LightSpaceMatrixUBO lightUBO{};
		for ( uint32_t i = 0; i < directionalLightNumber; ++i )
			lightUBO.dirSpaceMatrix[i] = dirLightSpaceMatrix[i];

		for ( uint32_t i = 0; i < spotLightNumber; ++i )
			lightUBO.spotSpaceMatrix[i] = spotLightSpaceMatrix[i];
		
		lightUBO.directionalLightsNumber = directionalLightNumber;
		lightUBO.spotLightsNumber        = spotLightNumber;
		
        void* data;
        vkMapMemory(device, lightSpaceMatrixMemory[currentImage], 0,
					sizeof(LightSpaceMatrixUBO), 0, &data);
        memcpy(data, &lightUBO, sizeof(LightSpaceMatrixUBO));
        vkUnmapMemory(device, lightSpaceMatrixMemory[currentImage]);
	}

    void CVulkanRenderer::mainRenderDrawFrame() {
		namespace cm = GLVM::ecs::components;
		// mutex0.lock();
		// mutex0.unlock();
		// mutex1.lock();
		// mutex1.unlock();
		// mutex2.lock();
		// mutex2.unlock();
        vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        vkResetFences(device, 1, &inFlightFences[currentFrame]);
        vkResetCommandBuffer(mainRenderCommandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
        recordCommandBuffer(mainRenderCommandBuffers[currentFrame], imageIndex);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &mainRenderCommandBuffers[currentFrame];

        VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = {swapChain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &imageIndex;

        result = vkQueuePresentKHR(presentQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
            framebufferResized = false;
            recreateSwapChain();
        } else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void CVulkanRenderer::directionalLightShadowMapDrawFrame() {
		namespace cm = GLVM::ecs::components;
//		shadowMapPassesMutex.lock();
        vkWaitForFences(device, 1, &directionalLightShadowMapInFlightFences[directionalLightCurrentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex = 0;
        // VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, directionalLightShadowMapImageAvailableSemaphores[directionalLightCurrentFrame], VK_NULL_HANDLE, &imageIndex);

        // if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        //     recreateSwapChain();
        //     return;
        // } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        //     throw std::runtime_error("failed to acquire swap chain image!");
        // }

        vkResetFences(device, 1, &directionalLightShadowMapInFlightFences[directionalLightCurrentFrame]);
        vkResetCommandBuffer(directionalLightCommandBuffers[directionalLightCurrentFrame], /*VkCommandBufferResetFlagBits*/ 0);
        directionalLightRecordCoomandBuffer(directionalLightCommandBuffers[directionalLightCurrentFrame], imageIndex);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        // VkSemaphore waitSemaphores[] = {directionalLightShadowMapImageAvailableSemaphores[directionalLightCurrentFrame]};
        // VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        // submitInfo.waitSemaphoreCount = 1;
        // submitInfo.pWaitSemaphores = waitSemaphores;
        // submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &directionalLightCommandBuffers[directionalLightCurrentFrame];

        // VkSemaphore signalSemaphores[] = {directionalLightShadowMapRenderFinishedSemaphores[directionalLightCurrentFrame]};
        // submitInfo.signalSemaphoreCount = 1;
        // submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, directionalLightShadowMapInFlightFences[directionalLightCurrentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        // VkPresentInfoKHR presentInfo{};
        // presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        // presentInfo.waitSemaphoreCount = 1;
        // presentInfo.pWaitSemaphores = signalSemaphores;

        // VkSwapchainKHR swapChains[] = {swapChain};
        // presentInfo.swapchainCount = 1;
        // presentInfo.pSwapchains = swapChains;

        // presentInfo.pImageIndices = &imageIndex;

        // result = vkQueuePresentKHR(presentQueue, &presentInfo);

        // if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
        //     framebufferResized = false;
        //     recreateSwapChain();
        // } else if (result != VK_SUCCESS) {
        //     throw std::runtime_error("failed to present swap chain image!");
        // }

        directionalLightCurrentFrame = (directionalLightCurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
		// shadowMapPassesMutex.unlock();
		// mutex0.unlock();
    }

	void CVulkanRenderer::spotLightShadowMapDrawFrame() {
		namespace cm = GLVM::ecs::components;
//		shadowMapPassesMutex.lock();
        vkWaitForFences(device, 1, &spotLightShadowMapInFlightFences[spotLightCurrentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex = 0;
        // VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, spotLightShadowMapImageAvailableSemaphores[spotLightCurrentFrame], VK_NULL_HANDLE, &imageIndex);

        // if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        //     recreateSwapChain();
        //     return;
        // } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        //     throw std::runtime_error("failed to acquire swap chain image!");
        // }

        vkResetFences(device, 1, &spotLightShadowMapInFlightFences[spotLightCurrentFrame]);
        vkResetCommandBuffer(spotLightCommandBuffers[spotLightCurrentFrame], /*VkCommandBufferResetFlagBits*/ 0);
        spotLightRecordCommandBuffer(spotLightCommandBuffers[spotLightCurrentFrame], imageIndex);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        // VkSemaphore waitSemaphores[] = {spotLightShadowMapImageAvailableSemaphores[spotLightCurrentFrame]};
        // VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        // submitInfo.waitSemaphoreCount = 1;
        // submitInfo.pWaitSemaphores = waitSemaphores;
        // submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &spotLightCommandBuffers[spotLightCurrentFrame];

        // VkSemaphore signalSemaphores[] = {spotLightShadowMapRenderFinishedSemaphores[spotLightCurrentFrame]};
        // submitInfo.signalSemaphoreCount = 1;
        // submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, spotLightShadowMapInFlightFences[spotLightCurrentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        // VkPresentInfoKHR presentInfo{};
        // presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        // presentInfo.waitSemaphoreCount = 1;
        // presentInfo.pWaitSemaphores = signalSemaphores;

        // VkSwapchainKHR swapChains[] = {swapChain};
        // presentInfo.swapchainCount = 1;
        // presentInfo.pSwapchains = swapChains;

        // presentInfo.pImageIndices = &imageIndex;

        // result = vkQueuePresentKHR(presentQueue, &presentInfo);

        // if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
        //     framebufferResized = false;
        //     recreateSwapChain();
        // } else if (result != VK_SUCCESS) {
        //     throw std::runtime_error("failed to present swap chain image!");
        // }

        spotLightCurrentFrame = (spotLightCurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
		// shadowMapPassesMutex.unlock();
		// mutex1.unlock();
    }

	    void CVulkanRenderer::pointLightShadowMapDrawFrame() {
		namespace cm = GLVM::ecs::components;
//		shadowMapPassesMutex.lock();
        vkWaitForFences(device, 1, &pointLightShadowMapInFlightFences[pointLightCurrentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex = 0;
        // VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, pointLightShadowMapImageAvailableSemaphores[pointLightCurrentFrame], VK_NULL_HANDLE, &imageIndex);

        // if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        //     recreateSwapChain();
        //     return;
        // } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        //     throw std::runtime_error("failed to acquire swap chain image!");
        // }

        vkResetFences(device, 1, &pointLightShadowMapInFlightFences[pointLightCurrentFrame]);
        vkResetCommandBuffer(pointLightCommandBuffers[pointLightCurrentFrame], /*VkCommandBufferResetFlagBits*/ 0);
        pointLightRecordCommandBuffer(pointLightCommandBuffers[pointLightCurrentFrame], imageIndex);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        // VkSemaphore waitSemaphores[] = {pointLightShadowMapImageAvailableSemaphores[pointLightCurrentFrame]};
        // VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        // submitInfo.waitSemaphoreCount = 1;
        // submitInfo.pWaitSemaphores = waitSemaphores;
        // submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &pointLightCommandBuffers[pointLightCurrentFrame];

        // VkSemaphore signalSemaphores[] = {pointLightShadowMapRenderFinishedSemaphores[pointLightCurrentFrame]};
        // submitInfo.signalSemaphoreCount = 1;
        // submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, pointLightShadowMapInFlightFences[pointLightCurrentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        // VkPresentInfoKHR presentInfo{};
        // presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        // presentInfo.waitSemaphoreCount = 1;
        // presentInfo.pWaitSemaphores = signalSemaphores;

        // VkSwapchainKHR swapChains[] = {swapChain};
        // presentInfo.swapchainCount = 1;
        // presentInfo.pSwapchains = swapChains;

        // presentInfo.pImageIndices = &imageIndex;

        // result = vkQueuePresentKHR(presentQueue, &presentInfo);

        // if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
        //     framebufferResized = false;
        //     recreateSwapChain();
        // } else if (result != VK_SUCCESS) {
        //     throw std::runtime_error("failed to present swap chain image!");
        // }

        pointLightCurrentFrame = (pointLightCurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
		// shadowMapPassesMutex.unlock();
		// mutex2.unlock();
    }
	
	void CVulkanRenderer::directionalLightRecordCoomandBuffer(VkCommandBuffer& commandBuffer, [[maybe_unused]] uint32_t imageIndex) {
		ecs::ComponentManager* componentManager  = ecs::ComponentManager::GetInstance();
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

		
		namespace cm = GLVM::ecs::components;
		core::vector<Entity> directionalLightEntities      = componentManager->collectLinkedEntities<cm::transform,
																									 cm::directionalLight,
																									 cm::mesh>();

		core::vector<Entity> linkedEntities      = componentManager->collectLinkedEntities<cm::transform,
																						   cm::material,
																						   cm::mesh>();

		for ( uint32_t directionalLightCounter = 0; directionalLightCounter < directionalLightEntities.GetSize(); ++ directionalLightCounter ) {
			VkClearValue shadowMapClearValues[1];
			shadowMapClearValues[0].depthStencil.depth = 1.0f;
			shadowMapClearValues[0].depthStencil.stencil = 0;

			VkRenderPassBeginInfo shadowMapRenderPassInfo{};
			shadowMapRenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			shadowMapRenderPassInfo.pNext = NULL;
			shadowMapRenderPassInfo.renderPass = directionalLightShadowMapRenderPass;
			shadowMapRenderPassInfo.framebuffer = directionalLightShadowMapFrameBuffers[directionalLightCounter];
			shadowMapRenderPassInfo.renderArea.offset.x = 0;
			shadowMapRenderPassInfo.renderArea.offset.y = 0;
			shadowMapRenderPassInfo.renderArea.extent.width = swapChainExtent.width;
			shadowMapRenderPassInfo.renderArea.extent.height = swapChainExtent.height;
			shadowMapRenderPassInfo.clearValueCount = 1;
			shadowMapRenderPassInfo.pClearValues = shadowMapClearValues;

			vkCmdBeginRenderPass(commandBuffer, &shadowMapRenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			VkViewport shadowMapViewPort;
			shadowMapViewPort.height = swapChainExtent.height;
			shadowMapViewPort.width = swapChainExtent.width;
			shadowMapViewPort.minDepth = 0.0f;
			shadowMapViewPort.maxDepth = 1.0f;
			shadowMapViewPort.x = 0;
			shadowMapViewPort.y = 0;
			vkCmdSetViewport(commandBuffer, 0, 1, &shadowMapViewPort);

			VkRect2D shadowMapScissor;
			shadowMapScissor.extent.width = swapChainExtent.width;
			shadowMapScissor.extent.height = swapChainExtent.height;
			shadowMapScissor.offset.x = 0;
			shadowMapScissor.offset.y = 0;
			vkCmdSetScissor(commandBuffer, 0, 1, &shadowMapScissor);

			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, directionalLightPipeline.pipeline);

			unsigned int directionalLightEntity = directionalLightEntities[directionalLightCounter];
			cm::directionalLight* directionalLightComponent = componentManager->GetComponent<cm::directionalLight>(directionalLightEntity);
			updateDirectionalLightSpaceMatrixShadowMapUBO(directionalLightComponent, directionalLightCounter);

			uint32_t actorsNumber = linkedEntities.GetSize();
			for ( unsigned int actorCounter = 0; actorCounter < actorsNumber; ++actorCounter ) {
				unsigned int meshOwnerEntity = linkedEntities[actorCounter];
				unsigned int meshId = componentManager->GetComponent<ecs::components::mesh>(meshOwnerEntity)->handle.id;
				cm::transform* meshOwnerTransformComponent = componentManager->GetComponent<cm::transform>(meshOwnerEntity);

				unsigned int uboDirectionalLightIndex = directionalLightNumber * actorsNumber * directionalLightCurrentFrame +
					actorsNumber * directionalLightCounter + actorCounter;

				updateDirectionalLightShadowMapMatrixUBO(uboDirectionalLightIndex, meshOwnerTransformComponent, directionalLightCounter, meshId);
				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, directionalLightPipeline.pipelineLayout, 0, 1, &shadowMapDirectionalLightDescriptorSets[uboDirectionalLightIndex], 0, nullptr);
				
				VkBuffer vertexBuffers[] = {vertexBufferContainer[meshId]};
				VkDeviceSize offsets[] = {0};
				vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

				vkCmdBindIndexBuffer(commandBuffer, indexBufferContainer[meshId], 0, VK_INDEX_TYPE_UINT32);

				unsigned int indicesContainerSize = aIndices_[meshId].size();
				vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indicesContainerSize), 1, 0, 0, 0);
			}
		
			vkCmdEndRenderPass(commandBuffer);
		}

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
	}

	void CVulkanRenderer::spotLightRecordCommandBuffer(VkCommandBuffer& commandBuffer, [[maybe_unused]] uint32_t imageIndex) {
		ecs::ComponentManager* componentManager  = ecs::ComponentManager::GetInstance();
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

		namespace cm = GLVM::ecs::components;
		core::vector<Entity> linkedEntities      = componentManager->collectLinkedEntities<cm::transform,
																						   cm::material,
																						   cm::mesh>();

		
		core::vector<Entity> spotLightEntities      = componentManager->collectLinkedEntities<cm::transform,
																							  cm::spotLight,
																							  cm::mesh>();
			
		for ( uint32_t spotLightCounter = 0; spotLightCounter < spotLightEntities.GetSize(); ++ spotLightCounter ) {
			VkClearValue spotLightShadowMapClearValues[1];
			spotLightShadowMapClearValues[0].depthStencil.depth = 1.0f;
			spotLightShadowMapClearValues[0].depthStencil.stencil = 0;

			VkRenderPassBeginInfo spotLightShadowMapRenderPassInfo{};
			spotLightShadowMapRenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			spotLightShadowMapRenderPassInfo.pNext = NULL;
			spotLightShadowMapRenderPassInfo.renderPass = spotLightShadowMapRenderPass;
			spotLightShadowMapRenderPassInfo.framebuffer = spotLightShadowMapFrameBuffers[spotLightCounter];
			spotLightShadowMapRenderPassInfo.renderArea.offset.x = 0;
			spotLightShadowMapRenderPassInfo.renderArea.offset.y = 0;
			spotLightShadowMapRenderPassInfo.renderArea.extent.width = swapChainExtent.width;
			spotLightShadowMapRenderPassInfo.renderArea.extent.height = swapChainExtent.height;
			spotLightShadowMapRenderPassInfo.clearValueCount = 1;
			spotLightShadowMapRenderPassInfo.pClearValues = spotLightShadowMapClearValues;

			vkCmdBeginRenderPass(commandBuffer, &spotLightShadowMapRenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			VkViewport spotLightShadowMapViewPort;
			spotLightShadowMapViewPort.height = swapChainExtent.height;
			spotLightShadowMapViewPort.width = swapChainExtent.width;
			spotLightShadowMapViewPort.minDepth = 0.0f;
			spotLightShadowMapViewPort.maxDepth = 1.0f;
			spotLightShadowMapViewPort.x = 0;
			spotLightShadowMapViewPort.y = 0;
			vkCmdSetViewport(commandBuffer, 0, 1, &spotLightShadowMapViewPort);

			VkRect2D spotLightShadowMapScissor;
			spotLightShadowMapScissor.extent.width = swapChainExtent.width;
			spotLightShadowMapScissor.extent.height = swapChainExtent.height;
			spotLightShadowMapScissor.offset.x = 0;
			spotLightShadowMapScissor.offset.y = 0;
			vkCmdSetScissor(commandBuffer, 0, 1, &spotLightShadowMapScissor);

			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, spotLightPipeline.pipeline);

			unsigned int spotLightEntity = spotLightEntities[spotLightCounter];
			cm::spotLight* spotLightComponent = componentManager->GetComponent<cm::spotLight>(spotLightEntity);
			updateSpotLightSpaceMatrixShadowMapUBO(spotLightComponent, spotLightCounter);

			uint32_t actorsNumber = linkedEntities.GetSize();
			for ( unsigned int actorsCounter = 0; actorsCounter < actorsNumber; ++actorsCounter ) {
				unsigned int meshOwnerEntity = linkedEntities[actorsCounter];
				unsigned int meshID = componentManager->GetComponent<ecs::components::mesh>(meshOwnerEntity)->handle.id;
				cm::transform* meshOwnerTransformComponent = componentManager->GetComponent<cm::transform>(meshOwnerEntity);
				unsigned int uboSpotLightIndex = spotLightNumber * actorsNumber * spotLightCurrentFrame +
					actorsNumber * spotLightCounter + actorsCounter;

				updateSpotLightShadowMapMatrixUBO(uboSpotLightIndex, meshOwnerTransformComponent, spotLightCounter, meshID);
				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, spotLightPipeline.pipelineLayout, 0, 1, &shadowMapSpotLightDescriptorSets[uboSpotLightIndex], 0, nullptr);
				VkBuffer vertexBuffers[] = {vertexBufferContainer[meshID]};
				VkDeviceSize offsets[] = {0};
				vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

				vkCmdBindIndexBuffer(commandBuffer, indexBufferContainer[meshID], 0, VK_INDEX_TYPE_UINT32);

				unsigned int indicesContainerSize = aIndices_[meshID].size();
				vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indicesContainerSize), 1, 0, 0, 0);
			}
		
			vkCmdEndRenderPass(commandBuffer);
		}

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
	}

	void CVulkanRenderer::pointLightRecordCommandBuffer(VkCommandBuffer& commandBuffer, [[maybe_unused]] uint32_t imageIndex) {
		ecs::ComponentManager* componentManager  = ecs::ComponentManager::GetInstance();
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

		namespace cm = GLVM::ecs::components;
		core::vector<Entity> linkedEntities      = componentManager->collectLinkedEntities<cm::transform,
																						   cm::material,
																						   cm::mesh>();
		
		core::vector<Entity> pointLightEntities = componentManager->collectLinkedEntities<cm::transform,
																						  cm::pointLight,
																						  cm::mesh>();

		VkDebugUtilsLabelEXT label;
		label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
		label.color[0] = 0.1;
		label.color[0] = 0.7;
		label.color[0] = 0.2;
		label.color[0] = 1.0;
		label.pLabelName = "pointLightShadowMap";
		label.pNext = NULL;
		
		CreateBeginDebugUtilsLabelEXT(instance, commandBuffer, &label);
		for ( uint32_t pointLightCounter = 0; pointLightCounter < pointLightEntities.GetSize(); ++pointLightCounter ) {
			uint32_t maxCubeMapLayers = 6;
			for ( uint32_t cubeMapLayerCounter = 0; cubeMapLayerCounter < maxCubeMapLayers; ++cubeMapLayerCounter ) {                      ///< 6 is a number of cube map layers.
				VkClearValue pointLightShadowMapClearValues[2];
				pointLightShadowMapClearValues[0].depthStencil.depth = 1.0f;
				pointLightShadowMapClearValues[0].depthStencil.stencil = 0;
				pointLightShadowMapClearValues[1].color = {{0.5f, 0.5f, 0.5f, 1.0f}};

				VkRenderPassBeginInfo pointLightShadowMapRenderPassInfo{};
				pointLightShadowMapRenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				pointLightShadowMapRenderPassInfo.pNext = NULL;
				pointLightShadowMapRenderPassInfo.renderPass = pointLightShadowMapRenderPass;
				pointLightShadowMapRenderPassInfo.framebuffer = pointLightShadowMapFrameBuffers[pointLightCounter][cubeMapLayerCounter];
				pointLightShadowMapRenderPassInfo.renderArea.offset.x = 0;
				pointLightShadowMapRenderPassInfo.renderArea.offset.y = 0;
				pointLightShadowMapRenderPassInfo.renderArea.extent.width = SHADOW_MAP_SIZE;
				pointLightShadowMapRenderPassInfo.renderArea.extent.height = SHADOW_MAP_SIZE;
				pointLightShadowMapRenderPassInfo.clearValueCount = 2;
				pointLightShadowMapRenderPassInfo.pClearValues = pointLightShadowMapClearValues;

				vkCmdBeginRenderPass(commandBuffer, &pointLightShadowMapRenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

				VkViewport pointLightShadowMapViewPort;
				pointLightShadowMapViewPort.height = SHADOW_MAP_SIZE;
				pointLightShadowMapViewPort.width = SHADOW_MAP_SIZE;
				pointLightShadowMapViewPort.minDepth = 0.0f;
				pointLightShadowMapViewPort.maxDepth = 1.0f;
				pointLightShadowMapViewPort.x = 0;
				pointLightShadowMapViewPort.y = 0;
				vkCmdSetViewport(commandBuffer, 0, 1, &pointLightShadowMapViewPort);

				VkRect2D pointLightShadowMapScissor;
				pointLightShadowMapScissor.extent.width = SHADOW_MAP_SIZE;
				pointLightShadowMapScissor.extent.height = SHADOW_MAP_SIZE;
				pointLightShadowMapScissor.offset.x = 0;
				pointLightShadowMapScissor.offset.y = 0;
				vkCmdSetScissor(commandBuffer, 0, 1, &pointLightShadowMapScissor);

				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pointLightPipeline.pipeline);

				unsigned int pointLightEntity = pointLightEntities[pointLightCounter];
				
				cm::pointLight* pointLightComponent = componentManager->GetComponent<cm::pointLight>(pointLightEntity);
				
				uint32_t actorsNumber = linkedEntities.GetSize();
				for ( unsigned int actorCounter = 0; actorCounter < actorsNumber; ++actorCounter ) {
					unsigned int meshOwnerEntity = linkedEntities[actorCounter];
					unsigned int meshID = componentManager->GetComponent<ecs::components::mesh>(meshOwnerEntity)->handle.id;
					cm::transform* meshOwnerTransformComponent = componentManager->GetComponent<cm::transform>(meshOwnerEntity);
						
					unsigned int uboIndex = pointLightNumber *
						actorsNumber * maxCubeMapLayers * pointLightCurrentFrame +                           ///< Choose frame (first 168 or second 168)
						actorsNumber * maxCubeMapLayers * pointLightCounter +                      ///< Choose point light (i)
						maxCubeMapLayers * actorCounter + cubeMapLayerCounter;                     ///< Choose actor (m) and layer (j)

					updatePointLightShadowMapMatrixUBO(uboIndex, meshOwnerTransformComponent, pointLightComponent, cubeMapLayerCounter, meshID);
					vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pointLightPipeline.pipelineLayout, 0, 1, &shadowMapPointLightDescriptorSets[uboIndex], 0, nullptr);

					VkBuffer vertexBuffers[] = {vertexBufferContainer[meshID]};
					VkDeviceSize offsets[] = {0};
					vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
					
					vkCmdBindIndexBuffer(commandBuffer, indexBufferContainer[meshID], 0, VK_INDEX_TYPE_UINT32);

					unsigned int indicesContainerSize = aIndices_[meshID].size();
					vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indicesContainerSize), 1, 0, 0, 0);
				}
		
				vkCmdEndRenderPass(commandBuffer);
			}
		}

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
	}
	
    VkShaderModule CVulkanRenderer::createShaderModule(const std::vector<char>& code) {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
        }

        return shaderModule;
    }

    VkSurfaceFormatKHR CVulkanRenderer::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    VkPresentModeKHR CVulkanRenderer::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR || availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
//				std::cout << "present mode found!" << std::endl;
                return availablePresentMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D CVulkanRenderer::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        } else {

			VkExtent2D actualExtent{};
            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }

    SwapChainSupportDetails CVulkanRenderer::querySwapChainSupport(VkPhysicalDevice device) {
        SwapChainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
        }
        
        return details;
    }

    bool CVulkanRenderer::isDeviceSuitable(VkPhysicalDevice device) {
        QueueFamilyIndices indices = findQueueFamilies(device);

        bool extensionsSupported = checkDeviceExtensionSupport(device);

        bool swapChainAdequate = false;
        if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

        return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
    }

    bool CVulkanRenderer::checkDeviceExtensionSupport(VkPhysicalDevice device) {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    QueueFamilyIndices CVulkanRenderer::findQueueFamilies(VkPhysicalDevice device) {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

            if (presentSupport) {
                indices.presentFamily = i;
            }

            if (indices.isComplete()) {
                break;
            }

            i++;
        }

        return indices;
    }

    std::vector<const char*> CVulkanRenderer::getRequiredExtensions() {
#ifdef VK_USE_PLATFORM_XLIB_KHR
        std::vector<const char*> pRequiredExtentions = {"VK_KHR_xlib_surface",
            "VK_EXT_acquire_xlib_display", "VK_KHR_display", "VK_KHR_surface",
            "VK_EXT_direct_mode_display"};
#endif

#ifdef VK_USE_PLATFORM_XCB_KHR
		std::vector<const char*> pRequiredExtentions = {"VK_KHR_xcb_surface",
            "VK_KHR_display", "VK_KHR_surface",
            "VK_EXT_direct_mode_display"};
#endif
		
#ifdef VK_USE_PLATFORM_WIN32_KHR
        std::vector<const char*> pRequiredExtentions = {"VK_KHR_win32_surface",
            "VK_KHR_surface"};
#endif

        if (enableValidationLayers) {
            pRequiredExtentions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return pRequiredExtentions;
    }

    bool CVulkanRenderer::checkValidationLayerSupport() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : validationLayers) {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }

        return true;
    }

    std::vector<char> CVulkanRenderer::readFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }

        size_t fileSize = (size_t) file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL CVulkanRenderer::debugCallback([[maybe_unused]] VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, [[maybe_unused]] void* pUserData) {
		// (void) messageSeverity;
		// (void) messageType;
		// (void) pUserData;
		// if ( pCallbackData->messageIdNumber == 941228658 ) {
		// 	[[maybe_unused]] int i = 0;
		// }


		
		// std::cout << "Error code: " << pCallbackData->messageIdNumber << std::endl;
		// std::cout << "Message name:: " << pCallbackData->pMessageIdName << std::endl;
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }

	[[nodiscard]] mat4* CVulkanRenderer::updateAnimationFrames(ecs::components::transform* _transformComponent, unsigned int meshID) {
		if ( jointMatricesPerMesh.GetSize() > 0 && jointMatricesPerMesh[meshID].GetSize() > 0 &&
			 _transformComponent->frameAccumulator >= frames[meshID][_transformComponent->currentAnimationFrame] * 1.0f ) {
			++_transformComponent->currentAnimationFrame;
			if ( jointMatricesPerMesh[meshID].GetSize() > 0 && _transformComponent->currentAnimationFrame == frames[meshID].GetSize() ) {
				_transformComponent->currentAnimationFrame = 0;
				_transformComponent->frameAccumulator = 0.0f;
			}
		}

		unsigned int joinMatricesDataSize{};
		if ( jointMatricesPerMesh.GetSize() > 0 )
			joinMatricesDataSize = jointMatricesPerMesh[meshID].GetSize();

		mat4* jointMatricesData = nullptr;
		if ( joinMatricesDataSize == 0 ) {
			jointMatricesData = new mat4[MAX_JOINTS_NUMBER];
			for ( unsigned int i = 0; i < MAX_JOINTS_NUMBER; ++i ) {
				mat4 unitMatrix(1.0f);
				jointMatricesData[i] = unitMatrix;
			}
				
		} else {
			jointMatricesData = new mat4[MAX_JOINTS_NUMBER];
			for ( unsigned int i = 0; i < joinMatricesDataSize; ++i ) {
				jointMatricesData[i] = jointMatricesPerMesh[meshID][i][_transformComponent->currentAnimationFrame];
			}

			for ( u32 j = joinMatricesDataSize; j < MAX_JOINTS_NUMBER; ++j ) {
				mat4 unitMatrix(1.0f);
				jointMatricesData[j] = unitMatrix;
			}
		}

		return jointMatricesData;
	}

	mat4 CVulkanRenderer::computeModelMatrix(ecs::components::transform* _transformComponent) {
		mat4 rotationMatrix(1.0f);
        mat4 scalingMatrix(1.0f);
        mat4 translationMatrix(1.0f);
		
		scalingMatrix[0][0] = _transformComponent->fScale;
		scalingMatrix[1][1] = _transformComponent->fScale;
		scalingMatrix[2][2] = _transformComponent->fScale;

		translationMatrix[3][0] = _transformComponent->tPosition[0];
		translationMatrix[3][1] = _transformComponent->tPosition[1];
		translationMatrix[3][2] = _transformComponent->tPosition[2];
		translationMatrix[3][3] = 1.0f;

		float sinPitch = std::sin(Radians(-_transformComponent->pitch / 2));
		float cosPitch = std::cos(Radians(-_transformComponent->pitch / 2));
		float sinYaw = std::sin(Radians(-(_transformComponent->yaw)  / 2));
		float cosYaw = std::cos(Radians(-(_transformComponent->yaw)  / 2));
		
		Quaternion pitchQuat;
		Quaternion yawQuat;
		pitchQuat.w = cosPitch;
		pitchQuat.x = 0.0f;
		pitchQuat.y = 0.0f;
		pitchQuat.z = sinPitch;

		yawQuat.w = cosYaw;
		yawQuat.x = 0.0f;
		yawQuat.y = sinYaw;
		yawQuat.z = 0.0f;

		Quaternion result;
		result = multiplyQuaternion(pitchQuat, yawQuat);
		rotationMatrix = rotateQuaternion<float, 4>(result);
		
        return scalingMatrix * rotationMatrix * translationMatrix;
	}
	
	void CVulkanRenderer::setImageDebugObjectName(VK_Image image) {
		VkDebugUtilsObjectNameInfoEXT imageObjectInfo{};
		imageObjectInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		std::string imageName = VK_DEBUG_IMAGE_SET_RED;
		const char* strImageName = imageName.c_str();
		imageObjectInfo.pObjectName = strImageName;
		imageObjectInfo.objectType = VK_OBJECT_TYPE_IMAGE;
		imageObjectInfo.objectHandle = (uint64_t)image.image;
		SetDebugObjectName(device, &imageObjectInfo);
	}
	
	void CVulkanRenderer::setDebugObjectNames() {
 		VkDebugUtilsObjectNameInfoEXT mainPipelineObjectInfo{};
		mainPipelineObjectInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		std::string mainPipeLineImageName = ConcatIntBetweenTwoStrings(VK_DEBUG_PIPELINE_RED, " \x1b[31mMain pipeline #\x1b[0m ", 0);
		const char* mainPipeLineStrImageName = mainPipeLineImageName.c_str();
		mainPipelineObjectInfo.pObjectName = mainPipeLineStrImageName;
		mainPipelineObjectInfo.objectType = VK_OBJECT_TYPE_PIPELINE;
		mainPipelineObjectInfo.objectHandle = (uint64_t)mainRenderScenePipeline.pipeline;
		SetDebugObjectName(device, &mainPipelineObjectInfo);

		VkDebugUtilsObjectNameInfoEXT mainPipelineLayoutObjectInfo{};
		mainPipelineLayoutObjectInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		std::string mainPipelineLayoutImageName = ConcatIntBetweenTwoStrings(VK_DEBUG_PIPELINE_LAYOUT_RED, " \x1b[31mMain pipeline layout #\x1b[0m ", 0);
		const char* mainPipelineLayoutStrImageName = mainPipelineLayoutImageName.c_str();
		mainPipelineLayoutObjectInfo.pObjectName = mainPipelineLayoutStrImageName;
		mainPipelineLayoutObjectInfo.objectType = VK_OBJECT_TYPE_PIPELINE_LAYOUT;
		mainPipelineLayoutObjectInfo.objectHandle = (uint64_t)mainRenderScenePipeline.pipelineLayout;
		SetDebugObjectName(device, &mainPipelineObjectInfo);

		VkDebugUtilsObjectNameInfoEXT directionalLightPipelineObjectInfo{};
		directionalLightPipelineObjectInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		std::string directionalLightPipeLineImageName = ConcatIntBetweenTwoStrings(VK_DEBUG_PIPELINE_RED, " \x1b[31mDirectional light pipeline #\x1b[0m ", 0);
		const char* directionalLightPipeLineStrImageName = directionalLightPipeLineImageName.c_str();
		directionalLightPipelineObjectInfo.pObjectName = directionalLightPipeLineStrImageName;
		directionalLightPipelineObjectInfo.objectType = VK_OBJECT_TYPE_PIPELINE;
		directionalLightPipelineObjectInfo.objectHandle = (uint64_t)directionalLightPipeline.pipeline;
		SetDebugObjectName(device, &directionalLightPipelineObjectInfo);

		VkDebugUtilsObjectNameInfoEXT directionalLightPipelineLayoutObjectInfo{};
		directionalLightPipelineLayoutObjectInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		std::string directionalLightPipelineLayoutImageName = ConcatIntBetweenTwoStrings(VK_DEBUG_PIPELINE_LAYOUT_RED, " \x1b[31mDirectional light pipeline layout #\x1b[0m ", 0);
		const char* directionalLightPipelineLayoutStrImageName = directionalLightPipelineLayoutImageName.c_str();
		directionalLightPipelineLayoutObjectInfo.pObjectName = directionalLightPipelineLayoutStrImageName;
		directionalLightPipelineLayoutObjectInfo.objectType = VK_OBJECT_TYPE_PIPELINE_LAYOUT;
		directionalLightPipelineLayoutObjectInfo.objectHandle = (uint64_t)directionalLightPipeline.pipelineLayout;
		SetDebugObjectName(device, &directionalLightPipelineLayoutObjectInfo);

		VkDebugUtilsObjectNameInfoEXT spotLightPipelineObjectInfo{};
		spotLightPipelineObjectInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		std::string spotLightPipeLineImageName = ConcatIntBetweenTwoStrings(VK_DEBUG_PIPELINE_RED, " \x1b[31mSpot light pipeline #\x1b[0m ", 0);
		const char* spotLightPipeLineStrImageName = spotLightPipeLineImageName.c_str();
		spotLightPipelineObjectInfo.pObjectName = spotLightPipeLineStrImageName;
		spotLightPipelineObjectInfo.objectType = VK_OBJECT_TYPE_PIPELINE;
		spotLightPipelineObjectInfo.objectHandle = (uint64_t)spotLightPipeline.pipeline;
		SetDebugObjectName(device, &spotLightPipelineObjectInfo);

		VkDebugUtilsObjectNameInfoEXT spotLightPipelineLayoutObjectInfo{};
		spotLightPipelineLayoutObjectInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		std::string spotLightPipelineLayoutImageName = ConcatIntBetweenTwoStrings(VK_DEBUG_PIPELINE_LAYOUT_RED, " \x1b[31mSpot light pipeline layout #\x1b[0m ", 0);
		const char* spotLightPipelineLayoutStrImageName = spotLightPipelineLayoutImageName.c_str();
		spotLightPipelineLayoutObjectInfo.pObjectName = spotLightPipelineLayoutStrImageName;
		spotLightPipelineLayoutObjectInfo.objectType = VK_OBJECT_TYPE_PIPELINE_LAYOUT;
		spotLightPipelineLayoutObjectInfo.objectHandle = (uint64_t)spotLightPipeline.pipelineLayout;
		SetDebugObjectName(device, &spotLightPipelineLayoutObjectInfo);

		VkDebugUtilsObjectNameInfoEXT pointLightPipelineObjectInfo{};
		pointLightPipelineObjectInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		std::string pointLightPipeLineImageName = ConcatIntBetweenTwoStrings(VK_DEBUG_PIPELINE_RED, " \x1b[31mPoint light pipeline #\x1b[0m ", 0);
		const char* pointLightPipeLineStrImageName = pointLightPipeLineImageName.c_str();
		pointLightPipelineObjectInfo.pObjectName = pointLightPipeLineStrImageName;
		pointLightPipelineObjectInfo.objectType = VK_OBJECT_TYPE_PIPELINE;
		pointLightPipelineObjectInfo.objectHandle = (uint64_t)pointLightPipeline.pipeline;
		SetDebugObjectName(device, &pointLightPipelineObjectInfo);

		VkDebugUtilsObjectNameInfoEXT pointLightPipelineLayoutObjectInfo{};
		pointLightPipelineLayoutObjectInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		std::string pointLightPipelineLayoutImageName = ConcatIntBetweenTwoStrings(VK_DEBUG_PIPELINE_LAYOUT_RED, " \x1b[31mPoint light pipeline layout #\x1b[0m ", 0);
		const char* pointLightPipelineLayoutStrImageName = pointLightPipelineLayoutImageName.c_str();
		pointLightPipelineLayoutObjectInfo.pObjectName = pointLightPipelineLayoutStrImageName;
		pointLightPipelineLayoutObjectInfo.objectType = VK_OBJECT_TYPE_PIPELINE_LAYOUT;
		pointLightPipelineLayoutObjectInfo.objectHandle = (uint64_t)pointLightPipeline.pipelineLayout;
		SetDebugObjectName(device, &pointLightPipelineLayoutObjectInfo);

		// for ( unsigned long i = 0; i < pointLightShadowMapImages.size(); ++i ) {
		// 	VkDebugUtilsObjectNameInfoEXT pointLightImageObjectInfo{};
		// 	pointLightImageObjectInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		// 	std::string imageName = ConcatIntBetweenTwoStrings(VK_DEBUG_IMAGE_SET_RED, " Point light shadow map image # ", i);
		// 	const char* strImageName = imageName.c_str();
		// 	pointLightImageObjectInfo.pObjectName = strImageName;
		// 	pointLightImageObjectInfo.objectType = VK_OBJECT_TYPE_IMAGE;
		// 	pointLightImageObjectInfo.objectHandle = (uint64_t)pointLightShadowMapImages[i].image;
		// 	SetDebugObjectName(device, &pointLightImageObjectInfo);
		// }

		// for ( unsigned long i = 0; i < spotLightShadowMapImages.size(); ++i ) {
		// 	VkDebugUtilsObjectNameInfoEXT spotLightImageObjectInfo{};
		// 	spotLightImageObjectInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		// 	std::string imageName = ConcatIntBetweenTwoStrings(VK_DEBUG_IMAGE_SET_RED, " Spot light shadow map image # ", i);
		// 	const char* strImageName = imageName.c_str();
		// 	spotLightImageObjectInfo.pObjectName = strImageName;
		// 	spotLightImageObjectInfo.objectType = VK_OBJECT_TYPE_IMAGE;
		// 	spotLightImageObjectInfo.objectHandle = (uint64_t)spotLightShadowMapImages[i].image;
		// 	SetDebugObjectName(device, &spotLightImageObjectInfo);			
		// }

		// for ( unsigned long i = 0; i < directionalLightPipeline.descriptors[0].textureImages.size(); ++i ) {
		// 	VkDebugUtilsObjectNameInfoEXT directionalLightImageObjectInfo{};
		// 	directionalLightImageObjectInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		// 	std::string imageName = ConcatIntBetweenTwoStrings(VK_DEBUG_IMAGE_SET_RED, " Directional light shadow map image # ", i);
		// 	const char* strImageName = imageName.c_str();
		// 	directionalLightImageObjectInfo.pObjectName = strImageName;
		// 	directionalLightImageObjectInfo.objectType = VK_OBJECT_TYPE_IMAGE;
		// 	directionalLightImageObjectInfo.objectHandle = (uint64_t)directionalLightPipeline.descriptors[0].textureImages[i].image;
		// 	SetDebugObjectName(device, &directionalLightImageObjectInfo);			
		// }

		// for ( unsigned long i = 0; i < textureImages.size(); ++i ) {
		// 	VkDebugUtilsObjectNameInfoEXT textureImageObjectInfo{};
		// 	textureImageObjectInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		// 	std::string imageName = ConcatIntBetweenTwoStrings(VK_DEBUG_IMAGE_SET_RED, " Texture image # ", i);
		// 	const char* strImageName = imageName.c_str();
		// 	textureImageObjectInfo.pObjectName = strImageName;
		// 	textureImageObjectInfo.objectType = VK_OBJECT_TYPE_IMAGE;
		// 	textureImageObjectInfo.objectHandle = (uint64_t)textureImages[i].image;
		// 	SetDebugObjectName(device, &textureImageObjectInfo);			
		// }

		// for ( unsigned long i = 0; i < swapChainImages.size(); ++i ) {
		// 	VkDebugUtilsObjectNameInfoEXT swapChainImageObjectInfo{};
		// 	swapChainImageObjectInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		// 	std::string imageName = ConcatIntBetweenTwoStrings(VK_DEBUG_IMAGE_SET_RED, " SwapChain image # ", i);
		// 	const char* strImageName = imageName.c_str();
		// 	swapChainImageObjectInfo.pObjectName = strImageName;
		// 	swapChainImageObjectInfo.objectType = VK_OBJECT_TYPE_IMAGE;
		// 	swapChainImageObjectInfo.objectHandle = (uint64_t)swapChainImages[i];
		// 	SetDebugObjectName(device, &swapChainImageObjectInfo);			
		// }
		
		// for ( unsigned long i = 0; i < directionalLightPipeline.descriptors.GetSize(); ++i ) {
		// 	VkDebugUtilsObjectNameInfoEXT descriptorSetLayoutObjectInfo{};
		// 	descriptorSetLayoutObjectInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		// 	std::string layoutName = ConcatIntBetweenTwoStrings(VK_DEBUG_DESCRIPTOR_SET_LAYOUT_RED, " Directional light shadow map descriptor set layout # ", i);
		// 	const char* strLayoutName = layoutName.c_str();
		// 	descriptorSetLayoutObjectInfo.pObjectName = strLayoutName;
		// 	descriptorSetLayoutObjectInfo.objectType = VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT;
		// 	descriptorSetLayoutObjectInfo.objectHandle = (uint64_t)directionalLightPipeline.descriptors[i].setLayout;
		// 	SetDebugObjectName(device, &descriptorSetLayoutObjectInfo);			
		// }

		// for ( unsigned long i = 0; i < mainRenderScenePipeline.descriptors.GetSize(); ++i ) {
		// 	VkDebugUtilsObjectNameInfoEXT descriptorSetLayoutObjectInfo{};
		// 	descriptorSetLayoutObjectInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		// 	std::string layoutName = ConcatIntBetweenTwoStrings(VK_DEBUG_DESCRIPTOR_SET_LAYOUT_RED, " Main render descriptor set layout # ", i);
		// 	const char* strLayoutName = layoutName.c_str();
		// 	descriptorSetLayoutObjectInfo.pObjectName = strLayoutName;
		// 	descriptorSetLayoutObjectInfo.objectType = VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT;
		// 	descriptorSetLayoutObjectInfo.objectHandle = (uint64_t)mainRenderScenePipeline.descriptors[i].setLayout;
		// 	SetDebugObjectName(device, &descriptorSetLayoutObjectInfo);			
		// }
		
		for ( unsigned long i = 0; i < shadowMapDirectionalLightDescriptorSets.size(); ++i ) {
			VkDebugUtilsObjectNameInfoEXT descriptorSetObjectInfo{};
			descriptorSetObjectInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
			std::string name = ConcatIntBetweenTwoStrings(VK_DEBUG_DESCRIPTOR_SET_RED, " Directional light shadow map descriptor set # ", i);
			const char* strName = name.c_str();
			descriptorSetObjectInfo.pObjectName = strName;
			descriptorSetObjectInfo.objectType = VK_OBJECT_TYPE_DESCRIPTOR_SET;
			descriptorSetObjectInfo.objectHandle = (uint64_t)shadowMapDirectionalLightDescriptorSets[i];
			SetDebugObjectName(device, &descriptorSetObjectInfo);
		}

 		// for ( unsigned long i = 0; i < matrixUboDescriptorSets.size(); ++i ) {
		// 	VkDebugUtilsObjectNameInfoEXT descriptorSetObjectInfo{};
		// 	descriptorSetObjectInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		// 	std::string name = ConcatIntBetweenTwoStrings(VK_DEBUG_DESCRIPTOR_SET_RED, " Main render model matrix descriptor set # ", i);
		// 	const char* strName = name.c_str();
		// 	descriptorSetObjectInfo.pObjectName = strName;
		// 	descriptorSetObjectInfo.objectType = VK_OBJECT_TYPE_DESCRIPTOR_SET;
		// 	descriptorSetObjectInfo.objectHandle = (uint64_t)matrixUboDescriptorSets[i];
		// 	SetDebugObjectName(device, &descriptorSetObjectInfo);
		// }

		// for ( unsigned long i = 0; i < viewPositionUboDescriptorSets.size(); ++i ) {
		// 	VkDebugUtilsObjectNameInfoEXT descriptorSetObjectInfo{};
		// 	descriptorSetObjectInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		// 	std::string name = ConcatIntBetweenTwoStrings(VK_DEBUG_DESCRIPTOR_SET_RED, " Main render view position descriptor set # ", i);
		// 	const char* strName = name.c_str();
		// 	descriptorSetObjectInfo.pObjectName = strName;
		// 	descriptorSetObjectInfo.objectType = VK_OBJECT_TYPE_DESCRIPTOR_SET;
		// 	descriptorSetObjectInfo.objectHandle = (uint64_t)viewPositionUboDescriptorSets[i];
		// 	SetDebugObjectName(device, &descriptorSetObjectInfo);
		// }

		// for ( unsigned long i = 0; i < materialUboDescriptorSets.size(); ++i ) {
		// 	VkDebugUtilsObjectNameInfoEXT descriptorSetObjectInfo{};
		// 	descriptorSetObjectInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		// 	std::string name = ConcatIntBetweenTwoStrings(VK_DEBUG_DESCRIPTOR_SET_RED, " Main render material descriptor set # ", i);
		// 	const char* strName = name.c_str();
		// 	descriptorSetObjectInfo.pObjectName = strName;
		// 	descriptorSetObjectInfo.objectType = VK_OBJECT_TYPE_DESCRIPTOR_SET;
		// 	descriptorSetObjectInfo.objectHandle = (uint64_t)materialUboDescriptorSets[i];
		// 	SetDebugObjectName(device, &descriptorSetObjectInfo);
		// }

		// for ( unsigned long i = 0; i < directionalLightUboDescriptorSets.size(); ++i ) {
		// 	VkDebugUtilsObjectNameInfoEXT descriptorSetObjectInfo{};
		// 	descriptorSetObjectInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		// 	std::string name = ConcatIntBetweenTwoStrings(VK_DEBUG_DESCRIPTOR_SET_RED, " Main render directional light descriptor set # ", i);
		// 	const char* strName = name.c_str();
		// 	descriptorSetObjectInfo.pObjectName = strName;
		// 	descriptorSetObjectInfo.objectType = VK_OBJECT_TYPE_DESCRIPTOR_SET;
		// 	descriptorSetObjectInfo.objectHandle = (uint64_t)directionalLightUboDescriptorSets[i];
		// 	SetDebugObjectName(device, &descriptorSetObjectInfo);
		// }

		// for ( unsigned long i = 0; i < pointLightUboDescriptorSets.size(); ++i ) {
		// 	VkDebugUtilsObjectNameInfoEXT descriptorSetObjectInfo{};
		// 	descriptorSetObjectInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		// 	std::string name = ConcatIntBetweenTwoStrings(VK_DEBUG_DESCRIPTOR_SET_RED, " Main render point light descriptor set # ", i);
		// 	const char* strName = name.c_str();
		// 	descriptorSetObjectInfo.pObjectName = strName;
		// 	descriptorSetObjectInfo.objectType = VK_OBJECT_TYPE_DESCRIPTOR_SET;
		// 	descriptorSetObjectInfo.objectHandle = (uint64_t)pointLightUboDescriptorSets[i];
		// 	SetDebugObjectName(device, &descriptorSetObjectInfo);
		// }

		// for ( unsigned long i = 0; i < spotLightUboDescriptorSets.size(); ++i ) {
		// 	VkDebugUtilsObjectNameInfoEXT descriptorSetObjectInfo{};
		// 	descriptorSetObjectInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		// 	std::string name = ConcatIntBetweenTwoStrings(VK_DEBUG_DESCRIPTOR_SET_RED, " Main render spot light descriptor set # ", i);
		// 	const char* strName = name.c_str();
		// 	descriptorSetObjectInfo.pObjectName = strName;
		// 	descriptorSetObjectInfo.objectType = VK_OBJECT_TYPE_DESCRIPTOR_SET;
		// 	descriptorSetObjectInfo.objectHandle = (uint64_t)spotLightUboDescriptorSets[i];
		// 	SetDebugObjectName(device, &descriptorSetObjectInfo);
		// }

		// for ( unsigned long i = 0; i < diffuseSamplerDescriptorSets.size(); ++i ) {
		// 	VkDebugUtilsObjectNameInfoEXT descriptorSetObjectInfo{};
		// 	descriptorSetObjectInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		// 	std::string name = ConcatIntBetweenTwoStrings(VK_DEBUG_DESCRIPTOR_SET_RED, " Main render diffuse sampler descriptor set # ", i);
		// 	const char* strName = name.c_str();
		// 	descriptorSetObjectInfo.pObjectName = strName;
		// 	descriptorSetObjectInfo.objectType = VK_OBJECT_TYPE_DESCRIPTOR_SET;
		// 	descriptorSetObjectInfo.objectHandle = (uint64_t)diffuseSamplerDescriptorSets[i];
		// 	SetDebugObjectName(device, &descriptorSetObjectInfo);
		// }

 		// for ( unsigned long i = 0; i < specularSamplerDescriptorSets.size(); ++i ) {
		// 	VkDebugUtilsObjectNameInfoEXT descriptorSetObjectInfo{};
		// 	descriptorSetObjectInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		// 	std::string name = ConcatIntBetweenTwoStrings(VK_DEBUG_DESCRIPTOR_SET_RED, " Main render specular sampler descriptor set # ", i);
		// 	const char* strName = name.c_str();
		// 	descriptorSetObjectInfo.pObjectName = strName;
		// 	descriptorSetObjectInfo.objectType = VK_OBJECT_TYPE_DESCRIPTOR_SET;
		// 	descriptorSetObjectInfo.objectHandle = (uint64_t)specularSamplerDescriptorSets[i];
		// 	SetDebugObjectName(device, &descriptorSetObjectInfo);
		// }

		// for ( unsigned long i = 0; i < shadowMapDirectionalLightDescriptorSets.size(); ++i ) {
		// 	VkDebugUtilsObjectNameInfoEXT descriptorSetObjectInfo{};
		// 	descriptorSetObjectInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		// 	std::string name = ConcatIntBetweenTwoStrings(VK_DEBUG_DESCRIPTOR_SET_RED, " Main render shadow map directional light descriptor set # ", i);
		// 	const char* strName = name.c_str();
		// 	descriptorSetObjectInfo.pObjectName = strName;
		// 	descriptorSetObjectInfo.objectType = VK_OBJECT_TYPE_DESCRIPTOR_SET;
		// 	descriptorSetObjectInfo.objectHandle = (uint64_t)shadowMapDirectionalLightDescriptorSets[i];
		// 	SetDebugObjectName(device, &descriptorSetObjectInfo);
		// }
	}
}

