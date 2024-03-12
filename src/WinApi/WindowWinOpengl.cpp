// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#include "WinApi/WindowWinOpengl.hpp"
#include "Event.hpp"
#include "GLPointer.h"
#include <iostream>
#include <iterator>

#define VK_W 0x57
#define VK_S 0x53
#define VK_A 0x41
#define VK_D 0x44

namespace GLVM::core
{
    WindowWinOpengl::WindowWinOpengl()
    {
        ///< Create classic window
        pClassic_Window_ = CreateWindowA( "STATIC", "", WS_POPUP | WS_DISABLED, 0, 0, 1, 1, NULL, NULL, GetModuleHandle( NULL ), NULL );
        pClassic_DC_ = GetDC( pClassic_Window_ );           ///< DC - device context.

        ///< Set classic pixel format
        PIXELFORMATDESCRIPTOR classic_Format_Descriptor =
            {
                sizeof( classic_Format_Descriptor ), 1, PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
                PFD_TYPE_RGBA, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24, 8, 0, PFD_MAIN_PLANE,
                0, 0, 0, 0
            };
        
        int iClassic_Pixel_Format = ChoosePixelFormat( pClassic_DC_, &classic_Format_Descriptor );
        SetPixelFormat( pClassic_DC_, iClassic_Pixel_Format, &classic_Format_Descriptor );

        ///< Create classic context
        pClassic_Context_ = wglCreateContext( pClassic_DC_ );
        wglMakeCurrent( pClassic_DC_, pClassic_Context_ );

        int iMajor, iMinor;
        glGetIntegerv( 4, &iMajor );
        glGetIntegerv( 2, &iMinor );
//		if ( iMajor < 3 || ( iMajor == 3 && iMinor < 2 ) ) throw VersionException();

        // Load OpenGL extensions
        //LoadExtensions();

        ///< Create final pixel format
        const int aPixel_Attribs[] =
            {
                WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
                WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
                WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
                WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
                WGL_COLOR_BITS_ARB, 32,
                WGL_DEPTH_BITS_ARB, 24,
                WGL_STENCIL_BITS_ARB, 8,
                0
            };

        BOOL ( WINAPI * WGLChoosePixFormatARB ) ( HDC hdc, const int* piAttribIList, const FLOAT* pfAttribFList, UINT nMaxFormats, int* piFormats, UINT* nNumFormats ) = (BOOL (WINAPI*)( HDC hdc, const int* piAttribIList, const FLOAT* pfAttribFList, UINT nMaxFormats, int* piFormats, UINT* nNumFormats )) wglGetProcAddress((LPCSTR)"wglChoosePixelFormatARB");

        HGLRC ( WINAPI * WGLCreateContextAtribbARB ) ( HDC hDC, HGLRC hShareContext, const int* attribList ) = (HGLRC (WINAPI *) ( HDC hDC, HGLRC hShareContext, const int* attribList )) wglGetProcAddress((LPCSTR)"wglCreateContextAttribsARB");

        const char aClass_Name[] = "Sample Window Class";
            
        window_Class_ = { };

//        window_Class_.style = WS_VISIBLE;
		window_Class_.style = CS_DBLCLKS | CS_PARENTDC;
        window_Class_.lpfnWndProc = MainWndProc;
        window_Class_.cbClsExtra = 0;
        window_Class_.cbWndExtra = 0;
        window_Class_.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        window_Class_.hCursor = LoadCursor(NULL, NULL);
        window_Class_.lpszClassName = (LPCSTR)aClass_Name;
        window_Class_.hInstance = GetModuleHandleA(NULL);
        window_Class_.lpszClassName = aClass_Name;

        RegisterClassA(&window_Class_);
        
        pModern_Window_ = CreateWindowEx(0, aClass_Name, "Sample Window Class", WS_OVERLAPPEDWINDOW, 0, 0, 1920, 1080, NULL, NULL, GetModuleHandleA(NULL), NULL);
        ShowWindow(pModern_Window_, SW_SHOW);
        pModern_DC_ = GetDC( pModern_Window_ );
        
        int iModern_Pixel_Format;
        UINT pFormat_Count;
        WGLChoosePixFormatARB(pModern_DC_, aPixel_Attribs, NULL, 1, &iModern_Pixel_Format, &pFormat_Count );
//		if ( pFormat_Count == 0 ) throw PixelFormatException();
        SetPixelFormat( pModern_DC_, iModern_Pixel_Format, &classic_Format_Descriptor );
		
        ///< Create modern OpenGL 4.2 context		
        int aAttributes[] =
        {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
            WGL_CONTEXT_MINOR_VERSION_ARB, 2,
            WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
            0
        };
		
        pModern_Context_ = WGLCreateContextAtribbARB( pModern_DC_, NULL, aAttributes );

        ///< Clean up
        wglMakeCurrent( pModern_DC_, pModern_Context_);

		SetCursorPos(0, 0);
        
		Initializer();
		const int kInterval = 1;
		pWGLSwap_Interval_EXT(kInterval);
    }

    void WindowWinOpengl::SwapBuffers()
    {
        ::SwapBuffers(pModern_DC_);
    }

    void WindowWinOpengl::ClearDisplay()
    {
        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    bool WindowWinOpengl::HandleEvent(CEvent& _Event)
    {
        ///< Create message struct object.
        MSG msg;

        SetWindowLongPtrW(pModern_Window_, GWLP_USERDATA, (LONG_PTR)&_Event);
        while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
			Input_Stack_->ControlInput(_Event);
        }
		return false;
    }

    void WindowWinOpengl::Close()
    {
        DestroyWindow(pModern_Window_);
		PostQuitMessage(0);
    }

    HWND WindowWinOpengl::GetClassicWindowHWND() { return pClassic_Window_; }
    HWND WindowWinOpengl::GetModernWindowHWND() { return pModern_Window_; }
    
    void WindowWinOpengl::CursorLock(int _x_position, int _y_position, int* _x_offset, int* _y_offset)
    {
        POINT point_position{960, 540};
        ClientToScreen(pModern_Window_, &point_position);

        ///< Solve a problem with endlessly growing numbers in the start game run.
        if(_x_position > 1911 || _x_position < 0 || _y_position > 1052 || _y_position < 0)
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

        SetCursorPos(point_position.x, point_position.y);
        SetCursor(NULL);
    }
    
    ///< Callback method for events handling.
    LRESULT CALLBACK WindowWinOpengl::MainWndProc(HWND _pHwnd, UINT _pMsg, WPARAM _pWParam, LPARAM _pLParam)
    {
		CEvent* pEvent = (CEvent*)GetWindowLongPtrW(_pHwnd, GWLP_USERDATA);

        int iMouse_Position_X, iMouse_Position_Y;
        // tagRECT rect;
        // const RECT* rect_ptr = &rect;
        
        switch (_pMsg) 
        { 
        case WM_CREATE:
            // GetWindowRect(pModern_Window_, &rect);
            // ClipCursor(rect_ptr);
            ///< Initialize the window. 
            return 0; 
 
        // case WM_PAINT: 
        //     ///< Paint the window's client area. 
        //     return 0; 
 
        case WM_SIZE:
            glViewport( 0, 0, LOWORD(_pLParam), HIWORD(_pLParam));
            ///< Set the size and position of the window. 
            return 0;

        case WM_LBUTTONDOWN:
            pEvent->SetEvent(EEvents::eMOUSE_LEFT_BUTTON);
            return 0;
            
        case WM_LBUTTONUP:
            pEvent->SetEvent(EEvents::eMOUSE_LEFT_BUTTON_RELEASE);
            return 0;

        case WM_MOUSEMOVE:
            iMouse_Position_X = GET_X_LPARAM(_pLParam);
            iMouse_Position_Y = GET_Y_LPARAM(_pLParam);
            pEvent->SetEvent(EEvents::eMOUSE_POINTER_POSITION);
            pEvent->mousePointerPosition.position_X = iMouse_Position_X;
            pEvent->mousePointerPosition.position_Y = iMouse_Position_Y;
            return 0;
            
        case WM_KEYDOWN: 
            switch (_pWParam) 
            { 
            case VK_LEFT: 
                break; 
 
            case VK_RIGHT: 
                break; 

            case VK_ESCAPE:
                pEvent->SetEvent(EEvents::eGAME_LOOP_KILL);
                break;
                
            case VK_W:
				pEvent->SetEvent(EEvents::eMOVE_FORWARD);
                break;

			case VK_S:
				pEvent->SetEvent(EEvents::eMOVE_BACKWARD);
                break;

			case VK_A:
				pEvent->SetEvent(EEvents::eMOVE_LEFT);
                break;

			case VK_D:
				pEvent->SetEvent(EEvents::eMOVE_RIGHT);
                break;

            case VK_SPACE:
                pEvent->SetEvent(EEvents::eJUMP);
                break;

			case VK_UP:
                break; 
 
            case VK_DOWN: 
                break; 
 
            case VK_HOME: 
                break; 
 
            case VK_END: 
                break; 
 
            case VK_INSERT: 
                break; 
 
            case VK_DELETE: 
                break; 
 
            case VK_F2: 
                break;
            
            default:
                break;
            }
            break;      

        case WM_KEYUP: 
            switch (_pWParam) 
            { 
            case VK_LEFT: 
                break; 
 
            case VK_RIGHT: 
                break; 
 
            case VK_W:
				pEvent->SetEvent(EEvents::eKEYRELEASE_W);
                break;

			case VK_S:
				pEvent->SetEvent(EEvents::eKEYRELEASE_S);
                break;

			case VK_A:
				pEvent->SetEvent(EEvents::eKEYRELEASE_A);
                break;

			case VK_D:
				pEvent->SetEvent(EEvents::eKEYRELEASE_D);
                break;

            case VK_SPACE:
                pEvent->SetEvent(EEvents::eKEYRELEASE_JUMP);

			case VK_UP:
                break; 
 
            case VK_DOWN: 
                break; 
 
            case VK_HOME: 
                break; 
 
            case VK_END: 
                break; 
 
            case VK_INSERT: 
                break; 
 
            case VK_DELETE: 
                break; 
 
            case VK_F2: 
                break;
            
            default:
                break;
            }
            break;      
			
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
                
            // 
            // Process other messages. 
            // 
 
        default: 
            return DefWindowProc(_pHwnd, _pMsg, _pWParam, _pLParam);
        }
        return 0;
    }
}
