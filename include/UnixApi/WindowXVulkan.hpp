// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#ifndef WINDOW_X_VULKAN
#define WINDOW_X_VULKAN

#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include "IWindow.hpp"
#include "EventsStack.hpp"

namespace GLVM::core
{    
    class WindowXVulkan : public IWindow
    {
        XWindowAttributes GWindow_Attributes_;
        Window Root_Window_;
        XSetWindowAttributes Set_Window_Attributes_;
        
        //XWindowAttributes gwa_;

    public:
        Display* pDisp_;
        Window Win_;
		CStack* Input_Stack_;
        
        WindowXVulkan();
        ~WindowXVulkan();
        
        Window GetWindow();
        Display* GetDisplay();
        void CursorLock(int _x_position, int _y_position, int* _x_offset, int* _y_offset) override;
        void SwapBuffers() override;
        void ClearDisplay() override;
        bool HandleEvent(CEvent& _Event) override;
        void Close() override;
    };
}
    
#endif


