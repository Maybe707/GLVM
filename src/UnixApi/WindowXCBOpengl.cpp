// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#include "UnixApi/WindowXCBOpengl.hpp"
#include <X11/X.h>
#include <X11/XKBlib.h>
#include <cstdint>
#include <xcb/xcb.h>
#include <xcb/xcb_cursor.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/xfixes.h>
#include <xcb/xproto.h>

namespace GLVM::core
{
	WindowXCBOpengl::WindowXCBOpengl() {
        /* Open Xlib Display */ 
        display = XOpenDisplay(0);
        if(!display)
        {
            fprintf(stderr, "Can't open display\n");
        }

        default_screen = DefaultScreen(display);

        /* Get the XCB connection from the display */
        connection = XGetXCBConnection(display);
        if(!connection)
        {
            XCloseDisplay(display);
            fprintf(stderr, "Can't get xcb connection from display\n");
        }

        /* Acquire event queue ownership */
        XSetEventQueueOwner(display, XCBOwnsEventQueue);

        /* Find XCB screen */
        screen = 0;
        xcb_screen_iterator_t screen_iter = 
            xcb_setup_roots_iterator(xcb_get_setup(connection));
        for(int screen_num = default_screen;
			screen_iter.rem && screen_num > 0;
            --screen_num, xcb_screen_next(&screen_iter));
        screen = screen_iter.data;


		setup_and_run(display, connection, default_screen, screen);

		
		
		HideCursor();
		Initializer();

		const int kInterval = 0;

		if (drawable)
		{
			pGLXSwap_Interval_EXT(display, drawable, kInterval);
		}
	}

	void draw()
	{
		glClearColor(0.2, 0.4, 0.9, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	
	int WindowXCBOpengl::main_loop(Display *display, xcb_connection_t *connection, [[maybe_unused]] xcb_window_t window, GLXDrawable drawable)
	{
		int running = 1;
		while(running)
			{
				/* Wait for event */
				xcb_generic_event_t *event = xcb_wait_for_event(connection);
				if(!event)
					{
						fprintf(stderr, "i/o error in xcb_wait_for_event");
						return -1;
					}

				switch(event->response_type & ~0x80)
					{
					case XCB_KEY_PRESS:
						/* Quit on key press */
						running = 0;
						break;
					case XCB_EXPOSE:
						/* Handle expose event, draw and swap buffers */
						draw();
						glXSwapBuffers(display, drawable);
						break;
					default:
						break;
					}

				free(event);
			}

		return 0;
	}

	
	void WindowXCBOpengl::setup_and_run(Display* display, xcb_connection_t *connection, int default_screen, xcb_screen_t *screen) {
        int visualID = 0;

		/*
		  Attribs filter the list of FBConfigs returned by glXChooseFBConfig().
		  Visual attribs further described in glXGetFBConfigAttrib(3)
		*/
		static int visual_attribs[] =
			{
				GLX_X_RENDERABLE, True,
				GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
				GLX_RENDER_TYPE, GLX_RGBA_BIT,
				GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
				GLX_RED_SIZE, 8,
				GLX_GREEN_SIZE, 8,
				GLX_BLUE_SIZE, 8,
				GLX_ALPHA_SIZE, 8,
				GLX_DEPTH_SIZE, 24,
				GLX_STENCIL_SIZE, 8,
				GLX_DOUBLEBUFFER, True,
				//GLX_SAMPLE_BUFFERS  , 1,
				//GLX_SAMPLES         , 4,
				None
			};
		
        /* Query framebuffer configurations that match visual_attribs */
        GLXFBConfig *fb_configs = 0;
        int num_fb_configs = 0;
        fb_configs = glXChooseFBConfig(display, default_screen, visual_attribs, &num_fb_configs);
        if(!fb_configs || num_fb_configs == 0)
			{
				fprintf(stderr, "glXGetFBConfigs failed\n");
			}

        printf("Found %d matching FB configs", num_fb_configs);

        /* Select first framebuffer config and query visualID */
        fb_config = fb_configs[0];
        glXGetFBConfigAttrib(display, fb_config, GLX_VISUAL_ID , &visualID);

        /* Create XID's for colormap and window */
        xcb_colormap_t colormap = xcb_generate_id(connection);
        window = xcb_generate_id(connection);

        /* Create colormap */
        xcb_create_colormap(
            connection,
            XCB_COLORMAP_ALLOC_NONE,
            colormap,
            screen->root,
            visualID
            );

        /* Create window */
        uint32_t eventmask = XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE |
			XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_EXPOSURE |
			XCB_EVENT_MASK_POINTER_MOTION | XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW;
        uint32_t valuelist[] = { eventmask, colormap, 0 };
        uint32_t valuemask = XCB_CW_EVENT_MASK | XCB_CW_COLORMAP;

        xcb_create_window(
            connection,
            XCB_COPY_FROM_PARENT,
            window,
            screen->root,
            0, 0,
            1920, 1080,
            0,
            XCB_WINDOW_CLASS_INPUT_OUTPUT,
            visualID,
            valuemask,
            valuelist
            );

        // NOTE: window must be mapped before glXMakeContextCurrent
        xcb_map_window(connection, window); 
		
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
        
        context = pGLXCreateContextAttribsARB_(display, fb_config, NULL, true,
												aContext_Attribs);
        if (!context)
			{
				printf("Failed to create OpenGL context. Exiting.\n");
				exit(1);
			}

		/* Create GLX Window */
		drawable = 0;

		GLXWindow glxwindow = 
			glXCreateWindow(
				display,
				fb_config,
				window,
				0
				);

		if(!window)
			{
				xcb_destroy_window(connection, window);
				glXDestroyContext(display, context);

				fprintf(stderr, "glXDestroyContext failed\n");
			}

		drawable = glxwindow;
		
		glXMakeCurrent(display, drawable, context);
    }
	
	void WindowXCBOpengl::HideCursor() {
		xcb_pixmap_t foreground_pixmap_id = xcb_generate_id (connection);
		xcb_create_pixmap(connection, 1, foreground_pixmap_id,
						  window, 8, 8);

		// Create graphical context
        xcb_gcontext_t graphical_context = xcb_generate_id (connection);

        uint32_t mask = XCB_GC_FOREGROUND | XCB_GC_BACKGROUND;
        uint32_t values_list[2];
        values_list[0] = screen->black_pixel;
        values_list[1] = screen->white_pixel;

		xcb_create_gc(connection, graphical_context, window, XCB_GC_FOREGROUND | XCB_GC_BACKGROUND, values_list);

		const uint8_t pix_map_data[] = {
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
		};
		
		xcb_put_image(connection, XCB_IMAGE_FORMAT_XY_PIXMAP, foreground_pixmap_id, graphical_context, 0, 0, 100, 100, 0, 8, 32, pix_map_data);
		
        xcb_cursor_t cursor = xcb_generate_id (connection);
        xcb_create_cursor (connection,
						   cursor,
						   foreground_pixmap_id,
						   foreground_pixmap_id,
						   0, 0, 0, 0, 0, 0, 8, 8);

        mask = XCB_CW_CURSOR;
        uint32_t value_list = cursor;
        xcb_change_window_attributes (connection, window, mask, &value_list);

        xcb_free_cursor (connection, cursor);
	}
	
	xcb_connection_t* WindowXCBOpengl::GetConnection() { return connection; }
	
	xcb_window_t WindowXCBOpengl::GetWindow() { return window; }

	Display* WindowXCBOpengl::GetDisplay() { return display; }
	
	void WindowXCBOpengl::Disconnect() { xcb_disconnect ( connection ); }

	void WindowXCBOpengl::SwapBuffers() {
		glXSwapBuffers(display, drawable);
	};
	
	void WindowXCBOpengl::ClearDisplay() {
		glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	};

	void WindowXCBOpengl::print_modifiers (uint32_t mask)
	{
		const char **mod, *mods[] = {
			"Shift", "Lock", "Ctrl", "Alt",
			"Mod2", "Mod3", "Mod4", "Mod5",
			"Button1", "Button2", "Button3", "Button4", "Button5"
		};
		printf ("Modifier mask: ");
		for (mod = mods ; mask; mask >>= 1, mod++)
			if (mask & 1)
				std::cout << *mod << std::endl;;
		putchar ('\n');
	}

	bool WindowXCBOpengl::HandleEvent([[maybe_unused]] CEvent& _Event) {
		xcb_generic_event_t* generic_event;

// 		while (( event = xcb_poll_for_event ( GetConnection() ))) {
// //			std::cout << event->response_type << std::endl;
// 		}
		bool next_generic_event_flag = false;
		while (next_generic_event_flag || (generic_event = xcb_poll_for_event (connection))) {
			next_generic_event_flag = false;
			// int num_events = 0;
			// while (xcb_poll_for_queued_event(connection)) {
			// 	num_events++;
			// }
			// std::cout << num_events << std::endl;
			switch (generic_event->response_type & ~0x80) {
			case XCB_EXPOSE: {
				[[maybe_unused]] xcb_expose_event_t *expose_event = (xcb_expose_event_t *)generic_event;

				// printf ("Window %i exposed. Region to be redrawn at location (%d,%d), with dimension (%d,%d)\n",
				// 		expose_event->window, expose_event->x, expose_event->y, expose_event->width, expose_event->height);
				break;
			}
			case XCB_BUTTON_PRESS: {
				xcb_button_press_event_t *expose_event = (xcb_button_press_event_t *)generic_event;
//				print_modifiers(expose_event->state);

				switch (expose_event->detail) {
				case 1:
					_Event.SetEvent(EEvents::eMOUSE_LEFT_BUTTON);
					// printf ("Button %d pressed in window %i, at coordinates (%d,%d)\n",
					// 		expose_event->detail, expose_event->event, expose_event->event_x, expose_event->event_y);
					break;
				case 3:
					_Event.SetEvent(EEvents::eMOUSE_RIGHT_BUTTON);
					// printf ("Button %d pressed in window %i, at coordinates (%d,%d)\n",
					// 		expose_event->detail, expose_event->event, expose_event->event_x, expose_event->event_y);
					break;
				case 4:
					// printf ("Wheel Button up in window %i, at coordinates (%d,%d)\n",
					// 		expose_event->event, expose_event->event_x, expose_event->event_y);
					break;
				case 5:
					// printf ("Wheel Button down in window %i, at coordinates (%d,%d)\n",
					// 		expose_event->event, expose_event->event_x, expose_event->event_y);
					break;
				}
				
				break;
			}
			case XCB_BUTTON_RELEASE: {
				xcb_button_release_event_t *expose_event = (xcb_button_release_event_t *)generic_event;
//				print_modifiers(expose_event->state);

				switch (expose_event->detail) {
				case 1:
					_Event.SetEvent(EEvents::eMOUSE_LEFT_BUTTON_RELEASE);
					// printf ("Button %d released in window %i, at coordinates (%d,%d)\n",
					// 		expose_event->detail, expose_event->event, expose_event->event_x, expose_event->event_y);
					break;
				case 3:
					_Event.SetEvent(EEvents::eMOUSE_RIGHT_BUTTON_RELEASE);
					// printf ("Button %d released in window %i, at coordinates (%d,%d)\n",
					// 		expose_event->detail, expose_event->event, expose_event->event_x, expose_event->event_y);
					break;
				}
				
				break;
			}
			case XCB_MOTION_NOTIFY: {
				xcb_motion_notify_event_t *expose_event = (xcb_motion_notify_event_t *)generic_event;

				_Event.SetEvent(EEvents::eMOUSE_POINTER_POSITION);
                _Event.mousePointerPosition.position_X = expose_event->event_x;
                _Event.mousePointerPosition.position_Y = expose_event->event_y;
				
				// printf ("Mouse moved in window %i, at coordinates (%d,%d)\n",
				// 		expose_event->event, expose_event->event_x, expose_event->event_y);
//				break;
			}
			case XCB_MAP_WINDOW: {
//				std::cout << "MAP WINDOW" << std::endl;
		
				/// Make sure commands are sent befour we pause so that the window gets shown
				xcb_flush ( connection );
				
				xcb_grab_pointer_cookie_t cookie = xcb_grab_pointer(connection, 1, window, XCB_EVENT_MASK_POINTER_MOTION | XCB_EVENT_MASK_BUTTON_PRESS,
                XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, window, XCB_NONE, XCB_CURRENT_TIME);
				xcb_grab_pointer_reply(connection, cookie, NULL);
				break;
			}
			case XCB_ENTER_NOTIFY: {
				[[maybe_unused]] xcb_enter_notify_event_t *expose_event = (xcb_enter_notify_event_t *)generic_event;

				// printf ("Mouse entered window %i, at coordinates (%d,%d)\n",
				// 		expose_event->event, expose_event->event_x, expose_event->event_y);
				break;
			}
			// case XCB_LEAVE_NOTIFY: {
			// 	xcb_leave_notify_event_t *expose_event = (xcb_leave_notify_event_t *)generic_event;

			// 	printf ("Mouse left window %i, at coordinates (%d,%d)\n",
			// 			expose_event->event, expose_event->event_x, expose_event->event_y);
			// 	break;
			// }
			case XCB_KEY_PRESS: {
				xcb_key_press_event_t *expose_event = (xcb_key_press_event_t *)generic_event;
//				print_modifiers(expose_event->state);
//				std::cout << "KEY PRESS" << std::endl;
				// printf ("Key pressed in window %i\n",
				// 		expose_event->event);

//				xcb_keycode_t key_code = expose_event->detail;
//				std::cout << "Detail: " << xcb_key_press_lookup_keysym(key_symbols, expose_event, 0) << std::endl;
				// [[maybe_unused]] xcb_keysym_t keysym = xcb_key_press_lookup_keysym(key_symbols, expose_event, 0);

				xcb_keysym_t keysym = convertKeyCodeToSym(expose_event);

				switch(keysym)
					{
					case 65307:
						_Event.SetEvent(EEvents::eGAME_LOOP_KILL);
						break;
					case 97:
//						std::cout << "A key press" << std::endl;
						_Event.SetEvent(EEvents::eMOVE_LEFT);
						break;
					case 100:
//						std::cout << "D key press" << std::endl;
						_Event.SetEvent(EEvents::eMOVE_RIGHT);
						break;
					case 115:
//						std::cout << "S key press" << std::endl;
						_Event.SetEvent(EEvents::eMOVE_BACKWARD);
						break;
					case 119:
//						std::cout << "W key press" << std::endl;
						_Event.SetEvent(EEvents::eMOVE_FORWARD);
						break;
					case 32:
						_Event.SetEvent(EEvents::eJUMP);
						break;
					}
				
				break;
			}
			case XCB_KEY_RELEASE: {
				xcb_key_release_event_t *key_release_event = (xcb_key_release_event_t *)generic_event;
//				print_modifiers(key_release_event->state);

				// int num_events = 0;
				// while (xcb_poll_for_queued_event(connection)) {
				// 	num_events++;
				// }
				// std::cout << "first check: " << num_events << std::endl;

				// num_events = 0;
				// while (xcb_poll_for_queued_event(connection)) {
				// 	num_events++;
				// }
				// std::cout << "second check: " << num_events << std::endl;

				// printf ("Key released in window %i\n",
				// 		key_release_event->event);
//				std::cout << "KEY REALEASE" << std::endl;
				next_generic_event = xcb_poll_for_event(connection);
				if ( next_generic_event != NULL ) {
					xcb_key_press_event_t *key_press_event = (xcb_key_press_event_t *)next_generic_event;
					// xcb_keysym_t press_keysym = xcb_key_press_lookup_keysym(key_symbols, key_press_event, 0);
					// xcb_keysym_t release_keysym = xcb_key_press_lookup_keysym(key_symbols, key_release_event, 0);

					xcb_keysym_t press_keysym = convertKeyCodeToSym(key_press_event);
					xcb_keysym_t release_keysym = convertKeyCodeToSym(key_release_event);
					
					if (next_generic_event->response_type == XCB_KEY_PRESS &&
						key_press_event->time == key_release_event->time &&
						press_keysym == release_keysym)
					{
						///< Key wasn’t actually released
						// printf ("Key FAKE released in window %i\n",
						// 		key_release_event->event);
//						std::cout << "Key FAKE released in window" << std::endl;
//						generic_event = xcb_poll_for_event (connection);
//						free (generic_event);
						next_generic_event = NULL;
						continue;
					} else {
						next_generic_event_flag = true;
					}
				}
				
				// xcb_keysym_t release_keysym = xcb_key_press_lookup_keysym(key_symbols, key_release_event, 0);
				// std::cout << "KEYSYM RELEASE: " << release_keysym << std::endl;

				xcb_keysym_t release_keysym = convertKeyCodeToSym(key_release_event);

                switch(release_keysym)
                {
                case 97:
					// printf ("Key released in window %i\n",
					// 		key_release_event->event);
//					std::cout << "A key release" << std::endl;
                    _Event.SetEvent(GLVM::core::eKEYRELEASE_A);
                    break;
                case 100:
					// printf ("Key released in window %i\n",
					// 		key_release_event->event);
//					std::cout << "D key release" << std::endl;
                    _Event.SetEvent(GLVM::core::eKEYRELEASE_D);
                    break;
                case 115:
 					// printf ("Key released in window %i\n",
					// 		key_release_event->event);
//					std::cout << "S key release" << std::endl;
                    _Event.SetEvent(GLVM::core::eKEYRELEASE_S);
                    break;
                case 119:
 					// printf ("Key released in window %i\n",
					// 		key_release_event->event);
//					std::cout << "W key release" << std::endl;
                    _Event.SetEvent(GLVM::core::eKEYRELEASE_W);
                    break;
                case 32:
 					// printf ("Key released in window %i\n",
					// 		key_release_event->event);
                    _Event.SetEvent(GLVM::core::eKEYRELEASE_JUMP);
                    break;
                }

				break;
			}
			// default:
			// 	/* Unknown event type, ignore it */
			// 	printf("Unknown event: %d\n", generic_event->response_type);
			// 	break;
			}
			if ( next_generic_event != NULL ) {

				*generic_event = *next_generic_event;
				next_generic_event = NULL;
//				goto buffer_event;
			} else {
				/* Free the Generic Event */
				free (generic_event);
			}

			Input_Stack_->ControlInput(_Event);
		}

		return false;
	};

	xcb_keysym_t WindowXCBOpengl::convertKeyCodeToSym(xcb_key_release_event_t *key_release_event) {
		xcb_keysym_t release_keysym = -1;
		int keycode = (int)key_release_event->detail;
		switch ( keycode ) {
		case 9:
			release_keysym = 65307;
			break;
		case 38:
			release_keysym = 97;
			break;
		case 40:
			release_keysym = 100;
			break;
		case 39:
			release_keysym = 115;
			break;
		case 25:
			release_keysym = 119;
			break;
		case 65:
			release_keysym = 32;
			break;
		}

		return release_keysym;
	}
	
	void WindowXCBOpengl::Close() {};
	void WindowXCBOpengl::CursorLock([[maybe_unused]] int _x_position, [[maybe_unused]] int _y_position, [[maybe_unused]] int* _x_offset, [[maybe_unused]] int* _y_offset) {
		int iOffset_X = 0, iOffset_Y = 0;
        iOffset_X = _x_position - 960;
        iOffset_Y = _y_position - 540;
        
        *_x_offset += iOffset_X;
        *_y_offset -= iOffset_Y;

        if(*_y_offset > 890)
            *_y_offset = 890;
        else if(*_y_offset < -890)
            *_y_offset = -890;

		xcb_warp_pointer(connection, XCB_NONE, window, 0, 0, 0, 0, 960, 540);
		xcb_flush(connection);
	};
} // namespace GLVM::core
