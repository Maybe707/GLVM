// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#ifndef EVENT
#define EVENT

//#include "Stack.hpp"

namespace GLVM::core
{

	class CStack;
	
    /*! \enum EEvents
        \brief Realise event event types.
    */

    enum EEvents
    {
		eDEFAULT,
		eKEYRELEASE_A,
		eKEYRELEASE_D,
		eKEYRELEASE_S,
		eKEYRELEASE_W,
        eKEYRELEASE_JUMP,
        eGRAVITY_COLLISION_FLAG,
        eRENDER,
        eATACK,
        eSPAWN,
        eJUMP,
		eMOVE_FORWARD,
		eMOVE_BACKWARD,
		eMOVE_LEFT,
		eMOVE_RIGHT,
        eMOVE_DIAGONAL_FB,
        eMOVE_DIAGONAL_FL,
        eMOVE_DIAGONAL_LB,
        eMOVE_DIAGONAL_BR,
        eMOUSE_POINTER_POSITION,
        eMOUSE_LEFT_BUTTON_RELEASE,
        eMOUSE_LEFT_BUTTON,
		eMOUSE_RIGHT_BUTTON_RELEASE,
        eMOUSE_RIGHT_BUTTON,
        eGAME_LOOP_KILL,
        eEmpty,
    };

    struct SMousePointerPosition
    {
        int position_X;
        int position_Y;
        int offset_X = 0;
        int offset_Y = 0;
        float pitch;
        float yaw;
    };
    
    /*! \class Event
        \brief Realise event game system.
    */

    class CEvent
    {
        EEvents eEvent_;
		EEvents nextEvent;
    
    public:
        SMousePointerPosition mousePointerPosition;
		bool nextEventFlag = false;
        
        CEvent();
        EEvents& GetEvent();
        void SetEvent(EEvents _eEvent);
		void SetNextEvent(EEvents _eEvent);
		EEvents GetNextEvent();
		void SetLastEvent(CStack _Stack);
    };


}

#endif
