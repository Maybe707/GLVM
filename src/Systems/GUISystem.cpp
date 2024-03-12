// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#include "Systems/GUISystem.hpp"
#include <GL/gl.h>


namespace GLVM::ecs
{
	CGUISystem::CGUISystem() {
		_Shader_Program = new Shader("../GLshaders/GUI.vert", "../GLshaders/GUI.frag");
		debugLines      = new Shader("../GLshaders/debugLines.vert", "../GLshaders/debugLines.frag");
	}
	
    void CGUISystem::Update()
    {
        Matrix<float, 4> tModel_Matrix(1.0);
        tModel_Matrix[0][0] = 0.05;
        tModel_Matrix[1][1] = 0.05;
        tModel_Matrix[2][2] = 0.05;
        Matrix<float, 4> tProjection_Matrix(1.0f);
        Matrix<float, 4> tView_Matrix(1.0f);

        _Shader_Program->Use();
		
        unsigned int uiTransformt_Loc = pGLGet_Uniform_Location(_Shader_Program->iID, "modelMatrix");
		pGLUniform_Matrix4fv(uiTransformt_Loc, NUMBER_OF_MATRICES, GL_FALSE, &tModel_Matrix[0][0]);

//		tProjection_Matrix = Perspective(Radians(90.0f), (float)1920 / (float)1080, 1.0f, 100.0f);
		unsigned int transformLocationProjection = pGLGet_Uniform_Location(_Shader_Program->iID, "projectionMatrix");
		pGLUniform_Matrix4fv(transformLocationProjection, NUMBER_OF_MATRICES, GL_FALSE, &tProjection_Matrix[0][0]);
						
        float aCrosshair_Vertices[] = {
			-0.1, 0.5, -0.3,
			0.1, -0.5, -0.3,
			0.1, 0.5, -0.3,
			-0.1, 0.5, -0.3,
			-0.1, -0.5, -0.3,
			0.1, -0.5, -0.3,
			-0.5, 0.1, -0.3,
		    0.5, -0.1, -0.3,
			0.5, 0.1, -0.3,
			-0.5, 0.1, -0.3,
			-0.5, -0.1, -0.3,
			0.5, -0.1, -0.3
        }; 

        pGLGen_Vertex_Arrays(1, &iVao_Crosshair_);
        pGLGen_Buffers(1, &iVbo_Crosshair_);
        // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
        pGLBind_Vertex_Array(iVao_Crosshair_);

        pGLBind_Buffer(GL_ARRAY_BUFFER, iVbo_Crosshair_);
        pGLBuffer_Data(GL_ARRAY_BUFFER, sizeof(aCrosshair_Vertices), aCrosshair_Vertices, GL_DYNAMIC_DRAW);

        pGLVertex_Attrib_Pointer(0, VERTEX_SIZE, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)VERTEX_OFFSET);
        pGLEnable_Vertex_Attrib_Array(LAYOUT_0);
        
        glClear(GL_DEPTH_BUFFER_BIT);
        
        glDrawArrays(GL_TRIANGLES, 0, 12);

        // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
        pGLBind_Buffer(GL_ARRAY_BUFFER, 0); 

        // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
        // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
        pGLBind_Vertex_Array(0);






// 		Matrix<float, 4> tModel_Matrix2(1.0);
//         tModel_Matrix[0][0] = 1.05;
//         tModel_Matrix[1][1] = 1.05;
//         tModel_Matrix[2][2] = 1.05;
//         Matrix<float, 4> tProjection_Matrix2(1.0f);
//         Matrix<float, 4> tView_Matrix2(1.0f);

//         _Shader_Program->Use();
		
//         unsigned int uiTransformt_Loc2 = pGLGet_Uniform_Location(_Shader_Program->iID, "modelMatrix");
// 		pGLUniform_Matrix4fv(uiTransformt_Loc2, NUMBER_OF_MATRICES, GL_FALSE, &tModel_Matrix[0][0]);

// 		tProjection_Matrix = Perspective(Radians(90.0f), (float)1920 / (float)1080, 1.0f, 100.0f);
// 		pGLUniform_Matrix4fv(transformLocationProjection, NUMBER_OF_MATRICES, GL_FALSE, &tProjection_Matrix[0][0]);
		
// 		GLuint vboRay, vaoRay;
// 		float ray[24] = {
// 			0.0f,  -0.5f, -0.3f,
// 			0.9f,  0.0f, -1.0f,
// 		};

// 		// GLuint vboLines, vaoLines;
// 		// float lines[48] = {
// 		// 	-0.3f,  0.3f, -0.5f, 0.0f, 0.0f, 0.0f,
// 		// 	0.3f,  0.3f, -0.5f, 0.0f, 0.0f, 0.0f,
// 		// 	-0.3f, -0.3f, -0.5f, 0.0f, 0.0f, 0.0f,
// 		// 	0.3f,  0.3f, -0.5f, 0.0f, 0.0f, 0.0f,
// 		// 	0.3f,  0.3f, -0.5f, 0.0f, 0.0f, 0.0f,
// 		// 	0.3f, -0.3f, -0.5f, 0.0f, 0.0f, 0.0f,
// 		// 	-0.3f,  0.3f, -0.5f, 0.0f, 0.0f, 0.0f,
// 		// 	-0.3f, -0.3f, -0.5f, 0.0f, 0.0f, 0.0f
// 		// };

//         pGLGen_Vertex_Arrays(1, &vaoRay);
//         pGLGen_Buffers(1, &vboRay);
//         // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
//         pGLBind_Vertex_Array(vaoRay);

//         pGLBind_Buffer(GL_ARRAY_BUFFER, vboRay);
//         pGLBuffer_Data(GL_ARRAY_BUFFER, sizeof(ray), ray, GL_DYNAMIC_DRAW);

//         pGLVertex_Attrib_Pointer(0, VERTEX_SIZE, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)VERTEX_OFFSET);
//         pGLEnable_Vertex_Attrib_Array(LAYOUT_0);
        
//         glClear(GL_DEPTH_BUFFER_BIT);
        
// //        glDrawArrays(GL_TRIANGLES, 0, 6);
// 		glDrawArrays(GL_LINES, 0, 8);	

//         // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
//         pGLBind_Buffer(GL_ARRAY_BUFFER, 0); 

//         // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
//         // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
//         pGLBind_Vertex_Array(0);
    }

	void CGUISystem::RaycastringDebug() {
// 		/// TODO: This code for debug purpouses only
// 		// float plane[] = {
// 		// 	-0.3f, -0.3f, -0.5f, 0.3f, 0.5f, 0.7f,
// 		// 	0.3f, -0.3f, -0.5f, 0.3f, 0.5f, 0.7f,
// 		// 	0.3f,  0.3f, -0.5f, 0.3f, 0.5f, 0.7f,
// 		// 	0.3f,  0.3f, -0.5f, 0.3f, 0.5f, 0.7f,
// 		// 	-0.3f,  0.3f, -0.5f, 0.3f, 0.5f, 0.7f,
// 		// 	-0.3f, -0.3f, -0.5f, 0.3f, 0.5f, 0.7f
// 		// };


// 		debugLines->Use();
		
// 		mat4 planeModelMatrix(1.0);
// 		planeModelMatrix[0][0] = 5.0;
// 		planeModelMatrix[1][1] = 5.0;
// 		planeModelMatrix[2][2] = 5.0;
// 		planeModelMatrix[3][3] = 1.0;
// 		planeModelMatrix[3][0] = 0.0;
// 		planeModelMatrix[3][1] = 0.0;
// 		planeModelMatrix[3][2] = 0.0;

// 		// for ( int i = 0; i < 4; ++i )
// 		// 	for ( int j = 0; j < 4; ++j)
// 		// 		std::cout << planeModelMatrix[i][j] << std::endl;
		
//         // Matrix<float, 4> planeModelMatrix(1.0);
//         // planeModelMatrix[0][0] = 0.05;
//         // planeModelMatrix[1][1] = 0.05;
//         // planeModelMatrix[2][2] = 0.05;

// 		namespace cm = GLVM::ecs::components;
//         ecs::ComponentManager* componentManager = ecs::ComponentManager::GetInstance();
// 		core::vector<unsigned int>* pEntityContainerRefView =
// 			componentManager->GetEntityContainer<cm::beholder>();
// 		unsigned int uiPlayerEntity = (*pEntityContainerRefView)[0];
// 		cm::beholder* playerViewComponent = componentManager->GetComponent<cm::beholder>(uiPlayerEntity);
// 		cm::transform* playerTransformComponent = componentManager->GetComponent<cm::transform>(uiPlayerEntity);
		
// //		viewPosition = playerViewComponent.Position;
		
// //		debugLines->SetMat4("modelMatrix", planeModelMatrix);
// 		unsigned int location = pGLGet_Uniform_Location(debugLines->iID, "modelMatrix");
// 		pGLUniform_Matrix4fv(location, NUMBER_OF_MATRICES, GL_FALSE, &planeModelMatrix[0][0]);
// 		// ComputeProjectionMatrix(debugLines);
// 		// ComputeViewMatrix(debugLines, *playerTransformComponent, *playerViewComponent);
// 		pGLGen_Vertex_Arrays(1, &vaoPlane);
// 		pGLGen_Buffers(1, &vboPlane);
// 		pGLBind_Vertex_Array(vaoPlane);
// 		pGLBind_Buffer(GL_ARRAY_BUFFER, vboPlane);
// 		pGLBuffer_Data(GL_ARRAY_BUFFER, sizeof(plane), plane, GL_STATIC_DRAW);

// 		pGLVertex_Attrib_Pointer(LAYOUT_0, VERTEX_SIZE, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)VERTEX_OFFSET);
// 		pGLEnable_Vertex_Attrib_Array(LAYOUT_0);
// 		pGLVertex_Attrib_Pointer(LAYOUT_1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
// 		pGLEnable_Vertex_Attrib_Array(LAYOUT_1);
		
// 		// pGLBind_Buffer(GL_ARRAY_BUFFER, 0);
// 		// pGLBind_Vertex_Array(0);

//         glClear(GL_DEPTH_BUFFER_BIT);
		
// //		pGLBind_Buffer(GL_ARRAY_BUFFER, vboLines);
// //		pGLBind_Vertex_Array(vaoPlane);
// 		glDrawArrays(GL_TRIANGLES, 0, 6);

// 		pGLBind_Buffer(GL_ARRAY_BUFFER, 0); 

//         // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
//         // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
//         pGLBind_Vertex_Array(0);         

// 		mat4 linesModelMatrix(1.0);
// 		linesModelMatrix[0][0] = 10.0;
// 		linesModelMatrix[1][1] = 10.0;
// 		linesModelMatrix[2][2] = 10.0;
// 		linesModelMatrix[3][3] = 1.0;
// 		linesModelMatrix[3][0] = 3.0;
// 		linesModelMatrix[3][1] = 5.0;
// 		linesModelMatrix[3][2] = 3.0;

// 		unsigned int locationLines = pGLGet_Uniform_Location(debugLines->iID, "modelMatrix");
// 		pGLUniform_Matrix4fv(locationLines, NUMBER_OF_MATRICES, GL_FALSE, &linesModelMatrix[0][0]);

// 		float lines[12] = { playerTransformComponent->tPosition[0],
// 			playerTransformComponent->tPosition[1],
// 			playerTransformComponent->tPosition[2],
// 			0.0f, 0.0f, 0.0f,
// 			playerTransformComponent->tPosition[0] + playerViewComponent->forward[0],
// 			playerTransformComponent->tPosition[1] + playerViewComponent->forward[1],
// 			playerTransformComponent->tPosition[2] + playerViewComponent->forward[2],
// 		    0.0f, 0.0f, 0.0f };
		
// 		pGLGen_Vertex_Arrays(1, &vaoLines);
// 		pGLGen_Buffers(1, &vboLines);
// 		pGLBind_Vertex_Array(vaoLines);
// 		pGLBind_Buffer(GL_ARRAY_BUFFER, vboLines);
// 		pGLBuffer_Data(GL_ARRAY_BUFFER, sizeof(lines), lines, GL_DYNAMIC_DRAW);

// 		pGLVertex_Attrib_Pointer(LAYOUT_0, VERTEX_SIZE, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)VERTEX_OFFSET);
// 		pGLEnable_Vertex_Attrib_Array(LAYOUT_0);
// 		pGLVertex_Attrib_Pointer(LAYOUT_1, VERTEX_SIZE, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
// 		pGLEnable_Vertex_Attrib_Array(LAYOUT_1);

// 		pGLBind_Vertex_Array(vaoLines);
// 		glDrawArrays(GL_LINES, 0, 2);	
	}


}
