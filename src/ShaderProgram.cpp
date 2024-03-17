// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#include "ShaderProgram.hpp"
#include "GLPointer.h"
#include <GL/gl.h>

#define ARRAY_INFO_LOG_RANGE 1024

///< Activate shader program
void Shader::Use()
{
	pGLUse_Program(iID);
}
///< Uniform functions
void Shader::SetBool(const std::string& name, bool value) const
{
	pGLUniform1i(pGLGet_Uniform_Location(iID, name.c_str()), (int)value);
}
void Shader::SetInt(const std::string& name, int value) const
{
	pGLUniform1i(pGLGet_Uniform_Location(iID, name.c_str()), value);
}
void Shader::SetInt(const std::string& name, GLsizei count, const GLint* value) const
{
	pGLUniform1iv(pGLGet_Uniform_Location(iID, name.c_str()), count, value);
}
void Shader::SetFloat(const std::string& name, float value) const
{
	pGLUniform1f(pGLGet_Uniform_Location(iID, name.c_str()), value);
}
void Shader::SetVec3(const std::string &name, float x, float y, float z) const
{ 
	pGLUniform3f(pGLGet_Uniform_Location(iID, name.c_str()), x, y, z);
}
void Shader::SetVec3(const std::string &name, const vec3& vector) const
{ 
	pGLUniform3fv(pGLGet_Uniform_Location(iID, name.c_str()), 1, &vector[0]);
}
void Shader::SetVec4(const std::string &name, float x, float y, float z, float w) const {
	pGLUniform4f(pGLGet_Uniform_Location(iID, name.c_str()), x, y, z, w);
}
void Shader::SetVec4(const std::string &name, int x, int y, int z, int w) const {
	pGLUniform4i(pGLGet_Uniform_Location(iID, name.c_str()), x, y, z, w);
}
void Shader::SetUniformID(const char* _uniformIdentificator, int _id)
{
	pGLUniform1i(pGLGet_Uniform_Location(iID, _uniformIdentificator), _id);
}

void Shader::SetMat4(const std::string &name, mat4 &mat) const
{
	pGLUniform_Matrix4fv(pGLGet_Uniform_Location(iID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::SetMat4(const std::string &name, unsigned int matrixNumber, mat4 &mat) const
{
	pGLUniform_Matrix4fv(pGLGet_Uniform_Location(iID, name.c_str()), matrixNumber, GL_FALSE, &mat[0][0]);
}

// void Shader::SetMat4(const std::string &name, glm::mat4 &mat) const
// {
// 	pGLUniform_Matrix4fv(pGLGet_Uniform_Location(iID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
// }

///< Functions for exmination of shaders errors
void Shader::CheckCompileErrors(unsigned int shader, std::string type)
{
	int iSuccess;
	char cInfo_Log[ARRAY_INFO_LOG_RANGE];
	if (type != "PROGRAM")
	{
		pGLGet_Shaderiv(shader, GL_COMPILE_STATUS, &iSuccess);
		if (!iSuccess)
		{
			pGLGet_Shader_Info_Log(shader, ARRAY_INFO_LOG_RANGE, 0, cInfo_Log);
			std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << cInfo_Log << "\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
	else
	{
		pGLGet_Programiv(shader, GL_LINK_STATUS, &iSuccess);
		if (!iSuccess)
		{
			pGLGet_Program_Info_Log(shader, ARRAY_INFO_LOG_RANGE, 0, cInfo_Log);
			std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << cInfo_Log << "\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
}
