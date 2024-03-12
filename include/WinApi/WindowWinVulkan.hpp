// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#ifndef WINDOW_WIN_VULKAN
#define WINDOW_WIN_VULKAN

#include "IWindow.hpp"
#include <stdio.h>
#include <wchar.h>
#include <windows.h>
#include <windowsx.h>
#include <GL/gl.h>
#include "glext.h"
#include "GLPointer.h"

//#define VULKAN_API
#define OPENGL_API
#include "EventsStack.hpp"

namespace GLVM::core
{
    
#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001

#define WGL_DRAW_TO_WINDOW_ARB 0x2001
#define WGL_SUPPORT_OPENGL_ARB 0x2010
#define WGL_DOUBLE_BUFFER_ARB 0x2011
#define WGL_PIXEL_TYPE_ARB 0x2013
#define WGL_COLOR_BITS_ARB 0x2014
#define WGL_DEPTH_BITS_ARB 0x2022
#define WGL_STENCIL_BITS_ARB 0x2023
#define WGL_SAMPLE_BUFFERS_ARB 0x2041
#define WGL_SAMPLES_ARB 0x2042
#define WGL_TYPE_RGBA_ARB 0x202B
        
    class WindowWinVulkan : public IWindow
    {
        HWND pClassic_Window_;
        HDC pClassic_DC_;
        HGLRC pClassic_Context_;
        
        WNDCLASS window_Class_;
        HDC pModern_DC_;
        HGLRC pModern_Context_;
        HWND pModern_Window_;
        
    public:
		CStack           * Input_Stack_;
        WindowWinVulkan();

        void SwapBuffers() override;
        void ClearDisplay() override;
        bool HandleEvent(CEvent& _Event) override;
        HWND GetClassicWindowHWND();
        HWND GetModernWindowHWND();
        void Close() override;
        virtual void CursorLock(int _x_position, int _y_position, int* _x_offset, int* _y_offset) override;
        ///< Callback method for events handling.
        static LRESULT CALLBACK MainWndProc(HWND _pHwnd, UINT _pMsg, WPARAM _pWParam, LPARAM _pLParam);
    }; // namespace GLVM::core
}

#endif
