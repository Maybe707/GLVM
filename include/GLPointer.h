// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#ifndef GLPOINTER
#define GLPOINTER

#ifdef __linux__
#include <X11/Xlib.h>
#include <GL/glx.h>
#endif

#ifdef _WIN32
#include <windows.h>
#include <GL/gl.h>
#include <GL/glext.h>
#endif

#ifdef INIT_EXT
#define EXTERN
#else
#define EXTERN extern
#endif

#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C
#endif

EXTERN_C void Initializer();

EXTERN void (*pGLVertex_Arrays)(GLsizei, GLuint);

EXTERN void (*pGLGen_Vertex_Arrays)(GLsizei, GLuint *);

EXTERN void (*pGLGen_Buffers)(GLsizei, GLuint *);

EXTERN void (*pGLBind_Vertex_Array)(GLuint);

EXTERN void (*pGLBind_Buffer)(GLenum, GLuint);

EXTERN void (*pGLBuffer_Data)(GLenum, GLsizeiptr, const GLvoid *, GLenum);

EXTERN void (*pGLVertex_Attrib_Pointer)(GLuint, GLint, GLenum, GLboolean, GLsizei,
                                 const GLvoid *);

EXTERN void (*pGLEnable_Vertex_Attrib_Array)(GLuint);

EXTERN void (*pGen_Textures)(GLsizei, GLuint *);

EXTERN void (*pGLBind_Textures)(GLuint, GLsizei, const GLuint *);

EXTERN void (*pGLTex_Parameteri)(GLenum, GLenum,
                          GLint);

EXTERN void (*pGLTex_Image2D)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum,
                       GLenum, const GLvoid *);

EXTERN void (*pGLGenerate_Mipmap)(GLenum);

EXTERN GLint (*pGLGet_Uniform_Location)(GLuint, const GLchar *);

EXTERN void (*pGLUniform3f)(GLuint, GLfloat, GLfloat, GLfloat);

EXTERN void (*pGLUniform4f)(GLuint, GLfloat, GLfloat, GLfloat, GLfloat);

EXTERN void (*pGLUniform4i)(GLint, GLfloat, GLfloat, GLfloat, GLfloat);

EXTERN void (*pGLUniform1i)(GLint, GLint);

EXTERN void (*pGLDelete_Vertex_Arrays)(GLsizei, const GLuint *);

EXTERN void (*pGLDelete_Buffers)(GLsizei,
                          const GLuint *);

EXTERN GLuint (*pGLCreate_Shader)(GLenum);

EXTERN void (*pGLShader_Source)(GLuint, GLsizei, const GLchar **, const GLint *);

EXTERN void (*pGLCompile_Shader)(GLuint);

EXTERN GLuint (*pGLCreate_Program)(void);

EXTERN void (*pGLAttach_Shader)(GLuint, GLuint);

EXTERN void (*pGLLink_Program)(GLuint);

EXTERN void (*pGLDelete_Shader)(GLuint);

EXTERN void (*pGLUse_Program)(GLuint);

EXTERN void (*pGLUniform1f)(GLint, GLfloat);

EXTERN void (*pGLGet_Shaderiv)(GLuint, GLenum,
                        GLint *);

EXTERN void (*pGLGet_Shader_Info_Log)(GLuint, GLsizei, GLsizei, GLchar *);

EXTERN void (*pGLGet_Programiv)(GLuint, GLenum,
                         GLint *);

EXTERN void (*pGLGet_Program_Info_Log)(GLuint, GLsizei, GLsizei, GLchar *);

EXTERN void (*pGLUniform_Matrix4fv)(GLint, GLsizei, GLboolean,
									const GLfloat*);

EXTERN void (*pGLActive_Texture)(GLenum);

EXTERN void (*pGLGen_Framebuffers)(GLsizei n, GLuint *ids);

EXTERN void (*pGLBind_Framebuffer)(GLenum target, GLuint framebuffer);

EXTERN void (*pGLFramebuffer_Texture2D)(GLenum target, GLenum atachment, GLenum textarget, GLuint texture, GLint level);

EXTERN void (*pGLFramebuffer_Texture)(GLenum target, GLenum attachment, GLuint texture, GLint level);

EXTERN void (*pGLUniform1fv)(GLint location, GLsizei count, const GLfloat* value);

EXTERN void (*pGLUniform3fv)(GLint location, GLsizei count, const GLfloat* value);

EXTERN void (*pGLUniformMatrix4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);

EXTERN void (*pGLUniform1iv)(GLint location, GLsizei count, const GLint* value);

#ifdef __linux__
EXTERN void (*pGLXSwap_Interval_EXT)(Display *, GLXDrawable, int);
#endif

#ifdef _WIN32
EXTERN BOOL (WINAPI *pWGLSwap_Interval_EXT)(int);
#endif

//EXTERN BOOL (WINAPI *pWGLSwap_Interval_EXT)(int);

#endif
