// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#ifndef SOUND_ENGINE
#define SOUND_ENGINE

#include "Vector.hpp"
#include "Event.hpp"
#include <alsa/asoundlib.h>
#include <alsa/pcm.h>
#include "ISoundEngine.hpp"

namespace GLVM::core::Sound
{
    class CSoundEngineAlsa : public ISoundEngine
    {
        vector<CSoundSample*> tSound_Contaier;
    public:
        void SoundStream() override;
        void PlaybackSoundSample(CSoundSample& _sound_sample) override;
        void SetMasterVolume(long _lVolume) override;
        vector<CSoundSample*>& GetSoundContainer() override;
    };
}

#endif
