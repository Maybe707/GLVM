// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#include "GraphicAPI/Opengl.hpp"
#include "ComponentManager.hpp"
#include "Components/ColliderComponent.hpp"
#include "Components/DirectionalLightComponent.hpp"
#include "Components/MaterialComponent.hpp"
#include "Components/PointLightComponent.hpp"
#include "Components/RigidBodyComponent.hpp"
#include "Components/SpotLightComponent.hpp"
#include "Constants.hpp"
#include "Engine.hpp"
#include "Event.hpp"
#include "Components/MaterialComponent.hpp"
#include "Components/TransformComponent.hpp"
#include "GLPointer.h"
#include "GraphicAPI/Vulkan.hpp"
#include "JsonParser.hpp"
#include "MeshManager.hpp"
#include "ShaderProgram.hpp"
#include "Texture.hpp"
#include "TextureManager.hpp"
#include "ToString.hpp"
#include "Vector.hpp"
#include "Components/VertexComponent.hpp"
#include "VertexData.hpp"
#include "VertexMath.hpp"
#include "Components/ViewComponent.hpp"
#include <GL/gl.h>
#include <GL/glext.h>
#include <cmath>
#include "Globals.hpp"
#include "WavefrontObjParser.hpp"

#include <chrono>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <math.h>
#include <ratio>
#include <sstream>
#include <thread>

namespace GLVM::core
{
    COpenglRenderer::COpenglRenderer()
	{
		coreShaderProgram           = new Shader("../GLshaders/CoreShader.vert", "../GLshaders/CoreShader.frag");
		flatShadowMapShaderProgram  = new Shader("../GLshaders/FlatShadowMap.vert", "../GLshaders/FlatShadowMap.frag");
 		cubeShadowMapShaderProgram  = new Shader("../GLshaders/CubeShadowMap.vert", "../GLshaders/CubeShadowMap.frag",
			                                     "../GLshaders/CubeShadowMap.geom");
		debugQuadDepth_             = new Shader("../GLshaders/DebugQuadDepth.vert", "../GLshaders/DebugQuadDepth.frag");
		debugLines                  = new Shader("../GLshaders/debugLines.vert", "../GLshaders/debugLines.frag");
		
        glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);

		debugQuadDepth_->Use();
		debugQuadDepth_->SetInt("depthMap", 31);
		
		coreShaderProgram->Use();
		coreShaderProgram->SetInt("material.diffuse", 28);
		coreShaderProgram->SetInt("material.specular", 29);
		
		AllocateTextureMemory(pointLightCubeShadowMapFBOcontainer, pointLightCubeShadowMapTextureContainer,
							  GL_TEXTURE_CUBE_MAP, GL_CLAMP_TO_EDGE, 16, "pointLightCubeShadowMapArray", 0);
		AllocateTextureMemory(spotLightFlatShadowMapFBOContainer, spotLightFlatShadowMapTextureContainer,
							  GL_TEXTURE_2D, GL_CLAMP_TO_BORDER, 8, "spotLightFlatShadowMapArray", 16);
		AllocateTextureMemory(directionalLightFlatShadowMapFBOcontainer, directionalLightFlatShadowMapTextureContainer,
							  GL_TEXTURE_2D, GL_CLAMP_TO_BORDER, 4, "directionalLightFlatShadowMapArray", 24);

		glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	}
	
	COpenglRenderer::~COpenglRenderer()
	{
        delete coreShaderProgram;
        coreShaderProgram = nullptr;
		delete flatShadowMapShaderProgram;
		flatShadowMapShaderProgram = nullptr;
		delete cubeShadowMapShaderProgram;
		cubeShadowMapShaderProgram = nullptr;

		for (unsigned int i = 0; i < VBOcontainer_.size(); ++i)
			pGLDelete_Buffers(NUMBER_OF_CREATING_VBO_OBJECT_1, &VBOcontainer_[i]);
		
		for (unsigned int i = 0; i < VAOcontainer_.size(); ++i)
			pGLDelete_Vertex_Arrays(NUMBER_OF_CREATING_VAO_OBJECT_1, &VAOcontainer_[i]);
		
        pGLDelete_Buffers(NUMBER_OF_CREATING_VBO_OBJECT_1, &quadVBO_);
		pGLDelete_Vertex_Arrays(NUMBER_OF_CREATING_VAO_OBJECT_1, &quadVAO_);
	}
    
	void COpenglRenderer::draw() {
		using namespace GLVM;
		namespace cm = GLVM::ecs::components;

		ecs::ComponentManager* pComponent_Manager = ecs::ComponentManager::GetInstance();

		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		coreShaderProgram->Use();
		
		core::vector<unsigned int>* pEntityContainerRefDirectionalLight =
			pComponent_Manager->GetEntityContainer<cm::directionalLight>();
		unsigned int appropriateDirectionalLightComponentIndex = 0;
		sampledDirectionalLightEntityIDcontainer.clear();
		mat4 directionalProjectionMatrixLight = ortho(-10.0f, 10.0f, -10.0f, 10.0f,
													  nearPlaneFlatShadowMap, farPlaneFlatShadowMap);
		for ( unsigned int i = 0; i < pEntityContainerRefDirectionalLight->GetSize(); ++i ) {
			unsigned int uiDirectionalLightsEntity = (*pEntityContainerRefDirectionalLight)[i];
			cm::directionalLight* directionalLightComponent = pComponent_Manager->
				GetComponent<cm::directionalLight>(uiDirectionalLightsEntity);

			directionalLightSpaceMatrixContainer[appropriateDirectionalLightComponentIndex] =
				EvaluateFlatShadowMap(directionalLightFlatShadowMapFBOcontainer[i],
									  *directionalLightComponent,directionalProjectionMatrixLight) ;

			sampledDirectionalLightEntityIDcontainer.push_back(i);
			coreShaderProgram->Use();
			coreShaderProgram->SetInt(ConcatIntBetweenTwoStrings("sampledShadowOrdinalNumbers[",
																 appropriateDirectionalLightComponentIndex, "]"), i);
			
			++appropriateDirectionalLightComponentIndex;
		}

		coreShaderProgram->SetInt("sampledDirectionalShadowOrdinalNumbersArraySize",
								  sampledDirectionalLightEntityIDcontainer.size());
		
		core::vector<unsigned int>* pEntityContainerRefSpotLight =
			pComponent_Manager->GetEntityContainer<cm::spotLight>();
		unsigned int appropriateSpotLightComponentIndex = 0;
		sampledSpotLightEntityIDcontainer.clear();
		mat4 spotProjectionMatrixLight = Perspective(Radians(90.0f),
													 (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT,
													 nearPlaneFlatShadowMap, farPlaneFlatShadowMap);
		
		for ( unsigned int i = 0; i < pEntityContainerRefSpotLight->GetSize(); ++i ) {
			unsigned int uiSpotLightsEntity = (*pEntityContainerRefSpotLight)[i];
			cm::spotLight* spotLightComponent = pComponent_Manager->GetComponent<cm::spotLight>(uiSpotLightsEntity);

			spotLightSpaceMatrixContainer[appropriateSpotLightComponentIndex] =
				EvaluateFlatShadowMap(spotLightFlatShadowMapFBOContainer[i],
									  *spotLightComponent,spotProjectionMatrixLight) ;

			sampledSpotLightEntityIDcontainer.push_back(i);
			coreShaderProgram->Use();
			coreShaderProgram->SetInt(ConcatIntBetweenTwoStrings("spotLightFlatShadowMapComponentIndices[",
																 appropriateSpotLightComponentIndex, "]"), i);
			++appropriateSpotLightComponentIndex;
		}

		coreShaderProgram->SetInt("sampledSpotShadowOrdinalNumbersArraySize",
								  sampledSpotLightEntityIDcontainer.size());
		
		// core::vector<unsigned int>* pEntityContainerRefView =
		// 	pComponent_Manager->GetEntityContainer<cm::beholder>();
		// unsigned int uiPlayerEntity = (*pEntityContainerRefView)[0];
		// cm::transform* playerTransformComponent = pComponent_Manager->GetComponent<cm::transform>(uiPlayerEntity);
		core::vector<unsigned int>* pEntityContainerRefPointLight =
			pComponent_Manager->GetEntityContainer<cm::pointLight>();
		unsigned int pointLightComponentContainerSize = pEntityContainerRefPointLight->GetSize();

		sampledPointLightEntityIDcontainer.clear();
		unsigned int appropriatePointLightComponentIndex = 0;
		for ( unsigned int i = 0; i < pointLightComponentContainerSize; ++i ) {
			unsigned int entityID = (*pEntityContainerRefPointLight)[i];
			cm::pointLight* pointLightComponent = pComponent_Manager->GetComponent<cm::pointLight>(entityID);
//			float distance = VectorLength(playerTransformComponent->tPosition, pointLightComponent->position);

//			if ( distance < 4.5f ) {
				glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				sampledPointLightEntityIDcontainer.push_back(i);
				
				EvaluateCubeShadowMap(pointLightCubeShadowMapFBOcontainer[appropriatePointLightComponentIndex],
									  *pointLightComponent);
				
				coreShaderProgram->Use();
				coreShaderProgram->SetInt(ConcatIntBetweenTwoStrings("pointLightCubeShadowMapComponentIndices[",
																	 appropriatePointLightComponentIndex, "]"), i);
				++appropriatePointLightComponentIndex;
//			}
		}
		
		coreShaderProgram->SetInt("sampledPointShadowOrdinalNumbersArraySize",
								  sampledPointLightEntityIDcontainer.size());
		
		ComputeDirectionalLight();
		ComputePointLight();
		ComputeSpotLight();

	    EvaluateCoreShader();
		RenderScene(coreShaderProgram);
	}

	void COpenglRenderer::AllocateTextureMemory(std::vector<unsigned int>& shadowMapFBOcontainer,
												std::vector<unsigned int>& shadowMapTextureContainer,
												GLenum textureTarget, GLint clampType,
												unsigned int lightSourceNumber,
												std::string shadowMapArrayType,
		                                        unsigned int textureUnitBaseIndex) {
		for ( unsigned int i = 0; i < lightSourceNumber; ++i ) {
			shadowMapFBOcontainer.emplace_back();
			shadowMapTextureContainer.emplace_back();
			InitializeShadowMapData(shadowMapFBOcontainer[i], shadowMapTextureContainer[i],
									textureTarget, clampType);
		}
		unsigned int lightCounter = 0;
		int* lightArrayUniformIndices = new int[lightSourceNumber];
		for ( unsigned int i = textureUnitBaseIndex; i < textureUnitBaseIndex + lightSourceNumber; ++i ) {
			lightArrayUniformIndices[lightCounter] = i;
			++lightCounter;
		}

		coreShaderProgram->Use();
		coreShaderProgram->SetInt(shadowMapArrayType, lightSourceNumber, lightArrayUniformIndices);
		delete [] lightArrayUniformIndices;
		lightArrayUniformIndices = nullptr;
	}
	
	void COpenglRenderer::InitializeShadowMapData(unsigned int& fbo_, unsigned int& texture_, GLenum textureTarget_,
												  GLint clampType_) {
				pGLGen_Framebuffers(1, &fbo_);
				glGenTextures(1, &texture_);
				pGLActive_Texture(GL_TEXTURE0);
				glBindTexture(textureTarget_, texture_);
				AllocateTexture(textureTarget_, clampType_);
				
				// Attach depth texture as FBO's depth buffer
				pGLBind_Framebuffer(GL_FRAMEBUFFER, fbo_);
				pGLFramebuffer_Texture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture_, 0);
				glDrawBuffer(GL_NONE);
				glReadBuffer(GL_NONE);
				pGLBind_Framebuffer(GL_FRAMEBUFFER, 0);
	}

	void COpenglRenderer::AllocateTexture ( GLenum textureTarget_, GLint clampType_ ) {
		switch ( textureTarget_ ) {
		case GL_TEXTURE_2D:
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clampType_); 
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clampType_);
			glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
			break;
		case GL_TEXTURE_CUBE_MAP:
			for (unsigned int j = 0; j < 6; ++j)
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, clampType_); 
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, clampType_);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, clampType_);
		break;
		default:
			break;
		}
	}

	mat4 COpenglRenderer::EvaluateFlatShadowMap(unsigned int& shadowMapFBO, ecs::components::directionalLight& directionalLightComponent, mat4 projectionMatrixLight) {
		vec3 positionVectorLight  = directionalLightComponent.position;
		vec3 directionVectorLight = directionalLightComponent.direction;
		mat4 viewMatrixLight = LookAtMain(positionVectorLight,
										  directionVectorLight,
										  { 0.0f, 1.0f, 0.0f });
		mat4 lightSpaceMatrix = viewMatrixLight * projectionMatrixLight;
		// Render scene from light's point of view
		flatShadowMapShaderProgram->Use();
		flatShadowMapShaderProgram->SetMat4("lightSpaceMatrix", lightSpaceMatrix);
		// Render to depth map
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		pGLBind_Framebuffer(GL_FRAMEBUFFER, shadowMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		RenderScene(flatShadowMapShaderProgram);
		pGLBind_Framebuffer(GL_FRAMEBUFFER, 0);

		return lightSpaceMatrix;
	}
 
	mat4 COpenglRenderer::EvaluateFlatShadowMap(unsigned int& shadowMapFBO, ecs::components::spotLight& directionalLightComponent, mat4 projectionMatrixLight) {
			vec3 positionVectorLight = directionalLightComponent.position;
			vec3 directionVectorLight = directionalLightComponent.direction;
			mat4 viewMatrixLight = LookAtMain(positionVectorLight,
														 directionVectorLight,
														 { 0.0f, 1.0f, 0.0f });
			mat4 lightSpaceMatrix = viewMatrixLight * projectionMatrixLight;
			// Render scene from light's point of view
			flatShadowMapShaderProgram->Use();
			flatShadowMapShaderProgram->SetMat4("lightSpaceMatrix", lightSpaceMatrix);
			// Render to depth map
			glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
			pGLBind_Framebuffer(GL_FRAMEBUFFER, shadowMapFBO);
			glClear(GL_DEPTH_BUFFER_BIT);
			RenderScene(flatShadowMapShaderProgram);
			pGLBind_Framebuffer(GL_FRAMEBUFFER, 0);

			return lightSpaceMatrix;
	}
	
	void COpenglRenderer::EvaluateCubeShadowMap(unsigned int& shadowMapFBO, ecs::components::pointLight& pointLightComponent) {
				vec3 positionVectorPointLight = pointLightComponent.position;
				mat4 projectionMatrixCubeShadowMap = Perspective(Radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, nearPlaneCubeShadowMap, farPlaneCubeShadowMap);
				vector<mat4> cubeShadowMapTransforms;
				cubeShadowMapTransforms.Push(LookAtMain(positionVectorPointLight, positionVectorPointLight + vec3( 1.0f,  0.0f,  0.0f), vec3(0.0f, -1.0f,  0.0f)) * projectionMatrixCubeShadowMap);
				cubeShadowMapTransforms.Push(LookAtMain(positionVectorPointLight, positionVectorPointLight + vec3( -1.0f,  0.0f,  0.0f), vec3(0.0f, -1.0f,  0.0f)) * projectionMatrixCubeShadowMap);
				cubeShadowMapTransforms.Push(LookAtMain(positionVectorPointLight, positionVectorPointLight + vec3( 0.0f,  1.0f,  0.0f), vec3(0.0f, 0.0f,  1.0f)) * projectionMatrixCubeShadowMap);
				cubeShadowMapTransforms.Push(LookAtMain(positionVectorPointLight, positionVectorPointLight + vec3( 0.0f,  -1.0f,  0.0f), vec3(0.0f, 0.0f,  -1.0f)) * projectionMatrixCubeShadowMap);
				cubeShadowMapTransforms.Push(LookAtMain(positionVectorPointLight, positionVectorPointLight + vec3( 0.0f,  0.0f,  1.0f), vec3(0.0f, -1.0f,  0.0f)) * projectionMatrixCubeShadowMap);
				cubeShadowMapTransforms.Push(LookAtMain(positionVectorPointLight, positionVectorPointLight + vec3( 0.0f,  0.0f,  -1.0f), vec3(0.0f, -1.0f,  0.0f)) * projectionMatrixCubeShadowMap);

				glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
				pGLBind_Framebuffer(GL_FRAMEBUFFER, shadowMapFBO);
				glClear(GL_DEPTH_BUFFER_BIT);
				cubeShadowMapShaderProgram->Use();
				for (unsigned int j = 0; j < 6; ++j)
					cubeShadowMapShaderProgram->SetMat4("shadowMatrices[" + std::to_string(j) + "]", cubeShadowMapTransforms[j]);
				cubeShadowMapShaderProgram->SetFloat("farPlane", farPlaneCubeShadowMap);
				cubeShadowMapShaderProgram->SetVec3("lightPosition", positionVectorPointLight);
				RenderScene(cubeShadowMapShaderProgram);
				pGLBind_Framebuffer(GL_FRAMEBUFFER, 0);
	}

	void COpenglRenderer::EvaluateCoreShader() {
		namespace cm = GLVM::ecs::components;
        ecs::ComponentManager* pComponent_Manager = ecs::ComponentManager::GetInstance();
		core::vector<unsigned int>* pEntityContainerRefView =
			pComponent_Manager->GetEntityContainer<cm::beholder>();
		unsigned int uiPlayerEntity = (*pEntityContainerRefView)[0];
		cm::beholder* playerViewComponent = pComponent_Manager->GetComponent<cm::beholder>(uiPlayerEntity);
		cm::transform* playerTransformComponent = pComponent_Manager->GetComponent<cm::transform>(uiPlayerEntity);
		
		vec3 viewPosition = playerTransformComponent->tPosition;
		bool reverseNormalsFlag = false;
		
		// Render scene as normal
		glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ComputeProjectionMatrix(coreShaderProgram);
		ComputeViewMatrix(coreShaderProgram, *playerTransformComponent, *playerViewComponent);
		coreShaderProgram->SetInt("shadows", shadows);
		coreShaderProgram->SetBool("reverseNormals", reverseNormalsFlag);
		coreShaderProgram->SetFloat("farPlane", farPlaneCubeShadowMap);
		coreShaderProgram->SetVec3("viewPosition", viewPosition);

		coreShaderProgram->SetInt("directionalLightSpaceMatrixContainerSize",
								  sampledDirectionalLightEntityIDcontainer.size());
		coreShaderProgram->SetMat4("directionalLightSpaceMatrixContainer",
								   sampledDirectionalLightEntityIDcontainer.size(),
								   directionalLightSpaceMatrixContainer[0]);
		coreShaderProgram->SetInt("spotLightSpaceMatrixContainerSize", sampledSpotLightEntityIDcontainer.size());
		coreShaderProgram->SetMat4("spotLightSpaceMatrixContainer", sampledSpotLightEntityIDcontainer.size(),
								   spotLightSpaceMatrixContainer[0]);
		coreShaderProgram->SetInt("spotLightArraySize", sampledSpotLightEntityIDcontainer.size());

		for ( unsigned int i = 0; i < sampledPointLightEntityIDcontainer.size(); ++i ) {
			pGLActive_Texture( GL_TEXTURE0 + i );
			glBindTexture( GL_TEXTURE_CUBE_MAP, pointLightCubeShadowMapTextureContainer[i] );
		}

		for ( unsigned int i = 0; i < sampledSpotLightEntityIDcontainer.size(); ++i ) {
			pGLActive_Texture( GL_TEXTURE16 + i );
			glBindTexture( GL_TEXTURE_2D, spotLightFlatShadowMapTextureContainer[i] );
		}

		for ( unsigned int i = 0; i < sampledDirectionalLightEntityIDcontainer.size(); ++i ) {
			pGLActive_Texture( GL_TEXTURE24 + i );
			glBindTexture( GL_TEXTURE_2D, directionalLightFlatShadowMapTextureContainer[i] );
		}
	}

	void COpenglRenderer::EvaluateFlatDebugShader() {
		glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		debugQuadDepth_->Use();
		debugQuadDepth_->SetFloat("nearPlane", nearPlaneFlatShadowMap);
		debugQuadDepth_->SetFloat("farPlane", farPlaneFlatShadowMap);
		pGLActive_Texture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, directionalLightFlatShadowMapTextureContainer[0]);
	}
	
	void COpenglRenderer::ComputeDirectionalLight() {
		namespace cm = GLVM::ecs::components;
		ecs::ComponentManager* pComponent_Manager = GLVM::ecs::ComponentManager::GetInstance();
		core::vector<unsigned int>* pEntityContainerRefDirectionalLight =
			pComponent_Manager->GetEntityContainer<cm::directionalLight>();
		unsigned int directionalLightComponentContainerSize = pEntityContainerRefDirectionalLight->GetSize();

		coreShaderProgram->SetInt("directionalLightsArraySize", directionalLightComponentContainerSize);
		for(unsigned int x = 0; x < directionalLightComponentContainerSize; ++x) {
			unsigned int uiDirectionalLightEntity = (*pEntityContainerRefDirectionalLight)[x];
			cm::directionalLight* directionalLightComponent = pComponent_Manager->GetComponent<cm::directionalLight>(uiDirectionalLightEntity);
			std::string leftString = "directionalLights[";
			coreShaderProgram->SetVec3(ConcatIntBetweenTwoStrings(leftString, x, "].position"),
									   directionalLightComponent->position);
			coreShaderProgram->SetVec3(ConcatIntBetweenTwoStrings(leftString, x, "].direction"),
										directionalLightComponent->direction);
			coreShaderProgram->SetVec3(ConcatIntBetweenTwoStrings(leftString, x, "].ambient"),
									   directionalLightComponent->ambient);
			coreShaderProgram->SetVec3(ConcatIntBetweenTwoStrings(leftString, x, "].diffuse"),
									   directionalLightComponent->diffuse); // darken diffuse light a bit
			coreShaderProgram->SetVec3(ConcatIntBetweenTwoStrings(leftString, x, "].specular"),
									   directionalLightComponent->specular);

		}
	}

	void COpenglRenderer::ComputePointLight() {
		namespace cm = GLVM::ecs::components;
		ecs::ComponentManager* pComponent_Manager = GLVM::ecs::ComponentManager::GetInstance();
		core::vector<unsigned int>* pEntityContainerRefPointLight =
			pComponent_Manager->GetEntityContainer<cm::pointLight>();
		unsigned int pointLightComponentContainerSize = pEntityContainerRefPointLight->GetSize();
		coreShaderProgram->SetInt("pointLightsArraySize", pointLightComponentContainerSize);
		for(unsigned int x = 0; x < pointLightComponentContainerSize; ++x) {
			unsigned int uiPointLightEntity = (*pEntityContainerRefPointLight)[x];
			cm::pointLight* pointLightComponent = pComponent_Manager->GetComponent<cm::pointLight>(uiPointLightEntity);

			std::string leftString = "pointLights[";
			coreShaderProgram->SetVec3(ConcatIntBetweenTwoStrings(leftString, x, "].position"),
									   pointLightComponent->position);
			coreShaderProgram->SetVec3(ConcatIntBetweenTwoStrings(leftString, x, "].ambient"),
									   pointLightComponent->ambient);
			coreShaderProgram->SetVec3(ConcatIntBetweenTwoStrings(leftString, x, "].diffuse"),
									   pointLightComponent->diffuse); // darken diffuse light a bit
			coreShaderProgram->SetVec3(ConcatIntBetweenTwoStrings(leftString, x, "].specular"),
									   pointLightComponent->specular);
			coreShaderProgram->SetFloat(ConcatIntBetweenTwoStrings(leftString, x, "].constant"),
										pointLightComponent->constant);
			coreShaderProgram->SetFloat(ConcatIntBetweenTwoStrings(leftString, x, "].linear"),
										pointLightComponent->linear);
			coreShaderProgram->SetFloat(ConcatIntBetweenTwoStrings(leftString, x, "].quadratic"),
										pointLightComponent->quadratic);
		}
	}

	void COpenglRenderer::ComputeSpotLight() {
		namespace cm = GLVM::ecs::components;
		ecs::ComponentManager* pComponent_Manager = GLVM::ecs::ComponentManager::GetInstance();
		core::vector<unsigned int>* pEntityContainerRefSpotLight =
			pComponent_Manager->GetEntityContainer<cm::spotLight>();
		unsigned int spotLightComponentContainerSize = pEntityContainerRefSpotLight->GetSize();
		coreShaderProgram->SetInt("spotLightsArraySize", spotLightComponentContainerSize);
		for(unsigned int x = 0; x < spotLightComponentContainerSize; ++x) {
			unsigned int uiSpotLightEntity = (*pEntityContainerRefSpotLight)[x];
			cm::spotLight* spotLightComponent = pComponent_Manager->GetComponent<cm::spotLight>(uiSpotLightEntity);
			std::string leftString = "spotLights[";
			coreShaderProgram->SetVec3(ConcatIntBetweenTwoStrings(leftString, x, "].position"),
									   spotLightComponent->position);
			coreShaderProgram->SetVec3(ConcatIntBetweenTwoStrings(leftString, x, "].direction"),
									   spotLightComponent->direction);
			coreShaderProgram->SetFloat(ConcatIntBetweenTwoStrings(leftString, x, "].cutOff"),
										std::cos(Radians(spotLightComponent->cutOff)));
			coreShaderProgram->SetFloat(ConcatIntBetweenTwoStrings(leftString, x, "].outerCutOff"),
										std::cos(Radians(spotLightComponent->outerCutOff)));
			coreShaderProgram->SetVec3(ConcatIntBetweenTwoStrings(leftString, x, "].ambient"),
									   spotLightComponent->ambient);
			coreShaderProgram->SetVec3(ConcatIntBetweenTwoStrings(leftString, x, "].diffuse"),
									   spotLightComponent->diffuse); // darken diffuse light a bit
			coreShaderProgram->SetVec3(ConcatIntBetweenTwoStrings(leftString, x, "].specular"),
									   spotLightComponent->specular);
			coreShaderProgram->SetFloat(ConcatIntBetweenTwoStrings(leftString, x, "].constant"),
										spotLightComponent->constant);
			coreShaderProgram->SetFloat(ConcatIntBetweenTwoStrings(leftString, x, "].linear"),
										spotLightComponent->linear);
			coreShaderProgram->SetFloat(ConcatIntBetweenTwoStrings(leftString, x, "].quadratic"),
										spotLightComponent->quadratic);
		}
	}
	
	void COpenglRenderer::RenderScene(Shader* shaderProgram_) {
		namespace cm = GLVM::ecs::components;
		ecs::ComponentManager* pComponent_Manager = GLVM::ecs::ComponentManager::GetInstance();
		mat4 modelMatrix(1.0f);

		Raycasting();


		

		namespace cm = GLVM::ecs::components;
		core::vector<Entity> linkedEntities      = pComponent_Manager->collectLinkedEntities<cm::transform,
																							 cm::material,
																							 cm::mesh>();

		unsigned int linkedEntitiesVectorSize      = linkedEntities.GetSize();			

		for(unsigned int i = 0; i < linkedEntitiesVectorSize; ++i) {
			unsigned int uiEntity_refTexture = linkedEntities[i];

			cm::mesh* vertexComponent = pComponent_Manager->GetComponent<cm::mesh>(uiEntity_refTexture);
			unsigned int uiVertexId = 0;
			if ( vertexComponent != nullptr )
				uiVertexId = vertexComponent->handle.id;

			cm::material* material = pComponent_Manager->GetComponent<cm::material>(uiEntity_refTexture);
			unsigned int diffuseTextureID  = 0;
			unsigned int specularTextureID = 0;
			if ( material != nullptr ) {
				diffuseTextureID  = material->diffuseTextureID_.id;
				specularTextureID = material->specularTextureID_.id;
			}

			cm::mesh* mesh = pComponent_Manager->GetComponent<cm::mesh>(uiEntity_refTexture);
			unsigned int meshID = mesh->handle.id;
			cm::transform* _transformComponent = pComponent_Manager->GetComponent<cm::transform>(uiEntity_refTexture);
			
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

				for ( unsigned int i = joinMatricesDataSize; i < MAX_JOINTS_NUMBER; ++i ) {
					mat4 unitMatrix(1.0f);
					jointMatricesData[i] = unitMatrix;
				}
			}
		
			shaderProgram_->SetMat4("jointMatrices", MAX_JOINTS_NUMBER, jointMatricesData[0]);
			delete [] jointMatricesData;
			jointMatricesData = nullptr;
			
			cm::transform* transformComponent = pComponent_Manager->GetComponent<cm::transform>(uiEntity_refTexture);
			if ( transformComponent != nullptr )
				modelMatrix = SetModelMatrix(*transformComponent);

			shaderProgram_->SetMat4("modelMatrix", modelMatrix);
			pGLActive_Texture(GL_TEXTURE28);
			glBindTexture(GL_TEXTURE_2D, textureVector[diffuseTextureID].iTexture_);
			pGLActive_Texture(GL_TEXTURE29);
			glBindTexture(GL_TEXTURE_2D, textureVector[specularTextureID].iTexture_);
			pGLBind_Vertex_Array(VAOcontainer_[uiVertexId]);
			cm::material* materialComponent = pComponent_Manager->GetComponent<cm::material>(uiEntity_refTexture);
			shaderProgram_->SetFloat("material.shininess", materialComponent->shininess);
			shaderProgram_->SetVec3("material.ambient",  materialComponent->ambient[0], materialComponent->ambient[1], materialComponent->ambient[2]);
			glDrawElements(GL_TRIANGLES, aIndices_[uiVertexId].size(), GL_UNSIGNED_INT, 0);
		}
	}

	void COpenglRenderer::Raycasting() {
		namespace cm = GLVM::ecs::components;
		ecs::ComponentManager* componentManager  = ecs::ComponentManager::GetInstance();
		core::vector<Entity> linkedEntities      = componentManager->collectUniqueLinkedEntities<cm::projectile,
																								 cm::transform,
																								 cm::material,
																								 cm::mesh,
																								 cm::collider>();
		
		core::vector<Entity> otherLinkedEntities = componentManager->collectUniqueLinkedEntities<cm::material,
																								 cm::collider,
																								 cm::mesh,
																								 cm::transform>();

		unsigned int linkedEntitiesVectorSize      = linkedEntities.GetSize();
		unsigned int otherLinkedEntitiesVectorSize = otherLinkedEntities.GetSize();

        for(unsigned int x = 0; x < linkedEntitiesVectorSize; ++x) {
          unsigned int uiEntity_refProjectile = linkedEntities[x];
          cm::transform* rTransformProjectile = componentManager->GetComponent<cm::transform>(uiEntity_refProjectile);
		  float rayLength = 1.0f;
		  vec3 ray        = rTransformProjectile->tForward * rayLength;

			for(unsigned int j = 0; j < otherLinkedEntitiesVectorSize; ++j) {
				unsigned int entityOther = otherLinkedEntities[j];
				cm::transform* transformOther = componentManager->GetComponent<cm::transform>(entityOther);
				float otherHalfScale = transformOther->fScale * 0.5f;
				float min = 0.0f;
				float max = 1.0f;

				for ( int dimension = 0; dimension < 3; ++dimension ) {
					float axis_invariant = 1.0f / ray[dimension];
					float box_min = transformOther->tPosition[dimension] - otherHalfScale;
					float box_max = transformOther->tPosition[dimension] + otherHalfScale;
					
					float delta1  = (box_min - rTransformProjectile->tPosition[dimension]) * axis_invariant;
					float delta2  = (box_max - rTransformProjectile->tPosition[dimension]) * axis_invariant;

					min = Max(min, Min(delta1, delta2));
					max = Min(max, Max(delta1, delta2));
					
					if ( max < min )
						break;
				}

				if ( max > min ) {
					ecs::EntityManager* entityManager       = GLVM::ecs::EntityManager::GetInstance();
					entityManager->RemoveEntity(uiEntity_refProjectile, componentManager);
					/// TODO: There is a big quastion is this decrement have sence.
//					--linkedEntitiesVectorSize;  
 				}
			}
		}
	}

	
	void COpenglRenderer::RaycastingDebug() {
		/// TODO: This code for debug purpouses only
		// float plane[] = {
		// 	-0.3f, -0.3f, -0.5f, 0.3f, 0.5f, 0.7f,
		// 	0.3f, -0.3f, -0.5f, 0.3f, 0.5f, 0.7f,
		// 	0.3f,  0.3f, -0.5f, 0.3f, 0.5f, 0.7f,
		// 	0.3f,  0.3f, -0.5f, 0.3f, 0.5f, 0.7f,
		// 	-0.3f,  0.3f, -0.5f, 0.3f, 0.5f, 0.7f,
		// 	-0.3f, -0.3f, -0.5f, 0.3f, 0.5f, 0.7f
		// };


		debugLines->Use();
		
		mat4 planeModelMatrix(1.0);
		planeModelMatrix[0][0] = 5.0;
		planeModelMatrix[1][1] = 5.0;
		planeModelMatrix[2][2] = 5.0;
		planeModelMatrix[3][3] = 1.0;
		planeModelMatrix[3][0] = 3.0;
		planeModelMatrix[3][1] = 5.0;
		planeModelMatrix[3][2] = 3.0;

		namespace cm = GLVM::ecs::components;
        ecs::ComponentManager* componentManager = ecs::ComponentManager::GetInstance();
		core::vector<unsigned int>* pEntityContainerRefView =
			componentManager->GetEntityContainer<cm::beholder>();
		unsigned int uiPlayerEntity = (*pEntityContainerRefView)[0];
		cm::beholder* playerViewComponent = componentManager->GetComponent<cm::beholder>(uiPlayerEntity);
		cm::transform* playerTransformComponent = componentManager->GetComponent<cm::transform>(uiPlayerEntity);
		
		unsigned int location = pGLGet_Uniform_Location(debugLines->iID, "modelMatrix");
		pGLUniform_Matrix4fv(location, NUMBER_OF_MATRICES, GL_FALSE, &planeModelMatrix[0][0]);
		ComputeProjectionMatrix(debugLines);
		ComputeViewMatrix(debugLines, *playerTransformComponent, *playerViewComponent);
		pGLGen_Vertex_Arrays(1, &vaoPlane);
		pGLGen_Buffers(1, &vboPlane);
		pGLBind_Vertex_Array(vaoPlane);
		pGLBind_Buffer(GL_ARRAY_BUFFER, vboPlane);
		pGLBuffer_Data(GL_ARRAY_BUFFER, sizeof(plane), plane, GL_STATIC_DRAW);

		pGLVertex_Attrib_Pointer(LAYOUT_0, VERTEX_SIZE, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)VERTEX_OFFSET);
		pGLEnable_Vertex_Attrib_Array(LAYOUT_0);
		pGLVertex_Attrib_Pointer(LAYOUT_1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		pGLEnable_Vertex_Attrib_Array(LAYOUT_1);
		
        glClear(GL_DEPTH_BUFFER_BIT);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		pGLBind_Buffer(GL_ARRAY_BUFFER, 0); 

        // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
        // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
        pGLBind_Vertex_Array(0);         

		mat4 linesModelMatrix(1.0);
		linesModelMatrix[0][0] = 1.0;
		linesModelMatrix[1][1] = 1.0;
		linesModelMatrix[2][2] = 1.0;
		linesModelMatrix[3][3] = 1.0;
		linesModelMatrix[3][0] = 0.0;
		linesModelMatrix[3][1] = 0.0;
		linesModelMatrix[3][2] = 0.0;

		unsigned int locationLines = pGLGet_Uniform_Location(debugLines->iID, "modelMatrix");
		pGLUniform_Matrix4fv(locationLines, NUMBER_OF_MATRICES, GL_FALSE, &linesModelMatrix[0][0]);

		float lines[36] = {
			0.0f, 5.0f, 0.0f,
			0.0f, 1.0f, 0.0f,
			0.0f, 5.0f, -5.5f,
		    0.0f, 1.0f, 0.0f,
			
			0.0f, 5.0f, 0.0f,
			0.0f, 1.0f, 0.0f,
			0.0f, 10.0f, 0.0f,
		    0.0f, 1.0f, 0.0f,
			
			0.0f, 5.0f, 0.0f,
			0.0f, 1.0f, 0.0f,
			5.0f, 5.0f, 0.0f,
		    0.0f, 1.0f, 0.0f
		};
		
		pGLGen_Vertex_Arrays(1, &vaoLines);
		pGLGen_Buffers(1, &vboLines);
		pGLBind_Vertex_Array(vaoLines);
		pGLBind_Buffer(GL_ARRAY_BUFFER, vboLines);
		pGLBuffer_Data(GL_ARRAY_BUFFER, sizeof(lines), lines, GL_DYNAMIC_DRAW);

		pGLVertex_Attrib_Pointer(LAYOUT_0, VERTEX_SIZE, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)VERTEX_OFFSET);
		pGLEnable_Vertex_Attrib_Array(LAYOUT_0);
		pGLVertex_Attrib_Pointer(LAYOUT_1, VERTEX_SIZE, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		pGLEnable_Vertex_Attrib_Array(LAYOUT_1);

		pGLBind_Vertex_Array(vaoLines);
		glDrawArrays(GL_LINES, 0, 6);	
	}
	
	void COpenglRenderer::RenderQuad()
	{
		if (quadVAO_ == 0)
			{
				float quadVertices[] = {
					// positions        // texture Coords
					-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
					-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
					1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
					1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
				};
				// setup plane VAO
				pGLGen_Vertex_Arrays(1, &quadVAO_);
				pGLGen_Buffers(1, &quadVBO_);
				pGLBind_Vertex_Array(quadVAO_);
				pGLBind_Buffer(GL_ARRAY_BUFFER, quadVBO_);
				pGLBuffer_Data(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
				pGLEnable_Vertex_Attrib_Array(0);
				pGLVertex_Attrib_Pointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
				pGLEnable_Vertex_Attrib_Array(1);
				pGLVertex_Attrib_Pointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
			}
		pGLBind_Vertex_Array(quadVAO_);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		pGLBind_Vertex_Array(0);
	}
	
	void COpenglRenderer::SetVertices(std::vector<unsigned int>& _aIndices,
									  std::vector<float>& _aVertices) {
		GLuint iVbo_;
		GLuint iVao_;
		GLuint iEbo_;
		pGLGen_Vertex_Arrays(NUMBER_OF_CREATING_VAO_OBJECT_1, &iVao_);
        pGLGen_Buffers(NUMBER_OF_CREATING_VBO_OBJECT_1, &iVbo_);
		
        pGLGen_Buffers(1, &iEbo_);
        
        ///< First we link the vertex array object, then we link and set the vertex buffers, and then we configure the vertex attributes.
        
        pGLBind_Vertex_Array(iVao_);
		
		pGLBind_Buffer(GL_ARRAY_BUFFER, iVbo_);
        pGLBuffer_Data(GL_ARRAY_BUFFER, sizeof(float) * _aVertices.size(), _aVertices.data(), GL_DYNAMIC_DRAW);

        pGLBind_Buffer(GL_ELEMENT_ARRAY_BUFFER, iEbo_);
        pGLBuffer_Data(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * _aIndices.size(), _aIndices.data(), GL_STATIC_DRAW);
        
		pGLVertex_Attrib_Pointer(LAYOUT_0, VERTEX_SIZE, GL_FLOAT, GL_FALSE, 16 * sizeof(float), (void*)VERTEX_OFFSET);
        pGLEnable_Vertex_Attrib_Array(LAYOUT_0);
		pGLVertex_Attrib_Pointer(LAYOUT_1, 3, GL_FLOAT, GL_FALSE, 16 * sizeof(float), (void*)(3 * sizeof(float)));
		pGLEnable_Vertex_Attrib_Array(LAYOUT_1);
		pGLVertex_Attrib_Pointer(2, TEXTURE_SIZE, GL_FLOAT, GL_FALSE, 16 * sizeof(float), (void*)(6 * sizeof(float)));
		pGLEnable_Vertex_Attrib_Array(2);
		pGLVertex_Attrib_Pointer(3, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(float), (void*)(8 * sizeof(float)));
		pGLEnable_Vertex_Attrib_Array(3);
		pGLVertex_Attrib_Pointer(4, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(float), (void*)(12 * sizeof(float)));
		pGLEnable_Vertex_Attrib_Array(4);

		VBOcontainer_.push_back(iVbo_);
		VAOcontainer_.push_back(iVao_);
		EBOcontainer_.push_back(iEbo_);
	}
	
    void COpenglRenderer::loadWavefrontObj() {
        for (unsigned int m = 0; m < pathsArray_.size(); ++m) {
            CWaveFrontObjParser parser;
            CWaveFrontObjParser* wavefrontObjParser = &parser;

            wavefrontObjParser->ReadFile(pathsArray_[m]);
            wavefrontObjParser->ParseFile();
			aVertexes_.emplace_back();
            aIndices_.emplace_back();

			jointMatricesPerMesh.Push({});
			frames.Push({});
			
            unsigned int vertexIndex = 0;
            unsigned int textureIndex = 0;
			unsigned int normalIndex = 0;
            unsigned int faceVerticesSize = wavefrontObjParser->getFaces().GetSize();
            for (unsigned int i = 0; i < faceVerticesSize; ++i)
                for (unsigned int j = 0; j < 3; ++j) {
                    vertexIndex = wavefrontObjParser->getFaces()[i][0][j] - 1;
                    aIndices_[m].push_back(i * 3 + j);
                    SVertex vertex = wavefrontObjParser->getCoordinateVertices()[vertexIndex];
                    textureIndex = wavefrontObjParser->getFaces()[i][1][j] - 1;
                    SVertex texture = wavefrontObjParser->getTextureVertices()[textureIndex];
					normalIndex = wavefrontObjParser->getFaces()[i][2][j] - 1;
					SVertex normal = wavefrontObjParser->getNormals()[normalIndex];
					aVertexes_[m].push_back(vertex[0]);
					aVertexes_[m].push_back(vertex[1]);
					aVertexes_[m].push_back(vertex[2]);
					aVertexes_[m].push_back(normal[0]);
					aVertexes_[m].push_back(normal[1]);
					aVertexes_[m].push_back(normal[2]);
					aVertexes_[m].push_back(texture[0]);
					aVertexes_[m].push_back(texture[1]);

					aVertexes_[m].push_back(-1);
					aVertexes_[m].push_back(-1);
					aVertexes_[m].push_back(-1);
					aVertexes_[m].push_back(-1);
					aVertexes_[m].push_back(1.0);
					aVertexes_[m].push_back(1.0);
					aVertexes_[m].push_back(1.0);
					aVertexes_[m].push_back(1.0);
                }
			SetVertices(aIndices_[m], aVertexes_[m]);
			++wavefrontObjCounter;
        }
    }

	void COpenglRenderer::EnlargeFrameAccumulator(float value) {
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
	
	mat4 COpenglRenderer::SetModelMatrix(ecs::components::transform& transformComponent_)
	{
        mat4 rotationMatrix(1.0f);
        mat4 modelMatrix(1.0f);
        mat4 scalingMatrix(1.0f);
        mat4 translationMatrix(1.0f);

        scalingMatrix[0][0] = transformComponent_.fScale;
        scalingMatrix[1][1] = transformComponent_.fScale;
        scalingMatrix[2][2] = transformComponent_.fScale;
		scalingMatrix[3][3] = 1.0f;
        
        translationMatrix[3][0] = transformComponent_.tPosition[0];
		translationMatrix[3][1] = transformComponent_.tPosition[1];
		translationMatrix[3][2] = transformComponent_.tPosition[2];
        translationMatrix[3][3] = 1.0f;

		float sinPitch = std::sin(Radians(-transformComponent_.pitch / 2));
		float cosPitch = std::cos(Radians(-transformComponent_.pitch / 2));
		float sinYaw = std::sin(Radians(-(transformComponent_.yaw)  / 2));
		float cosYaw = std::cos(Radians(-(transformComponent_.yaw)  / 2));
		
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
        modelMatrix = scalingMatrix * rotationMatrix * translationMatrix;

		return modelMatrix;
	}

    void COpenglRenderer::SetViewMatrix(mat4 _viewMatrix) {
        unsigned int uniformLocationViewWorld = pGLGet_Uniform_Location(coreShaderProgram->iID, "viewMatrix");
        pGLUniform_Matrix4fv(uniformLocationViewWorld, NUMBER_OF_MATRICES, GL_FALSE, &_viewMatrix[0][0]);
    }

    void COpenglRenderer::SetProjectionMatrix(mat4 _projectionMatrix) {
        unsigned int uniformLocationProjectionWorld = pGLGet_Uniform_Location(coreShaderProgram->iID, "projectionMatrix");
		pGLUniform_Matrix4fv(uniformLocationProjectionWorld, NUMBER_OF_MATRICES, GL_FALSE, &_projectionMatrix[0][0]);
    }
    
    void COpenglRenderer::SetTextureData(std::vector<ecs::Texture>& _texture_data) {
		texture_load_data = _texture_data;
	}
	
    void COpenglRenderer::SetMeshData(std::vector<const char*> _pathsArray, core::vector<const char*> pathsGLTF_) {
		for (unsigned int i = 0; i < _pathsArray.size(); ++i)
            pathsArray_.push_back(_pathsArray[i]);

		for (unsigned int i = 0; i < pathsGLTF_.GetSize(); ++i)
			pathsGLTF_.Push(pathsGLTF_[i]);
	}

 	void COpenglRenderer::LoadTextureData(GLVM::ecs::Texture& texture)
	{
		///< Loading and creating texture.
		glGenTextures(NUMBER_OF_CREATING_TEXTURE_OBJECT_1, &texture.iTexture_);
		glBindTexture(GL_TEXTURE_2D, texture.iTexture_);

		[[maybe_unused]] const char* path_to_stb_image = nullptr;
		[[maybe_unused]] int channels = 0;
#ifdef STB_IMAGE_IMPLEMENTATION
		path_to_stb_image = texture.path_to_image;
		texture.u_iData_ = stbi_load(path_to_stb_image, reinterpret_cast<int*>(&texture.iWidth_), reinterpret_cast<int*>(&texture.iHeight_), &channels, 0);
#endif
		
		///< Loading image, creating texture and generation mipmap-levels
		glTexImage2D(GL_TEXTURE_2D, MIPMAP_LEVEL, GL_RGBA, texture.iWidth_, texture.iHeight_, SOME_OLD_STUFF, GL_RGBA, GL_UNSIGNED_BYTE, texture.u_iData_);
		pGLGenerate_Mipmap(GL_TEXTURE_2D);

		///< Setting applying parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
	
    void COpenglRenderer::run() {
		for ( unsigned int i = 0; i < textureVector.size(); ++i ) {
			LoadTextureData(textureVector[i]);
		}
		loadWavefrontObj();
		
		core::vector<bool> animationFlags;
		for (unsigned int m = 0; m < pathsGLTF_.GetSize(); ++m) {
			Core::CJsonParser jsonParser;
			aVertexes_.emplace_back();
			aIndices_.emplace_back();
			jointMatricesPerMesh.Push({});
			frames.Push({});
			animationFlags.Push({});
			uint32_t nextIndexGLTF = wavefrontObjCounter + m;
			jsonParser.LoadGLTF(pathsGLTF_[m], aVertexes_[nextIndexGLTF], aIndices_[nextIndexGLTF], jointMatricesPerMesh[nextIndexGLTF], frames[nextIndexGLTF], animationFlags[m]);
		}
		for (unsigned int m = 0; m < pathsGLTF_.GetSize(); ++m) {
			uint32_t nextIndexGLTF = wavefrontObjCounter + m;
			std::vector<float> currentVertices;
			if ( animationFlags[m] ) {
				for ( unsigned int n = 0; n < aVertexes_[nextIndexGLTF].size(); n += 8 ) {
					currentVertices.push_back(aVertexes_[nextIndexGLTF][n]);
					currentVertices.push_back(aVertexes_[nextIndexGLTF][n + 1]);
					currentVertices.push_back(aVertexes_[nextIndexGLTF][n + 2]);
					currentVertices.push_back(aVertexes_[nextIndexGLTF][n + 3]);
					currentVertices.push_back(aVertexes_[nextIndexGLTF][n + 4]);
					currentVertices.push_back(aVertexes_[nextIndexGLTF][n + 5]);
					currentVertices.push_back(aVertexes_[nextIndexGLTF][n + 6]);
					currentVertices.push_back(aVertexes_[nextIndexGLTF][n + 7]);
					
					currentVertices.push_back(-1);
					currentVertices.push_back(-1);
					currentVertices.push_back(-1);
					currentVertices.push_back(-1);
					currentVertices.push_back(1);
					currentVertices.push_back(1);
					currentVertices.push_back(1);
					currentVertices.push_back(1);
				}
			
				SetVertices(aIndices_[nextIndexGLTF], currentVertices);
			} else {
				SetVertices(aIndices_[nextIndexGLTF], aVertexes_[nextIndexGLTF]);
			}
		}
	}

	void COpenglRenderer::ComputeViewMatrix(Shader* shaderProgram, ecs::components::transform& player, ecs::components::beholder& beholder)
    {
        Matrix<float, 4> viewMatrix(1.0f);
        const float kSensitivity = 0.1f;

        fYaw = g_eEvent.mousePointerPosition.offset_X;
        pitch = g_eEvent.mousePointerPosition.offset_Y;
        fYaw *= kSensitivity;
        pitch *= kSensitivity;

        g_eEvent.mousePointerPosition.pitch = pitch;
        g_eEvent.mousePointerPosition.yaw = fYaw;
        
        if(pitch > 89.0f)
            pitch = 89.0f;
        if(pitch < -89.0f)
            pitch = -89.0f;

		vec3 forward;
		float sinPitch = std::sin(Radians(pitch / 2));
		float cosPitch = std::cos(Radians(pitch / 2));
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
        beholder.forward = Normalize(forward);
        viewMatrix = LookAtMain(player.tPosition,
								player.tPosition + beholder.forward,
								beholder.up);

		shaderProgram->SetMat4("viewMatrix", viewMatrix);
    }

	void COpenglRenderer::ComputeProjectionMatrix(Shader* shaderProgram) {
		mat4 tProjection_Matrix = Perspective(Radians(90.0f), (float)1920 / (float)1080, 0.1f, 1000.0f);
		shaderProgram->SetMat4("projectionMatrix", tProjection_Matrix);
	}
}
