// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#ifndef SOUND_ENGINE_FACTORY
#define SOUND_ENGINE_FACTORY

#include "ISoundEngine.hpp"

namespace GLVM::core::Sound
{    
/*!
  \brief Create sound engine enterface.

  This class creates a sound engine independent interface.
  Implemented by means of the factory method.
*/

    class CSoundEngineFactory
    {
    public:
        ISoundEngine* CreateSoundEngine();
    };
    
}
#endif
