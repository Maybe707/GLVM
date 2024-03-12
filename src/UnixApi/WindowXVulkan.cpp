// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#include "UnixApi/WindowXVulkan.hpp"

#include "GLPointer.h"
#include <GL/glx.h>
#include <X11/Xlib.h>
#include <bits/types/time_t.h>
#include <bits/types/wint_t.h>
#include <iostream>

namespace GLVM::core
{
    WindowXVulkan::WindowXVulkan()
    {
        // const int aAttrib[] =
        // {
        //     GLX_RENDER_TYPE, GLX_RGBA_BIT,
        //     GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
        //     GLX_DOUBLEBUFFER, true,
        //     GLX_RED_SIZE, 1,
        //     GLX_GREEN_SIZE, 1,
        //     GLX_BLUE_SIZE, 1,
        //     None
        // };
        
        pDisp_ = XOpenDisplay(NULL);
        Root_Window_ = DefaultRootWindow(pDisp_);
        Set_Window_Attributes_.event_mask = KeyPressMask | KeyReleaseMask |
            PointerMotionMask | StructureNotifyMask | ButtonPressMask | ButtonReleaseMask;

        Win_ = XCreateWindow(pDisp_, Root_Window_, 0, 0, 1920, 1080, 0, CopyFromParent, InputOutput,
                            CopyFromParent, CWEventMask, &Set_Window_Attributes_);    
        ///< Show_the_window

        XMapWindow(pDisp_, Win_);

        XWarpPointer(pDisp_, None, Win_, 0, 0, 0, 0, 0, 0);
		
        Cursor invisibleCursor;
        Pixmap bitmapNoData;
        XColor black;
        static char noData[] = { 0,0,0,0,0,0,0,0 };
        black.red = black.green = black.blue = 0;

        bitmapNoData = XCreateBitmapFromData(pDisp_, Win_, noData, 8, 8);
        invisibleCursor = XCreatePixmapCursor(pDisp_, bitmapNoData, bitmapNoData, 
                                              &black, &black, 0, 0);
        XDefineCursor(pDisp_,Win_, invisibleCursor);
        
        XFreeCursor(pDisp_, invisibleCursor);
        XFreePixmap(pDisp_, bitmapNoData);
        
        XGetWindowAttributes(pDisp_, Win_, &GWindow_Attributes_);
//		const int kInterval = 1;
    }
    
    WindowXVulkan::~WindowXVulkan() = default;

    Window WindowXVulkan::GetWindow() { return Win_; }
    Display* WindowXVulkan::GetDisplay() { return pDisp_; }

    void WindowXVulkan::CursorLock(int _x_position, int _y_position, int* _x_offset, int* _y_offset)
    {
        ///< Solve a problem with endlessly growing numbers in the start game run.
        // if(_x_position > 1920 || _x_position < 0 || _y_position > 1080 || _y_position < 0)
        //     return;

        int iOffset_X = 0, iOffset_Y = 0;
        iOffset_X = _x_position - 960;
        iOffset_Y = _y_position - 540;
        
        *_x_offset += iOffset_X;
        *_y_offset -= iOffset_Y;

        if(*_y_offset > 890)
            *_y_offset = 890;
        else if(*_y_offset < -890)
            *_y_offset = -890;
        
        XWarpPointer(pDisp_, None, Win_, 0, 0, 0, 0, 960, 540);
        XFlush(pDisp_);
    }
    
    void WindowXVulkan::SwapBuffers()
    {
    }

    void WindowXVulkan::ClearDisplay()
    {
    }

    bool WindowXVulkan::HandleEvent(CEvent& _Event)
    {
        XEvent uXEvent;

        while(XPending(pDisp_))
        {
            XNextEvent(pDisp_, &uXEvent);
			KeySym ulKey;
            unsigned int uiMouse_Button;
            XMotionEvent motion;

			switch(uXEvent.type)
			{
            case MotionNotify:
                motion = uXEvent.xmotion;

                _Event.SetEvent(EEvents::eMOUSE_POINTER_POSITION);
                _Event.mousePointerPosition.position_X = motion.x;
                _Event.mousePointerPosition.position_Y = motion.y;

                ///< Search MapNotify events depend on XMapWindow(pDisp_, Win_) function.
//				break;
            case MapNotify:
                ///< Link mouse cursor to specified window.
                XGrabPointer
                    (
                        pDisp_, Win_, 
                        True, PointerMotionMask,
                        GrabModeAsync, GrabModeAsync,
                        Win_,
                        None, CurrentTime
                    );
                break;
            case ButtonPress:
                uiMouse_Button = uXEvent.xbutton.button;
                switch(uiMouse_Button)
                {
                case 1:
                    _Event.SetEvent(EEvents::eMOUSE_LEFT_BUTTON);
                    break;
                }
                break;

            case ButtonRelease:
                uiMouse_Button = uXEvent.xbutton.button;
                switch(uiMouse_Button)
                {
                case 1:
                    _Event.SetEvent(EEvents::eMOUSE_LEFT_BUTTON_RELEASE);
                    break;
                }
                break;
                
			case KeyPress:
				ulKey = XLookupKeysym(&uXEvent.xkey, 0);
				switch(ulKey)
				{
				case XK_Escape:
					_Event.SetEvent(EEvents::eGAME_LOOP_KILL);
					break;
				case XK_a:
					_Event.SetEvent(EEvents::eMOVE_LEFT);
					break;
				case XK_d:
					_Event.SetEvent(EEvents::eMOVE_RIGHT);
					break;
				case XK_s:
					_Event.SetEvent(EEvents::eMOVE_BACKWARD);
					break;
				case XK_w:
					_Event.SetEvent(EEvents::eMOVE_FORWARD);
					break;
                case XK_space:
                    _Event.SetEvent(EEvents::eJUMP);
                    break;
				}
				break;

			case KeyRelease:
				if(XEventsQueued(pDisp_, QueuedAfterReading))
				{
					XEvent uXNext_Event;
					XPeekEvent(pDisp_, &uXNext_Event);
    
					if (uXNext_Event.type == KeyPress && uXNext_Event.xkey.time == uXEvent.xkey.time &&
						uXNext_Event.xkey.keycode == uXEvent.xkey.keycode)
					{
						///< Key wasn’t actually released
                        XNextEvent(pDisp_, &uXNext_Event);
						continue;
					}
				}
		    	ulKey = XLookupKeysym(&uXEvent.xkey, 0);
                switch(ulKey)
                {
                case XK_a:
                    _Event.SetEvent(GLVM::core::eKEYRELEASE_A);
                    break;
                case XK_d:
                    _Event.SetEvent(GLVM::core::eKEYRELEASE_D);
                    break;
                case XK_s:
                    _Event.SetEvent(GLVM::core::eKEYRELEASE_S);
                    break;
                case XK_w:
                    _Event.SetEvent(GLVM::core::eKEYRELEASE_W);
                    break;
                case XK_space:
                    _Event.SetEvent(GLVM::core::eKEYRELEASE_JUMP);
                    break;
                }
				break;
			}

			Input_Stack_->ControlInput(_Event);
        }
		return false;
    }

    void WindowXVulkan::Close()
    {
        XDestroyWindow(pDisp_, Win_);
//        XFreeColormap(pDisp_, Color_Map_);
//        XFree(pVisual_);
//        XFree(pFbc_);
        XCloseDisplay(pDisp_);
    }
}


