// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#include "Components/RigidBodyComponent.hpp"
#include "Engine.hpp"
#include "SpritesData.hpp"
#include "Texture.hpp"

int main()
{
	using namespace GLVM;
	namespace cm  = GLVM::ecs::components;

	ecs::EntityManager   * EntityManager     = ecs::EntityManager::GetInstance();
	ecs::ComponentManager* ComponentManager  = ecs::ComponentManager::GetInstance();

	core::Engine* GLVM = core::Engine::GetInstance();
	[[maybe_unused]] cm::MeshHandle cubeHandle_OBJ = GLVM->LoadMeshFromFile_OBJ("../waveFrontObj/cube.obj");
	[[maybe_unused]] cm::MeshHandle coneHandle_OBJ = GLVM->LoadMeshFromFile_OBJ("../waveFrontObj/cone.obj");
	[[maybe_unused]] cm::MeshHandle icoSphereHandle_OBJ = GLVM->LoadMeshFromFile_OBJ("../waveFrontObj/ico_sphere.obj");
	[[maybe_unused]] cm::MeshHandle monkeyHandle_OBJ = GLVM->LoadMeshFromFile_OBJ("../waveFrontObj/monkey.obj");
	[[maybe_unused]] cm::MeshHandle uvSphereHandle_OBJ = GLVM->LoadMeshFromFile_OBJ("../waveFrontObj/uv_sphere.obj");
	[[maybe_unused]] cm::MeshHandle torusHandle_OBJ = GLVM->LoadMeshFromFile_OBJ("../waveFrontObj/torus.obj");
	[[maybe_unused]] cm::MeshHandle pipeHandle_OBJ = GLVM->LoadMeshFromFile_OBJ("../waveFrontObj/pipe.obj");
	[[maybe_unused]] cm::MeshHandle hyperCubeHandle_GLTF = GLVM->LoadMeshFromFile_GLTF("../gltf/hyper_cube.gltf");
	[[maybe_unused]] cm::MeshHandle megaChelHandle_GLTF = GLVM->LoadMeshFromFile_GLTF("../gltf/mega_chel.gltf");
	[[maybe_unused]] cm::MeshHandle simpleCubeHandle_GLTF = GLVM->LoadMeshFromFile_GLTF("../gltf/simpleCube2.gltf");

	[[maybe_unused]] ecs::TextureHandle glvmTextureHandle = GLVM->LoadTextureFromAddress(128, 128, glvm_dat_len, glvm_dat);
	[[maybe_unused]] ecs::TextureHandle sample1Texturehandle = GLVM->LoadTextureFromAddress(128, 128, sample1_dat_len, sample1_dat);
	[[maybe_unused]] ecs::TextureHandle sample2TextureHandle = GLVM->LoadTextureFromAddress(128, 128, sample2_dat_len, sample2_dat);

	/// Loading method with stb_image
	// [[maybe_unused]] ecs::TextureHandle chelikTextureHandle = GLVM->LoadTextureFromFile("../textures/data/glvm.png");
	// [[maybe_unused]] ecs::TextureHandle witchTexturehandle = GLVM->LoadTextureFromFile("../textures/data/sample1.png");
	// [[maybe_unused]] ecs::TextureHandle grayTextureHandle = GLVM->LoadTextureFromFile("../textures/data/sample2.png");
	
    Entity uiPlayer = EntityManager->CreateEntity();
    ComponentManager->CreateComponent<cm::mesh, cm::controller, cm::collider, cm::animation, cm::beholder,
		cm::transform, cm::rigidBody, cm::event>(uiPlayer);
	*ComponentManager->GetComponent<cm::transform>(uiPlayer) = { .tPosition = { 2.7f, 10.0f, 3.0f }, .fScale = 1.0f };
	*ComponentManager->GetComponent<cm::rigidBody>(uiPlayer) = { .fMass_ = 6.0f };
    *ComponentManager->GetComponent<cm::beholder>(uiPlayer) = { .forward = { 0.0f, 0.0f, -1.0f },
		.up = { 0.0f, 1.0f, 0.0f } };
    ComponentManager->GetComponent<cm::mesh>(uiPlayer)->handle = simpleCubeHandle_GLTF;
	
	Entity plain0 = EntityManager->CreateEntity();
	ComponentManager->CreateComponent<cm::material, cm::mesh, cm::transform, cm::collider>(plain0);
	*ComponentManager->GetComponent<cm::transform>(plain0) = { .tPosition = { 0.0f, -20.5f, 0.0f }, .yaw = 10.0f, .pitch = 0.0f, .fScale = 20.2f, .gltf = true };
    ComponentManager->GetComponent<cm::mesh>(plain0)->handle = hyperCubeHandle_GLTF;
	cm::material* materialPlain0  = ComponentManager->GetComponent<cm::material>(plain0);
	*materialPlain0 = { .diffuseTextureID_ = glvmTextureHandle, .specularTextureID_ = glvmTextureHandle, .ambient = { 0.05f, 0.05f, 0.0f },
		.shininess = 128.0f * 0.078125f };

	for ( u32 i = 0; i < 40; ++i ) {
	Entity uiWitch = EntityManager->CreateEntity();
	ComponentManager->CreateComponent<cm::material, cm::mesh, cm::collider, cm::transform>(uiWitch);
	*ComponentManager->GetComponent<cm::transform>(uiWitch) = { .tPosition = { (float)i, 10.0f, 0.0f },
		.yaw = 0.0f, .pitch = 0.0f, .fScale = 1.2f };
	ComponentManager->GetComponent<cm::mesh>(uiWitch)->handle = megaChelHandle_GLTF;
	cm::material* materialWitch  = ComponentManager->GetComponent<cm::material>(uiWitch);
	*materialWitch  = { .diffuseTextureID_ = sample1Texturehandle, .specularTextureID_ = sample1Texturehandle, .ambient = { 0.05f, 0.05f, 0.05f },
		.shininess = 128.0f * 0.078125f };
	}

 	Entity cube0 = EntityManager->CreateEntity();
	ComponentManager->CreateComponent<cm::material, cm::mesh, cm::collider, cm::transform>(cube0);
	*ComponentManager->GetComponent<cm::transform>(cube0) = { .tPosition = { 7.0f, 3.0f, 0.0f },
		.yaw = 0.0f, .pitch = 0.0f, .fScale = 1.0f };
    ComponentManager->GetComponent<cm::mesh>(cube0)->handle = monkeyHandle_OBJ;
	cm::material* materialCube0  = ComponentManager->GetComponent<cm::material>(cube0);
	*materialCube0  = { .diffuseTextureID_ = sample2TextureHandle, .specularTextureID_ = sample2TextureHandle, .ambient = { 0.05f, 0.05f, 0.05f },
		.shininess = 128.0f * 0.078125f };
	
	// Entity directionalLight0 = EntityManager->CreateEntity();
	// ComponentManager->CreateComponent<cm::mesh, cm::material, cm::directionalLight, cm::transform>(directionalLight0);
	// *ComponentManager->GetComponent<cm::directionalLight>(directionalLight0) = { .position = { 10.0f, 15.0f, -2.0f },
	// 	.direction = { -5.0f, -3.0f, 0.0f}, .ambient = { 0.05f, 0.05f, 0.05f }, .diffuse = {0.8f, 0.8f, 0.8f},
	// 	.specular = {1.0f, 1.0f, 1.0f}};
 	// *ComponentManager->GetComponent<cm::transform>(directionalLight0) = { .tPosition = { 10.0f, 15.0f, -2.0f },
	// 	.fScale = 0.2f };
	// ComponentManager->GetComponent<cm::mesh>(directionalLight0)->handle = hyperCubeHandle_GLTF;
	// cm::material* materialDirectionalLight0  = ComponentManager->GetComponent<cm::material>(directionalLight0);
	// *materialDirectionalLight0 = { .diffuseTextureID_ = grayTextureHandle, .specularTextureID_ = grayTextureHandle, .ambient = { 0.05f, 0.05f, 0.0f },
	// 	.shininess = 128.0f * 0.078125f };

	// Entity directionalLight1 = EntityManager->CreateEntity();
	// ComponentManager->CreateComponent<cm::mesh, cm::material, cm::directionalLight, cm::transform>(directionalLight1);
	// *ComponentManager->GetComponent<cm::directionalLight>(directionalLight1) = { .position = { 0.0f, 3.0f, 2.0f },
	// 	.direction = { 5.0f, -1.0f, 0.0f}, .ambient = { 0.05f, 0.05f, 0.05f }, .diffuse = {0.8f, 0.8f, 0.8f},
	// 	.specular = {1.0f, 1.0f, 1.0f}};
 	// *ComponentManager->GetComponent<cm::transform>(directionalLight1) = { .tPosition = { 0.0f, 3.0f, 2.0f },
	// 	.fScale = 0.2f };
	// ComponentManager->GetComponent<cm::mesh>(directionalLight1)->handle = hyperCubeHandle_GLTF;
	// cm::material* materialDirectionalLight1  = ComponentManager->GetComponent<cm::material>(directionalLight1);
	// *materialDirectionalLight1 = { .diffuseTextureID_ = grayTextureHandle, .specularTextureID_ = grayTextureHandle, .ambient = { 0.05f, 0.05f, 0.0f },
	// 	.shininess = 128.0f * 0.078125f };

	// Entity directionalLight2 = EntityManager->CreateEntity();
	// ComponentManager->CreateComponent<cm::mesh, cm::material, cm::directionalLight, cm::transform>(directionalLight2);
	// *ComponentManager->GetComponent<cm::directionalLight>(directionalLight2) = { .position = { 3.0f, 3.0f, 0.0f },
	// 	.direction = { 1.0f, -1.0f, -5.0f}, .ambient = { 0.05f, 0.05f, 0.05f }, .diffuse = {0.8f, 0.8f, 0.8f},
	// 	.specular = {1.0f, 1.0f, 1.0f}};
 	// *ComponentManager->GetComponent<cm::transform>(directionalLight2) = { .tPosition = { 3.0f, 3.0f, 0.0f },
	// 	.fScale = 0.2f };
	// ComponentManager->GetComponent<cm::mesh>(directionalLight2)->handle = hyperCubeHandle_GLTF;
	// cm::material* materialDirectionalLight2  = ComponentManager->GetComponent<cm::material>(directionalLight2);
	// *materialDirectionalLight2 = { .diffuseTextureID_ = grayTextureHandle, .specularTextureID_ = grayTextureHandle, .ambient = { 0.05f, 0.05f, 0.0f },
	// 	.shininess = 128.0f * 0.078125f };
	
	Entity pointLight0 = EntityManager->CreateEntity();
	ComponentManager->CreateComponent<cm::mesh, cm::material, cm::pointLight, cm::transform>(pointLight0);
	*ComponentManager->GetComponent<cm::pointLight>(pointLight0) = { .position = { 0.0f, 15.0f, 2.0f },
		.ambient = { 0.1f, 0.1f, 0.1f }, .diffuse = { 0.8f, 0.8f, 0.8f }, .specular = { 2.0f, 2.0f, 2.0f },
		.constant = 1.0f, .linear = 0.09f, .quadratic = 0.032f };
	*ComponentManager->GetComponent<cm::transform>(pointLight0) = { .tPosition = { 0.0f, 15.0f, 2.0f }, .fScale = 0.2f };
	ComponentManager->GetComponent<cm::mesh>(pointLight0)->handle = hyperCubeHandle_GLTF;
	cm::material* materialPointLight0   = ComponentManager->GetComponent<cm::material>(pointLight0);
	*materialPointLight0 = { .diffuseTextureID_ = glvmTextureHandle, .specularTextureID_ = glvmTextureHandle };

 	// Entity pointLight1 = EntityManager->CreateEntity();
	// ComponentManager->CreateComponent<cm::mesh, cm::material, cm::pointLight, cm::transform>(pointLight1);
	// *ComponentManager->GetComponent<cm::pointLight>(pointLight1)  = { .position = { 0.0f, 3.0f, 0.0f },
	// 	.ambient = { 0.2f, 0.2f, 0.2f }, .diffuse = { 0.7f, 0.7f, 0.7f }, .specular = { 0.8f, 0.8f, 0.8f },
	// 	.constant = 1.0f, .linear = 0.09f, .quadratic = 0.032f };
	// *ComponentManager->GetComponent<cm::transform>(pointLight1) = { .tPosition = { 0.0f, 3.0f, 0.0f }, .fScale = 0.3f };
	// ComponentManager->GetComponent<cm::mesh>(pointLight1)->handle = hyperCubeHandle_GLTF;
	// cm::material* materialPointLight1 = ComponentManager->GetComponent<cm::material>(pointLight1);
	// *materialPointLight1 = { .diffuseTextureID_ = container2Texturehandle, .specularTextureID_ = container2Texturehandle };

	// Entity pointLight2 = EntityManager->CreateEntity();
	// ComponentManager->CreateComponent<cm::mesh, cm::material, cm::pointLight, cm::transform>(pointLight2);
	// *ComponentManager->GetComponent<cm::pointLight>(pointLight2)  = { .position = { 0.0f, 3.0f, 2.0f },
	// 	.ambient = { 0.2f, 0.2f, 0.2f }, .diffuse = { 0.7f, 0.7f, 0.7f }, .specular = { 0.8f, 0.8f, 0.8f },
	// 	.constant = 1.0f, .linear = 0.09f, .quadratic = 0.032f };
	// *ComponentManager->GetComponent<cm::transform>(pointLight2) = { .tPosition = { 0.0f, 3.0f, 2.0f }, .fScale = 0.3f };
	// ComponentManager->GetComponent<cm::mesh>(pointLight2)->handle = hyperCubeHandle_GLTF;
	// cm::material* materialPointLight2 = ComponentManager->GetComponent<cm::material>(pointLight2);
	// *materialPointLight2 = { .diffuseTextureID_ = container2Texturehandle, .specularTextureID_ = container2Texturehandle };

	// Entity pointLight3 = EntityManager->CreateEntity();
	// ComponentManager->CreateComponent<cm::mesh, cm::material, cm::pointLight, cm::transform>(pointLight3);
	// *ComponentManager->GetComponent<cm::pointLight>(pointLight3)  = { .position = { 2.0f, 3.0f, 0.0f },
	// 	.ambient = { 0.2f, 0.2f, 0.2f }, .diffuse = { 0.7f, 0.7f, 0.7f }, .specular = { 0.8f, 0.8f, 0.8f },
	// 	.constant = 1.0f, .linear = 0.09f, .quadratic = 0.032f };
	// *ComponentManager->GetComponent<cm::transform>(pointLight3) = { .tPosition = { 2.0f, 3.0f, 0.0f }, .fScale = 0.3f };
	// ComponentManager->GetComponent<cm::mesh>(pointLight3)->handle = hyperCubeHandle_GLTF;
	// cm::material* materialPointLight3 = ComponentManager->GetComponent<cm::material>(pointLight3);
	// *materialPointLight3 = { .diffuseTextureID_ = container2Texturehandle, .specularTextureID_ = container2Texturehandle };

	// Entity pointLight4 = EntityManager->CreateEntity();
	// ComponentManager->CreateComponent<cm::mesh, cm::material, cm::pointLight, cm::transform>(pointLight4);
	// *ComponentManager->GetComponent<cm::pointLight>(pointLight4)  = { .position = { 0.27f, 5.3f, 0.25f },
	// 	.ambient = { 0.2f, 0.2f, 0.2f }, .diffuse = { 0.7f, 0.7f, 0.7f }, .specular = { 0.8f, 0.8f, 0.8f },
	// 	.constant = 2.17f, .linear = 0.39f, .quadratic = 0.532f };
	// *ComponentManager->GetComponent<cm::transform>(pointLight4) = { .tPosition = { 0.5f, 3.0f, 0.8f }, .fScale = 0.3f };
	// ComponentManager->GetComponent<cm::mesh>(pointLight4)->handle = hyperCubeHandle_GLTF;
	// cm::material* materialPointLight4 = ComponentManager->GetComponent<cm::material>(pointLight4);
	// *materialPointLight4 = { .diffuseTextureID_ = container2Texturehandle, .specularTextureID_ = container2Texturehandle };
	
	// Entity spotLight1 = EntityManager->CreateEntity();
	// ComponentManager->CreateComponent<cm::mesh, cm::material, cm::spotLight, cm::transform>(spotLight1);
	// *ComponentManager->GetComponent<cm::spotLight>(spotLight1) = { .position = { 1.0f, 15.0f, -5.0f },
	// 	.direction = { 0.0f, -1.0f, 1.0f }, .cutOff = 32.5f, .outerCutOff = 37.5f, .ambient = { 0.05f, 0.05f, 0.05f },
	// 	.diffuse = { 1.8f, 1.8f, 1.8f }, .specular = { 2.0f, 2.0f, 2.0f }, .constant = 1.0f, .linear = 0.09f,
	// 	.quadratic = 0.032f };
	// *ComponentManager->GetComponent<cm::transform>(spotLight1) = { .tPosition = { 1.0f, 15.0f, -5.0f }, .fScale = 0.5f };
	// ComponentManager->GetComponent<cm::mesh>(spotLight1)->handle = simpleCubeHandle_GLTF;
	// cm::material* materialSpotLight1   = ComponentManager->GetComponent<cm::material>(spotLight1);
	// *materialSpotLight1 = { .diffuseTextureID_ = grayTextureHandle, .specularTextureID_ = grayTextureHandle };

	// Entity spotLight2 = EntityManager->CreateEntity();
	// ComponentManager->CreateComponent<cm::mesh, cm::material, cm::spotLight, cm::transform>(spotLight2);
	// *ComponentManager->GetComponent<cm::spotLight>(spotLight2) = { .position = { 0.0f, 3.0f, 10.0f },
	// 	.direction = { 0.0f, 0.0f, -5.0f }, .cutOff = 32.5f, .outerCutOff = 37.5f, .ambient = { 0.05f, 0.05f, 0.05f },
	// 	.diffuse = { 0.8f, 0.8f, 0.8f }, .specular = { 1.0f, 1.0f, 1.0f }, .constant = 1.0f, .linear = 0.09f,
	// 	.quadratic = 0.032f };
	// *ComponentManager->GetComponent<cm::transform>(spotLight2) = { .tPosition = { 0.0f, 3.0f, 10.0f }, .fScale = 1.0f };
	// ComponentManager->GetComponent<cm::mesh>(spotLight2)->handle = simpleCubeHandle_GLTF;
	// cm::material* materialSpotLight2   = ComponentManager->GetComponent<cm::material>(spotLight2);
	// *materialSpotLight2 = { .diffuseTextureID_ = grayTextureHandle, .specularTextureID_ = grayTextureHandle };

	
    ///< Game rendering loop
//	GLVM->GameLoop(GLVM::core::OPENGL_RENDERER);
	GLVM->GameLoop(GLVM::core::VULKAN_RENDERER);

	GLVM->GameKill();

    return 0;
}
