#ifdef __linux__
#include <GL/glx.h>
#define GET_PROC_ADDRESS glXGetProcAddress
#endif

#ifdef _WIN32
#include <windows.h>
#include <GL/gl.h>
#include <GL/glext.h>
#define GET_PROC_ADDRESS(s) wglGetProcAddress((LPCSTR)s)
#endif

#define INIT_EXT
#include "include/GLPointer.h"

EXTERN_C void Initializer()
{
    pGLVertex_Arrays = (void (*)(GLsizei, GLuint))GET_PROC_ADDRESS((const GLubyte *)"glGenVertexArrays");

    pGLGen_Vertex_Arrays =(void (*)(GLsizei, GLuint *))
        GET_PROC_ADDRESS((const GLubyte *)"glGenVertexArrays");

    pGLGen_Buffers = (void (*)(GLsizei, GLuint *))
        GET_PROC_ADDRESS((const GLubyte *)"glGenBuffers");

    pGLBind_Vertex_Array = (void (*)(GLuint))
        GET_PROC_ADDRESS((const GLubyte *)"glBindVertexArray");

    pGLBind_Buffer = (void (*)(GLenum, GLuint))
        GET_PROC_ADDRESS((const GLubyte *)"glBindBuffer");

    pGLBuffer_Data = (void (*)(GLenum, GLsizeiptr, const GLvoid *, GLenum))GET_PROC_ADDRESS((const GLubyte *)"glBufferData");

    pGLVertex_Attrib_Pointer =(void (*)(GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid *))GET_PROC_ADDRESS((const GLubyte *)"glVertexAttribPointer");

    pGLEnable_Vertex_Attrib_Array = (void (*)(GLuint))GET_PROC_ADDRESS((const GLubyte *)"glEnableVertexAttribArray");

    pGen_Textures = (void (*)(GLsizei, GLuint *))GET_PROC_ADDRESS((const GLubyte *)"glGenTextures");

    pGLBind_Textures = (void (*)(GLuint, GLsizei, const GLuint *))GET_PROC_ADDRESS((const GLubyte *)"glBindTextures");

    pGLTex_Parameteri = (void (*)(GLenum, GLenum, GLint))GET_PROC_ADDRESS((const GLubyte *)"glTexParameteri");

    pGLTex_Image2D =(void (*)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid *))GET_PROC_ADDRESS((const GLubyte *)"glTexImage2D");

    pGLGenerate_Mipmap = (void (*)(GLenum))GET_PROC_ADDRESS((const GLubyte *)"glGenerateMipmap");

    pGLGet_Uniform_Location = (GLint (*)(GLuint, const GLchar *))GET_PROC_ADDRESS((const GLubyte *)"glGetUniformLocation");

    pGLUniform1i = (void (*)(GLint, GLint))GET_PROC_ADDRESS((const GLubyte *)"glUniform1i");

    pGLDelete_Vertex_Arrays = (void (*)(GLsizei, const GLuint *))GET_PROC_ADDRESS((const GLubyte *)"glDeleteVertexArrays");

    pGLDelete_Buffers = (void (*)(GLsizei, const GLuint *))GET_PROC_ADDRESS((const GLubyte *)"glDeleteBuffers");

    pGLCreate_Shader = (GLuint(*)(GLenum))GET_PROC_ADDRESS((GLubyte *)"glCreateShader");

    pGLShader_Source =(void (*)(GLuint, GLsizei, const GLchar **, const GLint *))GET_PROC_ADDRESS((const GLubyte *)"glShaderSource");

    pGLCompile_Shader = (void (*)(GLuint))GET_PROC_ADDRESS((const GLubyte *)"glCompileShader");

    pGLCreate_Program = (GLuint(*)(void))GET_PROC_ADDRESS((const GLubyte *)"glCreateProgram");

    pGLAttach_Shader = (void (*)(GLuint, GLuint))GET_PROC_ADDRESS((const GLubyte *)"glAttachShader");

    pGLLink_Program = (void (*)(GLuint))GET_PROC_ADDRESS((const GLubyte *)"glLinkProgram");

    pGLDelete_Shader = (void (*)(GLuint))GET_PROC_ADDRESS((const GLubyte *)"glDeleteShader");

    pGLUse_Program = (void (*)(GLuint))GET_PROC_ADDRESS((const GLubyte *)"glUseProgram");

	pGLUniform3f = (void (*)(GLuint, GLfloat, GLfloat, GLfloat))GET_PROC_ADDRESS((const GLubyte*)"glUniform3f");

	pGLUniform4f = (void (*)(GLuint, GLfloat, GLfloat, GLfloat, GLfloat))GET_PROC_ADDRESS((const GLubyte*)"glUniform4f");

	pGLUniform4i = (void (*)(GLint, GLfloat, GLfloat, GLfloat, GLfloat))GET_PROC_ADDRESS((const GLubyte*)"glUniform4i");
	
    pGLUniform1f = (void (*)(GLint, GLfloat))GET_PROC_ADDRESS((const GLubyte *)"glUniform1f");

    pGLGet_Shaderiv = (void (*)(GLuint, GLenum, GLint *))GET_PROC_ADDRESS((const GLubyte *)"glGetShaderiv");

    pGLGet_Shader_Info_Log = (void (*)(GLuint, GLsizei, GLsizei, GLchar *))GET_PROC_ADDRESS((const GLubyte *)"glGetShaderInfoLog");

    pGLGet_Programiv = (void (*)(GLuint, GLenum, GLint *))
        GET_PROC_ADDRESS((const GLubyte *)"glGetProgramiv");

    pGLGet_Program_Info_Log = (void (*)(GLuint, GLsizei, GLsizei, GLchar *))GET_PROC_ADDRESS((const GLubyte *)"glGetProgramInfoLog");

	pGLUniform_Matrix4fv = (void (*)(GLint, GLsizei, GLboolean, const GLfloat*))GET_PROC_ADDRESS((const GLubyte *)"glUniformMatrix4fv");

	pGLActive_Texture = (void (*)(GLenum))GET_PROC_ADDRESS((const GLubyte *)"glActiveTexture");

	pGLGen_Framebuffers = (void (*)(GLsizei n, GLuint* ids))GET_PROC_ADDRESS((const GLubyte *)"glGenFramebuffers");

	pGLBind_Framebuffer = (void (*)(GLenum target, GLuint framebuffer))GET_PROC_ADDRESS((const GLubyte *)"glBindFramebuffer");

	pGLFramebuffer_Texture2D = (void (*)(GLenum target, GLenum atachment, GLenum textarget, GLuint texture, GLint level))GET_PROC_ADDRESS((const GLubyte *)"glFramebufferTexture2D");

	pGLFramebuffer_Texture = (void (*)(GLenum target, GLenum attachment, GLuint texture, GLint level))GET_PROC_ADDRESS((const GLubyte *)"glFramebufferTexture");

	pGLUniform1fv = (void (*)(GLint location, GLsizei count, const GLfloat* value))GET_PROC_ADDRESS((const GLubyte *)"glUniform1fv");
	
	pGLUniform3fv = (void (*)(GLint location, GLsizei count, const GLfloat* value))GET_PROC_ADDRESS((const GLubyte *)"glUniform3fv");

	pGLUniformMatrix4fv = (void (*)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value))GET_PROC_ADDRESS((const GLubyte *)"glUniformMatrix4fv");

	pGLUniform1iv = (void (*)(GLint location, GLsizei count, const GLint* value))GET_PROC_ADDRESS((const GLubyte *)"glUniform1iv");

#ifdef __linux__
pGLXSwap_Interval_EXT = (void (*)(Display*, GLXDrawable, int))GET_PROC_ADDRESS((const GLubyte *)"glXSwapIntervalEXT");
#endif
	
#ifdef _WIN32
pWGLSwap_Interval_EXT = (BOOL (WINAPI *)(int))GET_PROC_ADDRESS((const GLubyte *)"wglSwapIntervalEXT");
#endif
}
