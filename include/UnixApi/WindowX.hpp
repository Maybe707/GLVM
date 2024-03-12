// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#ifndef WINDOW_LIN
#define WINDOW_LIN

#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include "IWindow.hpp"

namespace GLVM::Core
{    

    class CWindowX : public IWindow
    {
        Display* pDisp_;
        Window Win_;
        int iNum_Fbc_ = 0;
        GLXContext (*pGLXCreateContextAttribsARB_) (Display*, GLXFBConfig,
                                                  GLXContext, Bool, const int*) = 0;
        GLXContext Context_;
        XWindowAttributes GWindow_Attributes_;
        Window Root_Window_;
        XSetWindowAttributes Set_Window_Attributes_;
        Colormap Color_Map_;
        XVisualInfo* pVisual_;
        GLXFBConfig* pFbc_;
		GLXDrawable Drawable;
        
        //XWindowAttributes gwa_;
        

    public:
        
        CWindowX();
        ~CWindowX();
        
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


