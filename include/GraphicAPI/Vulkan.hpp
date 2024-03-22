// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#ifndef VULKAN_RENDERER_HG
#define VULKAN_RENDERER_HG

#include <cstdint>
#include <iostream>
#include <fstream>
#include <new>
#include <stdexcept>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <array>
#include <optional>
#include <set>
#include <cmath>

#include "Components/MaterialComponent.hpp"
#include "Components/TransformComponent.hpp"
#include "Components/TextureComponent.hpp"
#include "IRenderer.hpp"
#include "Texture.hpp"
#include "Vector.hpp"
#include "VertexMath.hpp"
#include "TextureManager.hpp"
#include "ComponentManager.hpp"
#include "WavefrontObjParser.hpp"
#include "MeshManager.hpp"
#include "Globals.hpp"
#include "ToString.hpp"
#include "JsonParser.hpp"

#ifdef __linux__
//#define VK_USE_PLATFORM_XLIB_KHR
#define VK_USE_PLATFORM_XCB_KHR
#endif

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#ifdef VK_USE_PLATFORM_XCB_KHR
#include "vulkan/vulkan.h"
#include <xcb/xcb.h>
#include "vulkan/vulkan_xcb.h"
#include "vulkan/vulkan_core.h"
#include "UnixApi/WindowXCBVulkan.hpp"
#endif

#ifdef VK_USE_PLATFORM_XLIB_KHR
#include "vulkan/vulkan.h"
#include <X11/Xlib.h>
#include "vulkan/vulkan_xlib.h"
#include "vulkan/vulkan_core.h"
#include "UnixApi/WindowXVulkan.hpp"
#endif

#ifdef VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include "WinApi/WindowWinVulkan.hpp"
#endif

#define SHADOW_MAP_SIZE 640

#define VK_DEBUG_IMAGE_SET_RED "\x1b[31mVULKAN DEBUG IMAGE\x1b[0m"
#define VK_DEBUG_DESCRIPTOR_SET_RED "\x1b[31mVULKAN DEBUG DESCRIPTOR SET\x1b[0m"
#define VK_DEBUG_DESCRIPTOR_SET_LAYOUT_RED "\x1b[31mVULKAN DEBUG DESCRIPTOR SET LAYOUT\x1b[0m"
#define VK_DEBUG_PIPELINE_RED "\x1b[31mVULKAN DEBUG PIPELINE:\x1b[0m"
#define VK_DEBUG_PIPELINE_LAYOUT_RED "\x1b[31mVULKAN DEBUG PIPELINE LAYOUT:\x1b[0m"

#define DIRECTIONAL_LIGHTS_NUMBER                          2
#define POINT_LIGHTS_NUMBER                                2
#define SPOT_LIGHTS_NUMBER                                 2

namespace GLVM::core
{
    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;

    const int MAX_FRAMES_IN_FLIGHT = 2;
#define NDEBUG
    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME, "VK_KHR_shader_non_semantic_info"
    };

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);

    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

    struct Texture {
        VkDeviceSize textureSize_;
        unsigned char* textureData_;
    };

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };
    
    struct Vertex {
        vec3 pos;
        vec3 color;
        vec2 texCoord;
		vec4 joinIndices;
		vec4 weights;

        static VkVertexInputBindingDescription getBindingDescription() {
            VkVertexInputBindingDescription bindingDescription{};
            bindingDescription.binding = 0;
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            bindingDescription.stride = sizeof(Vertex);

            return bindingDescription;
        }

        static std::array<VkVertexInputAttributeDescription, 5> getAttributeDescriptions() {
            std::array<VkVertexInputAttributeDescription, 5> attributeDescriptions{};

            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[0].offset = offsetof(Vertex, pos);

            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[1].offset = offsetof(Vertex, color);

            attributeDescriptions[2].binding = 0;
            attributeDescriptions[2].location = 2;
            attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

            attributeDescriptions[3].binding = 0;
            attributeDescriptions[3].location = 3;
            attributeDescriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
            attributeDescriptions[3].offset = offsetof(Vertex, joinIndices);

            attributeDescriptions[4].binding = 0;
            attributeDescriptions[4].location = 4;
            attributeDescriptions[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
            attributeDescriptions[4].offset = offsetof(Vertex, weights);
			
            return attributeDescriptions;
        }
    };
	
	enum class DescriptorsTypes {
		/// UBO - uniform buffer object
		DIRECTIONAL_LIGHT_SHADOW_MAP_MATRIX_UBO,
		SPOT_LIGHT_SHADOW_MAP_MATRIX_UBO,
		POINT_LIGHT_SHADOW_MAP_MATRIX_UBO,
		MODEL_MATRIX_UBO,

		LIGHT_DATA,
		SPECULAR_SAMPLER,
		LIGHT_SAMPLERS,
	};

	struct VK_Image {
		VkImage image;
		VkDeviceMemory deviceMemory;
		std::vector<VkImageView> views;
		VkImageViewType viewType;
		VkImageCreateFlags createFlags;
		VkMemoryPropertyFlags memoryPropertyFlags;
		VkImageUsageFlags usageFlags;
		VkImageAspectFlags aspectFlags;
		VkFormat format;
		VkImageTiling tiling;
		VkSampler sampler;
		VkComponentSwizzle red;
		VkComponentSwizzle green;
		VkComponentSwizzle blue;
		VkComponentSwizzle alpha;
		uint32_t arrayLayers;
		uint32_t width;
		uint32_t height;
	};
	
	struct Descriptor {
		VkDescriptorType       vkType;
		DescriptorsTypes       type;
		core::vector<u32> binding;
		VkShaderStageFlags     shaderStageFlag;
		VkDescriptorSetLayout  setLayout;
		core::vector<u32>               descriptorsNumber;

		std::vector<VkBuffer> uniformBuffers;
		std::vector<VkDeviceMemory> uniformBuffersMemory;
		
		std::vector<VK_Image> textureImages;
	};

	struct Pipeline {
		core::vector<Descriptor> descriptors;
		unsigned int globalDescriptorsNumber = 0;
		unsigned int uboDescriptorsNumber = 0;
		unsigned int combinedImageSamplersNumber = 0;
		VkPipeline  pipeline;
		VkPipelineLayout pipelineLayout;
		const char* vertShader = nullptr;
		const char* fragShader = nullptr;
		VkVertexInputBindingDescription bindingDescription;
		std::array<VkVertexInputAttributeDescription, 5> attributeDescriptions;

		void addDescriptor(VkDescriptorType vkType, DescriptorsTypes type, VkShaderStageFlags shaderStageFlag,
						   core::vector<u32> descriptorsNumbers, core::vector<uint32_t> bindings) {
			if (vkType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
				descriptors.Push({vkType, type, bindings, shaderStageFlag, VkDescriptorSetLayout(), descriptorsNumbers, {}, {}, {}});
				++uboDescriptorsNumber;
			} else if (vkType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
				descriptors.Push({vkType, type, bindings, shaderStageFlag, VkDescriptorSetLayout(), descriptorsNumbers, {}, {}, {}});
				++combinedImageSamplersNumber;
			} else {
				assert(!"unreachable");
			}
		}

		core::vector<u32> getBindingOfDescriptor(DescriptorsTypes type) {
			for ( unsigned int i = 0; i < descriptors.GetSize(); ++i ) {
				if ( type == descriptors[i].type )
					return descriptors[i].binding;
			}

			core::vector<u32> empty;
			return empty;
		}
	};

	struct LightSpaceMatrixUBO {
		alignas(16) mat4 spotSpaceMatrix[SPOT_LIGHTS_NUMBER];
		alignas(16) uint32_t spotLightsNumber;
		
		alignas(16) mat4 dirSpaceMatrix[DIRECTIONAL_LIGHTS_NUMBER];
		alignas(16) uint32_t directionalLightsNumber;
	};

#define MAX_JOINTS_NUMBER 18
	
    struct alignas(64) ModelMatrixUBO {
        mat4 model;
        mat4 view;
        mat4 proj;
		mat4 jointMatrices[MAX_JOINTS_NUMBER];

		vec3  ambient;
		float shininess;

		alignas(16) mat4 spotSpaceMatrix[SPOT_LIGHTS_NUMBER];
		alignas(16) uint32_t spotLightsNumber;
		
		alignas(16) mat4 dirSpaceMatrix[DIRECTIONAL_LIGHTS_NUMBER];
		alignas(16) uint32_t directionalLightsNumber;
    };

	struct alignas(16) ShadowMapMatrixUBO {
		mat4 model;
		mat4 lightSpaceMatrix;
		mat4 jointMatrices[30];
	};

	struct alignas(16) SpotLightShadowMapMatrixUBO {
		mat4 model;
		mat4 lightSpaceMatrix;
	};

	struct alignas(64) PointLightShadowMapMatrixUBO {
		mat4 model;
		mat4 lightSpaceMatrix;
		vec3 lightPosition;
		float farPlane;
		mat4 jointMatrices[MAX_JOINTS_NUMBER];
	};

	struct alignas(16) UniformBufferObjectLightUBO {
		vec3 lightPosition;
		float farPlane;
	};
	
	struct alignas(16) DirectionalLight {
		vec4 position;
		vec4 direction;

		vec4 ambient;
		vec4 diffuse;
		vec4 specular;
	};

	struct alignas(16) PointLight {
		vec3 position;
		float padding0;

		vec3 ambient;
		float padding1;
		vec3 diffuse;
		float padding2;

		vec3 specular;
		float constant;
		float linear;
		float quadratic;
	};

	struct alignas(16) SpotLight {
		alignas(16) vec3  position;
		alignas(16) vec3  direction;
		float cutOff;
		float outerCutOff;

		alignas(16) vec3  ambient;
		alignas(16) vec3  diffuse;
		alignas(16) vec3  specular;

		float constant;
		float linear;
		float quadratic;
	};
	
    struct LightData {
		alignas(16) vec3 viewPosition;

		PointLight pointLights[POINT_LIGHTS_NUMBER];
		int pointLightsArraySize;
		float farPlane;

		DirectionalLight directionalLights[DIRECTIONAL_LIGHTS_NUMBER];
		int directionalLightsArraySize;
		
		SpotLight spotLights[SPOT_LIGHTS_NUMBER];
		int spotLightArraySize;
    };

    const std::vector<Vertex> vertices = {
        {{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 0.0f}},
        {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 0.0f}},
        {{0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 0.0f}},

        {{-0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 0.0f}},
        {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 0.0f}},
        {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f},{0.0f, 0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 0.0f}},
        {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 0.0f}},
    };
    
    const std::vector<uint16_t> indices = {
        4, 2, 0,
        2, 7, 3,
        6, 5, 7,
        1, 7, 5,
        0, 3, 1,
        4, 1, 5,
        4, 6, 2,
        2, 6, 7,
        6, 4, 5,
        1, 3, 7,
        0, 2, 3,
        4, 0, 1
    };

    const std::vector<uint16_t> hudIndices = {
        0, 1, 2, 2, 1, 3,
        4, 5, 6, 6, 5, 7
    };
    
    class CVulkanRenderer : public IRenderer {
    public:
	    float previousTime = 0;
		float accumulator = 0;
		bool animationFlag = false;
		unsigned int actorsNumber = 0;
		
        std::vector<ecs::Texture> initializeTextureData_;
        std::vector<ecs::Texture> texture_load_data_;
        std::vector<ecs::Texture> hudTexture_load_data_;
        std::vector<ecs::components::transform> transform_data_;
        std::vector<const char*> pathsArray_;
		core::vector<const char*> pathsGLTF_;
        std::vector<std::vector<core::Vertex>> aVertices_;
//		std::vector<std::vector<core::Vertex>> aVertices_GLTF;
        std::vector<std::vector<uint32_t>> aIndices_;                 ///< wavefront.obj indices
		std::vector<std::vector<float>> aVertexesTemp_;                   ///< gltf indices
		std::vector<std::vector<uint32_t>> aIndicesTemp_;             ///< Temp
		core::vector<core::vector<core::vector<mat4>>> jointMatricesPerMesh;
		core::vector<core::vector<float>> frames;

		float fYaw   = -90.0f;
        float fPitch = 0.0f;

        const char* vertShaderMain_ = "../VKshaders/mainRendererShaders/vert.spv";
        const char* fragShaderMain_ = "../VKshaders/mainRendererShaders/frag.spv";

        const char* vertShaderFlatShadowMap = "../VKshaders/flatShadowMapShaders/vertFlatShadowMap.spv";
        const char* fragShaderDirectionalLightShadowMap = "../VKshaders/flatShadowMapShaders/fragFlatShadowMap.spv";

        const char* vertShaderCubeShadowMap = "../VKshaders/cubeShadowMapShaders/vertCubeShadowMap.spv";
        const char* fragShaderCubeShadowMap = "../VKshaders/cubeShadowMapShaders/fragCubeShadowMap.spv";
		
        unsigned int texturePool_;

#ifdef VK_USE_PLATFORM_XCB_KHR
		GLVM::core::WindowXCBVulkan Window;
#endif
		
#ifdef VK_USE_PLATFORM_XLIB_KHR
        GLVM::core::WindowXVulkan Window;
#endif
    
#ifdef VK_USE_PLATFORM_WIN32_KHR
        GLVM::core::WindowWinVulkan Window;
#endif
        
        CVulkanRenderer();
        ~CVulkanRenderer() override;

        void createTextureImage();
        void recreateSwapChain();
        void draw() override;
        void loadWavefrontObj() override;
		void EnlargeFrameAccumulator(float value) override;
        void SetTextureData(std::vector<ecs::Texture>& _texture_data) override;
        void SetMeshData(std::vector<const char*> _pathsArray, core::vector<const char*> pathsGLTF) override;
        void SetViewMatrix(mat4 _viewMatrix) override;
        void SetProjectionMatrix(mat4 _projectionMatrix) override;
		void SetViewMatrix(ecs::components::transform& _Player, ecs::components::beholder& cameraComponent);
		void SetProjectionMatrix();
        void run() override;
    
    private:
        VkInstance instance;
        VkDebugUtilsMessengerEXT debugMessenger;
		mat4 viewMatrix;
		mat4 projectionMatrix;

#ifdef VK_USE_PLATFORM_XLIB_KHR
        VkXlibSurfaceCreateInfoKHR createXlibSurfaceInfo;
#endif

#ifdef VK_USE_PLATFORM_XCB_KHR
        VkXcbSurfaceCreateInfoKHR createXcbSurfaceInfo;
#endif		
		
#ifdef VK_USE_PLATFORM_WIN32_KHR
        VkWin32SurfaceCreateInfoKHR createWin32SurfaceInfo;
#endif
    
        VkSurfaceKHR surface;

        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkDevice device;

        VkQueue graphicsQueue;
        VkQueue presentQueue;

        VkSwapchainKHR swapChain;
        std::vector<VkImage> swapChainImages;
        VkFormat swapChainImageFormat;
        VkExtent2D swapChainExtent;
        std::vector<VkImageView> swapChainImageViews;
        std::vector<VkFramebuffer> swapChainFramebuffers;

        VkRenderPass renderPass;

		Pipeline mainRenderScenePipeline;
		Pipeline directionalLightPipeline;
		Pipeline spotLightPipeline;
		Pipeline pointLightPipeline;
		
        VkPipelineLayout pipelineLayout;
        VkPipeline graphicsPipeline;

        VkCommandPool directionalLightCommandPool;
		VkCommandPool spotLightCommandPool;
		VkCommandPool pointLightCommandPool;
		VkCommandPool mainRenderCommandPool;

		/// Main pipeline depth.
        VkImageView depthImageView;

		/// Depth varialbes for shadow map.
		unsigned int	directionalLightNumber = 0;
		std::vector<VK_Image> directionalLightShadowMapImages;
		std::vector<VkFramebuffer> directionalLightShadowMapFrameBuffers;
		VkRenderPass directionalLightShadowMapRenderPass;
		std::vector<VkSampler> directionalLightShadowMapTextureSamplers;
		std::vector<VkDescriptorSet> shadowMapDirectionalLightDescriptorSets;
		std::vector<VkBuffer> shadowMapDirectionalLightModelMatrixUniformBuffers;
		std::vector<VkDeviceMemory> shadowMapDirectionalLightModelMatrixUniformBuffersMemory;

		/*
		===================================
		FOR TEST ONLY!!!
		===================================
		*/
		mat4 dirLightSpaceMatrix[DIRECTIONAL_LIGHTS_NUMBER];
		mat4 spotLightSpaceMatrix[SPOT_LIGHTS_NUMBER];
		std::vector<VkBuffer> lightSpaceMatrixBuffer;
		std::vector<VkDeviceMemory> lightSpaceMatrixMemory;
		std::vector<VkDescriptorSet> lightSpaceMatrixDescriptorSet;
		
		unsigned int	pointLightNumber	   = 0;
		std::vector<VK_Image> spotLightShadowMapImages;
		std::vector<std::vector<VkFramebuffer>> pointLightShadowMapFrameBuffers;
		VkRenderPass pointLightShadowMapRenderPass;
		std::vector<VkSampler> pointLightShadowMapTextureSamplers;
		std::vector<VkDescriptorSet> shadowMapPointLightDescriptorSets;
		std::vector<VkDescriptorSet> shadowMapPointLightDataDescriptorSets;
		std::vector<VkBuffer> shadowMapPointLightModelMatrixUniformBuffers;
		std::vector<VkDeviceMemory> shadowMapPointLightModelMatrixUniformBuffersMemory;
		std::vector<VkBuffer> shadowMapPointLightDataUniformBuffers;
		std::vector<VkDeviceMemory> shadowMapPointLightDataUniformBuffersMemory;

		unsigned int	spotLightNumber		   = 0;
		std::vector<VK_Image> pointLightShadowMapImages;
		std::vector<VK_Image> pointLightImages;
		std::vector<VkFramebuffer> spotLightShadowMapFrameBuffers;
		VkRenderPass spotLightShadowMapRenderPass;
		std::vector<VkSampler> spotLightShadowMapTextureSamplers;
		std::vector<VkDescriptorSet> shadowMapSpotLightDescriptorSets;
		std::vector<VkBuffer> shadowMapSpotLightModelMatrixUniformBuffers;
		std::vector<VkDeviceMemory> shadowMapSpotLightModelMatrixUniformBuffersMemory;

		core::vector<mat4> shadowMapBasisMatrices;
		std::vector<VkDescriptorSet> shadowMapMatrixUboDescriptorSets;

		std::vector<VK_Image> textureImages;

        std::vector<VkBuffer> vertexBufferContainer;
        std::vector<VkDeviceMemory> vertexBufferMemoryContainer;
        std::vector<VkBuffer> indexBufferContainer;
        std::vector<VkDeviceMemory> indexBufferMemoryContaner;
		uint32_t wavefrontObjCounter = 0;

        std::vector<VkBuffer> modelMatrixUniformBuffers;
        std::vector<VkDeviceMemory> modelMatrixUniformBuffersMemory;
        std::vector<VkBuffer> lightDataUniformBuffers;
        std::vector<VkDeviceMemory> lightDataUniformBuffersMemory;
        std::vector<VkBuffer> materialUniformBuffers;
        std::vector<VkDeviceMemory> materialUniformBuffersMemory;
        std::vector<VkBuffer> directionalLightsUniformBuffers;
        std::vector<VkDeviceMemory> directionalLightsUniformBuffersMemory;
		std::vector<VkBuffer> pointLightsUniformBuffers;
        std::vector<VkDeviceMemory> pointLightsUniformBuffersMemory;
		std::vector<VkBuffer> spotLightsUniformBuffers;
        std::vector<VkDeviceMemory> spotLightsUniformBuffersMemory;

		VkDescriptorImageInfo directionalLightsImageInfo[DIRECTIONAL_LIGHTS_NUMBER];
		VkDescriptorImageInfo pointLightsImageInfo[POINT_LIGHTS_NUMBER];
		VkDescriptorImageInfo spotLightsImageInfo[SPOT_LIGHTS_NUMBER];
		
        VkDescriptorPool descriptorPool;
		unsigned int matrixUboDescriptorsNumber = 0;
//		unsigned int viewPositionUboDescriptorsNumber = 0;
		unsigned int directionalLightUboDescriptorsNumber = 0;
		unsigned int pointLightUboDescriptorsNumber = 0;
		unsigned int spotLightUboDescriptorsNumber = 0;
		u32 lightDataSize;                                                        ///< Var for choose correct number of ds from dir, spot, point light and beholder number
        std::vector<VkDescriptorSet> matrixUboDescriptorSets;
		std::vector<VkDescriptorSet> lightDataUboDescriptorSets;
		std::vector<VkDescriptorSet> materialUboDescriptorSets;
		std::vector<VkDescriptorSet> directionalLightUboDescriptorSets;
		std::vector<VkDescriptorSet> pointLightUboDescriptorSets;
		std::vector<VkDescriptorSet> spotLightUboDescriptorSets;
		std::vector<VkDescriptorSet> diffuseSamplerDescriptorSets;
		std::vector<VkDescriptorSet> specularSamplerDescriptorSets;
		std::vector<VkDescriptorSet> directionalLightSamperDescriptorSets;
		std::vector<VkDescriptorSet> pointLightSamplerDescriptorSets;
		std::vector<VkDescriptorSet> spotLightSamplerDescriptorSets;

        std::vector<VkCommandBuffer> directionalLightCommandBuffers;
		std::vector<VkCommandBuffer> spotLightCommandBuffers;
		std::vector<VkCommandBuffer> pointLightCommandBuffers;
		std::vector<VkCommandBuffer> mainRenderCommandBuffers;

		/// Main render pipe line sync objects
        std::vector<VkSemaphore> imageAvailableSemaphores;
        std::vector<VkSemaphore> renderFinishedSemaphores;
        std::vector<VkFence> inFlightFences;

		/// Directional light shadow map sync objects
        std::vector<VkSemaphore> directionalLightShadowMapImageAvailableSemaphores;
        std::vector<VkSemaphore> directionalLightShadowMapRenderFinishedSemaphores;
        std::vector<VkFence> directionalLightShadowMapInFlightFences;

		/// Spot light shadow map sync objects
        std::vector<VkSemaphore> spotLightShadowMapImageAvailableSemaphores;
        std::vector<VkSemaphore> spotLightShadowMapRenderFinishedSemaphores;
        std::vector<VkFence> spotLightShadowMapInFlightFences;

		/// Point light shadow map sync objects
        std::vector<VkSemaphore> pointLightShadowMapImageAvailableSemaphores;
        std::vector<VkSemaphore> pointLightShadowMapRenderFinishedSemaphores;
        std::vector<VkFence> pointLightShadowMapInFlightFences;
		
        uint32_t currentFrame = 0;
		uint32_t directionalLightCurrentFrame = 0;
		uint32_t spotLightCurrentFrame = 0;
		uint32_t pointLightCurrentFrame = 0;

		std::mutex mutex0; 
		std::mutex mutex1;
		std::mutex mutex2;
		std::mutex shadowMapPassesMutex;
		
        bool framebufferResized = false;

        void initWindow();
		void initializeGLTF();
        void initVulkan();
        void cleanupSwapChain();
        void cleanup();
        void createInstance();
        void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
        void setupDebugMessenger();
        void createSurface();
        void pickPhysicalDevice();
        void createLogicalDevice();
        void createSwapChain();
        void createImageViews();
        void createMainRenderPass();
		void createDirectionalLightShadowMapRenderPass();
		void createSpotLightShadowMapRenderPass();
		void createPointLightShadowMapRenderPass();
        void createDescriptorSetLayout(core::vector<Descriptor>& descriptors);
        void createGraphicsPipeline(Pipeline& pipeline, VkRenderPass& renderPass);
        void createRenderPassFramebuffers(std::vector<VkImageView>& attachments, VkRenderPass& renderPass_,
										  VkFramebuffer& swapChainFramebuffer, uint32_t width,
										  uint32_t height);
		void createFramebuffers();
        void createCommandPool(VkCommandPool& commandPool);
        void createDepthResources();
		void createDirectionalLightShadowMapDepthResources();
		void createSpotLightShadowMapDepthResources();
		void createPointLightShadowMapDepthResources();
        VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
        VkFormat findDepthFormat();
        bool hasStencilComponent(VkFormat format);
        void createTextureImageView();
		void createShadowMapTextureImageView();
        void createTextureSampler();
		void createShadowMapTextureSampler();
		void createDirectionalLightShadowMapTextureSamplers();
		void createSpotLightShadowMapTextureSamplers();
		void createPointLightShadowMapTextureSamplers();
		void createRenderPassShadowMapTextureSamplers(VkSampler& shadowMapTextureSampler);
        VkImageView createImageView(VK_Image image, uint32_t baseArrayLayers, uint32_t layerCount);
        void createImage(VK_Image& image);
        void transitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);
		void transitionShadowMapImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);
        void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
        void createVertexBuffer(VkBuffer& _vertexBuffer, VkDeviceMemory& _vertexBufferMemory, const std::vector<Vertex>& _vertices);
        void createIndexBuffer(VkBuffer& _indexBuffer, VkDeviceMemory& _indexBufferMemory, const std::vector<uint32_t>& _indices);
        void createMainRenderUniformBuffers();
        void createMainRenderDescriptorPool();
		void createDirectionalLightShadowMapDescriptorSets();
		void createSpotLightShadowMapDescriptorSets();
		void createPointLightShadowMapDescriptorSets();
        void createMainRenderDescriptorSets();
		void updateSamplersDescriptroSets(uint32_t diffuse_id, uint32_t specular_id );
		void updateDirectionalLightShadowMapDescriptorSets();
		void updateSpotLightShadowMapDescriptorSets();
		void updatePointLightShadowMapDescriptorSets();
		void updateDescriptorSets();
        void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
        VkCommandBuffer beginSingleTimeCommands(VkCommandPool& commandPool);
        void endSingleTimeCommands(VkCommandPool& commandPool, VkCommandBuffer& commandBuffer);
        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
        void createCommandBuffers(VkCommandPool& commandPool, std::vector<VkCommandBuffer>& commandBuffers);
        void recordCommandBuffer(VkCommandBuffer& commandBuffer, uint32_t imageIndex);
        void createSyncObjects(std::vector<VkSemaphore>& imageAvailableSemaphores,
							   std::vector<VkSemaphore>& renderFinishedSemaphores,
							   std::vector<VkFence>& inFlightFences);
		void updateDirectionalLightSpaceMatrixShadowMapUBO(ecs::components::directionalLight* directionalLightComponent, uint32_t currentLight);
		void updateDirectionalLightShadowMapMatrixUBO(uint32_t currentImage, ecs::components::transform* _transformComponent, uint32_t currentLight, u32 meshID);
		void updateSpotLightSpaceMatrixShadowMapUBO(ecs::components::spotLight* spotLightComponent,
																		 uint32_t currentLight);
		void updateSpotLightShadowMapMatrixUBO(uint32_t currentImage, ecs::components::transform* _transformComponent, uint32_t currentLight, u32 meshID);
		void updatePointLightShadowMapMatrixUBO(uint32_t currentImage, ecs::components::transform* _transformComponent, ecs::components::pointLight* pointLightComponent, uint32_t layer, unsigned int meshID);
		void updatePointLightShadowMapDataUBO(uint32_t currentImage, ecs::components::pointLight* pointLightComponent, float farPlane);
        void updateMatrixUniformBuffer(uint32_t currentImage, uint32_t offset, ecs::components::transform* _transformComponent,
									   unsigned int meshID, ecs::components::material* materialComponent);
		void updateViewPositionUniformBuffer(uint32_t currentImage, ecs::components::transform* transformComponent);
		void updateDirSpaceMatrix(uint32_t currentImage);
        void mainRenderDrawFrame();
		void directionalLightShadowMapDrawFrame();
		void spotLightShadowMapDrawFrame();
		void pointLightShadowMapDrawFrame();
		void directionalLightRecordCoomandBuffer(VkCommandBuffer& commandBuffer, uint32_t imageIndex);
		void spotLightRecordCommandBuffer(VkCommandBuffer& commandBuffer, uint32_t imageIndex);
		void pointLightRecordCommandBuffer(VkCommandBuffer& commandBuffer, uint32_t imageIndex);
        VkShaderModule createShaderModule(const std::vector<char>& code);
        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
        bool isDeviceSuitable(VkPhysicalDevice device);
        bool checkDeviceExtensionSupport(VkPhysicalDevice device);
        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
        std::vector<const char*> getRequiredExtensions();
        bool checkValidationLayerSupport();
        static std::vector<char> readFile(const std::string& filename);
        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
		[[nodiscard]] mat4* updateAnimationFrames(ecs::components::transform* _transformComponent, unsigned int meshID);
		mat4 computeModelMatrix(ecs::components::transform* _transformComponent);
		void setImageDebugObjectName(VK_Image image);
		void setDebugObjectNames();
    };

};

#endif
