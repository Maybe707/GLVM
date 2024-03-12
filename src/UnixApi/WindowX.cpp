// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#include "UnixApi/WindowX.hpp"

#include "GLPointer.h"
#include "Event.hpp"
#include <GL/glx.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <bits/types/time_t.h>
#include <bits/types/wint_t.h>
#include <iostream>

//#define VULKAN
#define OPENGL

namespace GLVM::Core
{
#ifdef OPENGL
    CWindowX::CWindowX()
    {
        const int aAttrib[] =
        {
            GLX_RENDER_TYPE, GLX_RGBA_BIT,
            GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
            GLX_DOUBLEBUFFER, true,
            GLX_RED_SIZE, 1,
            GLX_GREEN_SIZE, 1,
            GLX_BLUE_SIZE, 1,
            None
        };
        
        pDisp_ = XOpenDisplay(NULL);
        
        pFbc_ = glXChooseFBConfig(pDisp_,
                                         DefaultScreen(pDisp_),
                                         aAttrib, &iNum_Fbc_);

        if (!pFbc_)
        {
            printf("glXChooseFBConfig() failed\n");
        exit(1);
        }

        pVisual_ = glXGetVisualFromFBConfig(pDisp_, pFbc_[0]);

        if(pVisual_ == NULL)
        {
            printf("\n\tno appropriate visual found\n\n");
            exit(0);
        } 
        else
        {
            
            ///< creates hexadecimal output like in glxinfo
            
            printf("\n\tvisual %p selected\n", (void *)pVisual_->visualid); 
        }

        Root_Window_ = DefaultRootWindow(pDisp_);
        Color_Map_ = XCreateColormap(pDisp_, Root_Window_, pVisual_->visual, AllocNone);

        Set_Window_Attributes_.colormap = Color_Map_;
        Set_Window_Attributes_.event_mask = KeyPressMask | KeyReleaseMask |
            PointerMotionMask | StructureNotifyMask | ButtonPressMask | ButtonReleaseMask;

        Win_ = XCreateWindow(pDisp_, Root_Window_, 0, 0, 1920, 1080, 0, pVisual_->depth, InputOutput,
                            pVisual_->visual, CWColormap | CWEventMask, &Set_Window_Attributes_);    

    
        pGLXCreateContextAttribsARB_ =
            (GLXContext (*)(Display*, GLXFBConfig, GLXContext, Bool, const int*))
        glXGetProcAddress((const GLubyte*)"glXCreateContextAttribsARB");

        if (!pGLXCreateContextAttribsARB_)
        {
            printf("glXCreateContextAttribsARB() not found\n");
            exit(1);
        }

        ///< Set desired minimum OpenGL version
        
        int aContext_Attribs[] =
            {
                GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
                GLX_CONTEXT_MINOR_VERSION_ARB, 2,
                // GLX_CONTEXT_PROFILE_MASK_ARB,
                // GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
                None
            };
        
        ///< Create modern OpenGL context
        
        Context_ = pGLXCreateContextAttribsARB_(pDisp_, pFbc_[0], NULL, true,
                                                    aContext_Attribs);
        if (!Context_)
        {
            printf("Failed to create OpenGL context. Exiting.\n");
            exit(1);
        }

        ///< Show_the_window

        XMapWindow(pDisp_, Win_);
        glXMakeCurrent(pDisp_, Win_, Context_);

        int iMajor = 0, iMinor = 0;
        glGetIntegerv(GL_MAJOR_VERSION, &iMajor);
        glGetIntegerv(GL_MINOR_VERSION, &iMinor);
        printf("OpenGL context created.\nVersion %d.%d\nVendor %s\nRenderer %s\n",
               iMajor, iMinor,
               glGetString(GL_VENDOR),
               glGetString(GL_RENDERER));

        ///< glEnable(GL_DEPTH_TEST);

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
		Drawable = glXGetCurrentDrawable();
		Initializer();
		const int kInterval = 1;

		if (Drawable)
		{
			pGLXSwap_Interval_EXT(pDisp_, Drawable, kInterval);
		}
    }
    
    CWindowX::~CWindowX()
    {
    }

    void CWindowX::CursorLock(int _x_position, int _y_position, int* _x_offset, int* _y_offset)
    {
        ///< Solve a problem with endlessly growing numbers in the start game run.
        if(_x_position > 1920 || _x_position < 0 || _y_position > 1080 || _y_position < 0)
            return;
        
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
    
    void CWindowX::SwapBuffers()
    {
        glXSwapBuffers(pDisp_, Win_);
    }

    void CWindowX::ClearDisplay()
    {
        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    bool CWindowX::HandleEvent(CEvent& _Event)
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
                
                ///< With structure "motion" we can get position of mouse pointer and e.c.
                motion = uXEvent.xmotion;
                
                _Event.SetEvent(EEvents::eMOUSE_POINTER_POSITION);
                _Event.mouse_Pointer_Position_.iPosition_X = motion.x;
                _Event.mouse_Pointer_Position_.iPosition_Y = motion.y;
                                
                ///< Search MapNotify events depend on XMapWindow(pDisp_, Win_) function.
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
                    _Event.SetEvent(GLVM::Core::eKEYRELEASE_A);
                    break;
                case XK_d:
                    _Event.SetEvent(GLVM::Core::eKEYRELEASE_D);
                    break;
                case XK_s:
                    _Event.SetEvent(GLVM::Core::eKEYRELEASE_S);
                    break;
                case XK_w:
                    _Event.SetEvent(GLVM::Core::eKEYRELEASE_W);
                    break;
                case XK_space:
                    _Event.SetEvent(GLVM::Core::eKEYRELEASE_JUMP);
                    break;
                }
				break;
			}
			return true;
        }
		return false;
    }

    void CWindowX::Close()
    {
        glXMakeCurrent(pDisp_, None, NULL);
        glXDestroyContext(pDisp_, Context_);
        XDestroyWindow(pDisp_, Win_);
        XFreeColormap(pDisp_, Color_Map_);
        XFree(pVisual_);
        XFree(pFbc_);
        XCloseDisplay(pDisp_);
    }
#endif

#ifdef VULKAN
    CWindowX::CWindowX()
    {
        const int aAttrib[] =
        {
            GLX_RENDER_TYPE, GLX_RGBA_BIT,
            GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
            GLX_DOUBLEBUFFER, true,
            GLX_RED_SIZE, 1,
            GLX_GREEN_SIZE, 1,
            GLX_BLUE_SIZE, 1,
            None
        };
        
        pDisp_ = XOpenDisplay(NULL);
        Root_Window_ = DefaultRootWindow(pDisp_);
        Set_Window_Attributes_.event_mask = KeyPressMask | KeyReleaseMask |
            PointerMotionMask | StructureNotifyMask | ButtonPressMask | ButtonReleaseMask;

        Win_ = XCreateWindow(pDisp_, Root_Window_, 0, 0, 1920, 1080, 0, CopyFromParent, InputOutput,
                            CopyFromParent, CWEventMask, &Set_Window_Attributes_);    
        ///< Show_the_window

        XMapWindow(pDisp_, Win_);

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
		const int kInterval = 1;
    }
    
    CWindowX::~CWindowX()
    {
    }

    Window CWindowX::GetWindow() { return Win_; }
    Display* CWindowX::GetDisplay() { return pDisp_; }

    void CWindowX::CursorLock(int _x_position, int _y_position, int* _x_offset, int* _y_offset)
    {
        ///< Solve a problem with endlessly growing numbers in the start game run.
        if(_x_position > 1920 || _x_position < 0 || _y_position > 1080 || _y_position < 0)
            return;
        
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
    
    void CWindowX::SwapBuffers()
    {
    }

    void CWindowX::ClearDisplay()
    {
    }

    bool CWindowX::HandleEvent(CEvent& _Event)
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
                _Event.mouse_Pointer_Position_.iPosition_X = motion.x;
                _Event.mouse_Pointer_Position_.iPosition_Y = motion.y;
                                
                ///< Search MapNotify events depend on XMapWindow(pDisp_, Win_) function.
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
                    _Event.SetEvent(GLVM::Core::eKEYRELEASE_A);
                    break;
                case XK_d:
                    _Event.SetEvent(GLVM::Core::eKEYRELEASE_D);
                    break;
                case XK_s:
                    _Event.SetEvent(GLVM::Core::eKEYRELEASE_S);
                    break;
                case XK_w:
                    _Event.SetEvent(GLVM::Core::eKEYRELEASE_W);
                    break;
                case XK_space:
                    _Event.SetEvent(GLVM::Core::eKEYRELEASE_JUMP);
                    break;
                }
				break;
			}
			return true;
        }
		return false;
    }

    void CWindowX::Close()
    {
        XDestroyWindow(pDisp_, Win_);
//        XFreeColormap(pDisp_, Color_Map_);
//        XFree(pVisual_);
//        XFree(pFbc_);
        XCloseDisplay(pDisp_);
    }
#endif
}


