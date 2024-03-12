// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#ifndef SHADER_PROGRAM
#define SHADER_PROGRAM

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include "GLPointer.h"
#include "VertexMath.hpp"

/*! \class Shader
    \brief Class for creating shader program

    Contains vertex and fragment shaders
*/

class Shader
{
public:
    unsigned int iID;

    Shader(const char* vertexShaderPath_, const char* fragmentShaderPath_, const char* geometryShaderPath_ = nullptr)
    {
        std::string vertexShaderCode;
        std::string fragmentShaderCode;
		std::string geometryShaderCode;
        std::ifstream vertexShaderFile;
        std::ifstream fragmentShaderFile;
		std::ifstream geometryShaderFile;

        vertexShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        fragmentShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		geometryShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try
        {
            /// Open files
            vertexShaderFile.open(vertexShaderPath_);
            fragmentShaderFile.open(fragmentShaderPath_);
            std::stringstream Vertex_Shader_Stream, Fragment_Shader_Stream;

            /// Read file buffers
            Vertex_Shader_Stream << vertexShaderFile.rdbuf();
            Fragment_Shader_Stream << fragmentShaderFile.rdbuf();

            /// Close files
            vertexShaderFile.close();
            fragmentShaderFile.close();

            /// Converting to string varibale thread data
            vertexShaderCode = Vertex_Shader_Stream.str();
            fragmentShaderCode = Fragment_Shader_Stream.str();

			/// If geometry shader path is present, also load a geometry shader
			if (geometryShaderPath_ != nullptr) {
				geometryShaderFile.open(geometryShaderPath_);
				std::stringstream geometryShaderStream;
				geometryShaderStream << geometryShaderFile.rdbuf();
				geometryShaderFile.close();
				geometryShaderCode = geometryShaderStream.str();
			}
        }
        catch (std::ifstream::failure& e)
        {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
        }
        const char* pVertexShaderCode   = vertexShaderCode.c_str();
        const char* pFragmentShaderCode = fragmentShaderCode.c_str();

        /// Shaders compilation
        GLuint uiVertex, uiFragment, uiGeometryShaderID;

        /// Vertex shader
        uiVertex = pGLCreate_Shader(GL_VERTEX_SHADER);
        pGLShader_Source(uiVertex, 1, &pVertexShaderCode, NULL);
        pGLCompile_Shader(uiVertex);
        CheckCompileErrors(uiVertex, "VERTEX");

        /// Fragment shader
        uiFragment = pGLCreate_Shader(GL_FRAGMENT_SHADER);
        pGLShader_Source(uiFragment, 1, &pFragmentShaderCode, NULL);
        pGLCompile_Shader(uiFragment);
        CheckCompileErrors(uiFragment, "FRAGMENT");

		/// Geometry shader
		if (geometryShaderPath_ != nullptr) {
			const char* pGeometryShaderCode = geometryShaderCode.c_str();
			uiGeometryShaderID = pGLCreate_Shader(GL_GEOMETRY_SHADER);
			pGLShader_Source(uiGeometryShaderID, 1, &pGeometryShaderCode, NULL);
			pGLCompile_Shader(uiGeometryShaderID);
			CheckCompileErrors(uiGeometryShaderID, "GEOMETRY");
		}

        ///< Shader program
        iID = pGLCreate_Program();
        pGLAttach_Shader(iID, uiVertex);
        pGLAttach_Shader(iID, uiFragment);
		if (geometryShaderPath_ != nullptr)
			pGLAttach_Shader(iID, uiGeometryShaderID);
        pGLLink_Program(iID);
        CheckCompileErrors(iID, "PROGRAM");

        ///< Free shaders
        pGLDelete_Shader(uiVertex);
        pGLDelete_Shader(uiFragment);
		if (geometryShaderPath_ != nullptr)
			pGLDelete_Shader(uiGeometryShaderID);
    }

    void Use();
    void SetBool(const std::string& name, bool value) const;
    void SetInt(const std::string& name, int value) const;
	void SetInt(const std::string& name, GLsizei count, const GLint* value) const;
    void SetFloat(const std::string& name, float value) const;
	void SetVec3(const std::string &name, float x, float y, float z) const;
	void SetVec3(const std::string &name, const vec3& vector) const;	
	void SetVec4(const std::string &name, float x, float y, float z, float w) const;
	void SetVec4(const std::string &name, int x, int y, int z, int w) const;
	void SetUniformID(const char* _uniformIdentificator, int _id);
	void SetMat4(const std::string &name, mat4 &mat) const;
	void SetMat4(const std::string &name, unsigned int matrixNumber, mat4 &mat) const;
//	void SetMat4(const std::string &name, glm::mat4 &mat) const;
	
private:
    void CheckCompileErrors(unsigned int shader, std::string type);
};

#endif
