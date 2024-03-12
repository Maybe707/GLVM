// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#include "SoundEngineWaveform.hpp"
#include <cstdio>
#include <fstream>
#include <iostream>
#include <mmeapi.h>

namespace GLVM::core::Sound
{
    void CSoundEngineWaveform::SoundStream()
    {
        for(unsigned int i = 0; i < tSound_Container.GetSize(); ++i)
        {
            PlaybackSoundSample(*tSound_Container[i]);
			tSound_Container.Remove(i);
//            tSound_Contaier.RemoveObject(tSound_Contaier[i]);
        }
    }

    void CSoundEngineWaveform::PlaybackSoundSample(CSoundSample& _sound_sample)
    {
//        MMRESULT     rc;
        HWAVEOUT     hWaveOut;
        WAVEHDR      lpWaveHdr {};
        WAVEFORMATEX Format;
		
//        Format.wFormatTag = WAVE_FORMAT_PCM;
		Format.wFormatTag = WAVE_FORMAT_PCM; 
        Format.nChannels = 2; 
        Format.nSamplesPerSec = _sound_sample.uiRate_; 
        Format.nAvgBytesPerSec = Format.nSamplesPerSec * Format.nChannels * 2; 
        Format.nBlockAlign = 4;                                                        ///< Change this field first if got any problems
        Format.wBitsPerSample = 16; 
        Format.cbSize = 0;
 
        /// Open a waveform device for output using window callback.

		unsigned int rc = 0;
        rc = waveOutOpen (&hWaveOut, WAVE_MAPPER, &Format, 0L, 0L, 0L);
         if(rc != MMSYSERR_NOERROR) {
             std::cerr << "waveOutOpen: " << "error code: " << rc << std::endl;;
 //            print_waveout_error(rc);        ///< MAKE DIFINITION!
             std::exit(-1);
         }

        std::ifstream file(_sound_sample.kPath_to_File_, std::ios_base::binary | std::ios_base::in);
        if(!file) {
            std::cerr << "Fail to open file." << std::endl;
            std::exit(-1);
        }
        
        char *buf = (char*)malloc(Format.nAvgBytesPerSec * 2);
//        frames = fread(buf, uiFrame_Size, FRAMES, iFile_Descritor);

        /// After allocation, set up and prepare header.

//        char* data_ptr = buf + 11;

        while(1)
        {
            file.read(buf, Format.nAvgBytesPerSec * 2);
            if(file.gcount() == 0)
                break;
            
            lpWaveHdr.lpData = buf;
            lpWaveHdr.dwBufferLength = file.gcount();
            lpWaveHdr.dwFlags = 0L;
            lpWaveHdr.dwLoops = 0L;
            waveOutPrepareHeader(hWaveOut, &lpWaveHdr, sizeof(WAVEHDR));
            waveOutWrite(hWaveOut, &lpWaveHdr, sizeof(WAVEHDR));
            Sleep((lpWaveHdr.dwBufferLength * 1000) / (Format.nAvgBytesPerSec * 2));
            waveOutUnprepareHeader(hWaveOut, &lpWaveHdr, sizeof(WAVEHDR));
        }
        
        free(buf);
        waveOutClose(hWaveOut);
    }

    void CSoundEngineWaveform::SetMasterVolume(long _lVolume) {}
    vector<CSoundSample*>& CSoundEngineWaveform::GetSoundContainer() { return tSound_Container; }
}

// #define FRAMES 32
//         HWAVEOUT hWaveOut = 0;
//         WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, FRAMES, FRAMES, 1, 8, 0 };
//         waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
// //        char buffer[8000 * 60] = {};

//         char* buf, *data;
//         WAVEHDR header;

//         unsigned int uiChannels = 2;
//         unsigned int uiFrame_Size = uiChannels * 2;

//         int frames, rest;
//         FILE* iFile_Descritor;

//         iFile_Descritor = fopen(_sound_sample.kPath_to_File_, "r");
            
//         buf = (char*)malloc(FRAMES * uiFrame_Size);
//         for (;;) {
//             frames = fread(buf, uiFrame_Size, FRAMES, iFile_Descritor);
//             if (frames <= 0)
//                 break;
//             rest = frames;
//             data = buf;
//             header = { buf, sizeof(buf), 0, 0, 0, 0, 0, 0 };
//             waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
//             while (rest > 0) {
//                 frames = waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));
//                 // if (frames < 0)
//                 //     CHECK(snd_pcm_recover(pPcm, frames, 0));
//                 // else {
//                 rest -= frames;
//                 data += frames * uiFrame_Size;
// //                    }
//             }
//         }
//         free(buf);

//         waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
//         waveOutClose(hWaveOut);
// //        Sleep(60 * 1000);










// Gekmi's code:

// #include <windows.h>
// #include <iostream>
// #include <fstream>
// #include <cstdlib>
 
// void print_waveout_error(MMRESULT error_code) {
//   static CHAR msg[MAXERRORLENGTH];
 
//   std::cerr << "Error " << error_code << ": ";
//   if(waveOutGetErrorText(error_code, msg, MAXERRORLENGTH) != MMSYSERR_NOERROR)
//     std::cerr << "(NULL)" << std::endl;
//   else
//     std::cerr << msg << std::endl;
// }
 
// int main(int argc, char *argv[]) {
//   if(argc != 4)
//     std::exit(-1);
 
//   MMRESULT rc;
//   HWAVEOUT hwo;
//   WAVEFORMATEX wfmt;
 
//   wfmt.wFormatTag      = WAVE_FORMAT_PCM;
//   wfmt.nChannels       = std::atoi(argv[2]);
//   wfmt.nSamplesPerSec  = std::atoi(argv[3]);
//   wfmt.nAvgBytesPerSec = wfmt.nSamplesPerSec * wfmt.nChannels * 2;
//   wfmt.nBlockAlign     = 2;
//   wfmt.wBitsPerSample  = 16;
//   wfmt.cbSize          = 0;
 
//   if(wfmt.nAvgBytesPerSec == 0) {
//     std::cerr << "Invalid value of frequence or channels number." << std::endl;
//     std::exit(-1);
//   }
 
//   rc = waveOutOpen(&hwo, WAVE_MAPPER, &wfmt, 0, 0, CALLBACK_NULL);
//   if(rc != MMSYSERR_NOERROR) {
//     std::cerr << "waveOutOpen: ";
//     print_waveout_error(rc);
//     std::exit(-1);
//   }
 
//   std::ifstream file(argv[1], std::ios_base::binary | std::ios_base::in);
//   if(!file) {
//     std::cerr << "Fail to open file." << std::endl;
//     std::exit(-1);
//   }
 
//   char *buffer = new char[wfmt.nAvgBytesPerSec];
 
//   while(file) {
//     WAVEHDR whdr {};
 
//     file.read(buffer, wfmt.nAvgBytesPerSec);
//     if(file.gcount() == 0)
//       break;
 
//     whdr.lpData         = buffer;
//     whdr.dwBufferLength = file.gcount();
//     whdr.dwFlags        = WHDR_ENDLOOP;
//     whdr.dwLoops        = 1;
 
//     waveOutPrepareHeader(hwo, &whdr, sizeof whdr);
//     waveOutWrite(hwo, &whdr, sizeof whdr);
//     Sleep(1000);
//     waveOutUnprepareHeader(hwo, &whdr, sizeof whdr);
 
//     std::cout << "loop" << std::endl;
//   }
 
//   delete[] buffer;
//   return 0;
// }
