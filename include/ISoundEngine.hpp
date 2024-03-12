// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#ifndef ISOUND_ENGINE
#define ISOUND_ENGINE

#include "Vector.hpp"

namespace GLVM::core::Sound
{
    struct CSoundSample
    {
        const char* kPath_to_File_;
        unsigned int uiDuration_;
        unsigned int uiRate_;
    };
    
    class ISoundEngine
    {
    public:
        virtual ~ISoundEngine() {}

        virtual vector<CSoundSample*>& GetSoundContainer() = 0;
        virtual void PlaybackSoundSample(CSoundSample& _sound_sample) = 0;
        virtual void SetMasterVolume(long _lVolume) = 0;
        virtual void SoundStream() = 0;
    };
}

#endif
