// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#include "SoundEngineAlsa.hpp"

namespace GLVM::core::Sound
{    
    void CSoundEngineAlsa::SoundStream()
    {
        for(unsigned int i = 0; i < tSound_Contaier.GetSize(); ++i)
        {
            // std::cout << (*tSound_Contaier[i]).kPath_to_File_ << std::endl;
            // std::cout << (*tSound_Contaier[i]).uiDuration_ << std::endl;
            // std::cout << (*tSound_Contaier[i]).uiRate_ << std::endl;
            PlaybackSoundSample(*tSound_Contaier[i]);
			tSound_Contaier.Remove(i);
//            tSound_Contaier.RemoveObject(tSound_Contaier[i]);
        }
    }

    void CSoundEngineAlsa::PlaybackSoundSample(CSoundSample& _sound_sample)
    {
        const char *kDevice = "default";
        snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;
//            snd_pcm_format_t format = SND_PCM_FORMAT_S24_LE;
        snd_pcm_access_t access = SND_PCM_ACCESS_RW_INTERLEAVED;
        unsigned int uiChannels = 2, uiRate;
        unsigned int uiLatency = 500000; /* 0.5 s */
        snd_pcm_t *pPcm;
        unsigned int uiFrame_Size = uiChannels * 2;

        uiRate = _sound_sample.uiRate_;
        (snd_pcm_open(&pPcm, kDevice, SND_PCM_STREAM_PLAYBACK, 0));
        (snd_pcm_set_params(pPcm, format, access, uiChannels, uiRate, 1, uiLatency));

#define FRAMES 32
        char* buf, *data;
        int frames, rest;
        FILE* iFile_Descritor;

        iFile_Descritor = fopen(_sound_sample.kPath_to_File_, "r");
            
        buf = (char*)malloc(FRAMES * uiFrame_Size);
        for (int i = 0; i < 300; ++i) {
            frames = fread(buf, uiFrame_Size, FRAMES, iFile_Descritor);
            if (frames <= 0)
                break;
            rest = frames;
            data = buf;
            while (rest > 0) {
                frames = snd_pcm_writei(pPcm, data, rest);
                // if (frames < 0)
                //     CHECK(snd_pcm_recover(pPcm, frames, 0));
                // else {
                rest -= frames;
                data += frames * uiFrame_Size;
//                    }
            }
        }
        free(buf);

        snd_pcm_drain(pPcm);
        snd_pcm_close(pPcm);
    }

    void CSoundEngineAlsa::SetMasterVolume(long _lVolume)
    {
        long lMin, lMax;
        snd_mixer_t* pHandle;
        snd_mixer_selem_id_t* pSid;
        const char* pCard = "default";
        const char* pSelem_Name = "Master";

        snd_mixer_open(&pHandle, 0);
        snd_mixer_attach(pHandle, pCard);
        snd_mixer_selem_register(pHandle, NULL, NULL);
        snd_mixer_load(pHandle);

        snd_mixer_selem_id_alloca(&pSid);
        snd_mixer_selem_id_set_index(pSid, 0);
        snd_mixer_selem_id_set_name(pSid, pSelem_Name);
        snd_mixer_elem_t* pElem = snd_mixer_find_selem(pHandle, pSid);

        snd_mixer_selem_get_playback_volume_range(pElem, &lMin, &lMax);
        snd_mixer_selem_set_playback_volume_all(pElem, _lVolume * lMax / 100);

        snd_mixer_close(pHandle);
    }
        
    vector<CSoundSample*>& CSoundEngineAlsa::GetSoundContainer() { return tSound_Contaier; }
}
