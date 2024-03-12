// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#include "WinApi/WindowWinVulkan.hpp"
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
    WindowWinVulkan::WindowWinVulkan()
    {
        const char* _title = "Window class";
        int _width = 1920, _height = 1080;
        
        // Register the window class for the main window.
        window_Class_.style = 0;
//        wc.lpfnWndProc = procedure;
        window_Class_.lpfnWndProc = MainWndProc;
        window_Class_.cbClsExtra = 0;
        window_Class_.cbWndExtra = 0;
        window_Class_.hInstance = NULL;
        window_Class_.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        window_Class_.hCursor = LoadCursor(NULL, NULL);
        window_Class_.hbrBackground = NULL;
        window_Class_.lpszMenuName = NULL;
        window_Class_.lpszClassName = "Window class";

        RegisterClassA(&window_Class_);

        DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

        RECT rect;
        SetRect(&rect, 0, 0, _width, _height);
        AdjustWindowRect(&rect, style, FALSE);

        // Create the main window.
        pModern_Window_ = CreateWindowA("Window class",
                              _title,
                              style, CW_USEDEFAULT, CW_USEDEFAULT,
                              rect.right - rect.left, rect.bottom - rect.top, (HWND)NULL,
                              (HMENU)NULL, NULL, (LPVOID)NULL);

        // Show the window and paint its contents.
        ShowWindow(pModern_Window_, SW_SHOWDEFAULT);
        UpdateWindow(pModern_Window_);
		SetCursorPos(0, 0);		
    }

    void WindowWinVulkan::SwapBuffers() {}

    void WindowWinVulkan::ClearDisplay() {}

    bool WindowWinVulkan::HandleEvent(CEvent& _Event)
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

    void WindowWinVulkan::Close()
    {
        DestroyWindow(pModern_Window_);
		PostQuitMessage(0);
    }

    HWND WindowWinVulkan::GetClassicWindowHWND() { return pClassic_Window_; }
    HWND WindowWinVulkan::GetModernWindowHWND() { return pModern_Window_; }
    
    void WindowWinVulkan::CursorLock(int _x_position, int _y_position, int* _x_offset, int* _y_offset)
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
//}    
///< Callback method for events handling.
    LRESULT CALLBACK WindowWinVulkan::MainWndProc(HWND _pHwnd, UINT _pMsg, WPARAM _pWParam, LPARAM _pLParam)
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
