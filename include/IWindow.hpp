// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#ifndef IWINDOW
#define IWINDOW

#include "Event.hpp"

namespace GLVM::core
{    

    class IWindow
    {
    public:
        virtual ~IWindow() = default;

        virtual void SwapBuffers() = 0;
        virtual void ClearDisplay() = 0;
        virtual bool HandleEvent(CEvent& _Event) = 0;
        virtual void Close() = 0;
        virtual void CursorLock(int _x_position, int _y_position, int* _x_offset, int* _y_offset) = 0;
    };

}
    
#endif
