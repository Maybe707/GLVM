// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#ifndef WINDOW_XCB_VULKAN
#define WINDOW_XCB_VULKAN

#include "IWindow.hpp"
#include <cstdint>
#include <unistd.h>
#include <cstdio>

#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/xcb_cursor.h>
#include <cassert>
#include <iostream>
#include "EventsStack.hpp"

namespace GLVM::core
{
    typedef uint32_t xcb_window_t;
	
	class WindowXCBVulkan : public IWindow
	{
		xcb_connection_t*  connection;
		xcb_screen_t*      screen;
		xcb_window_t       window;
		xcb_key_symbols_t* key_symbols;
		xcb_generic_event_t* next_generic_event;

		static void print_modifiers (uint32_t mask);
	public:
		CStack           * Input_Stack_;
		
		WindowXCBVulkan ();

		void HideCursor();
		xcb_connection_t* GetConnection();
		xcb_window_t GetWindow();
		void Disconnect();

		void SwapBuffers() override;
        void ClearDisplay() override;
        bool HandleEvent(CEvent& _Event) override;
        void Close() override;
        void CursorLock(int _x_position, int _y_position, int* _x_offset, int* _y_offset) override;
	};
} // namespace GLVM::core

#endif
