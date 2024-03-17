// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#ifndef PROJECTILE_COMPONENT
#define PROJECTILE_COMPONENT

namespace GLVM::ecs::components
{
    class projectile
    {
    public:
		unsigned int owner;
        bool bCollision_Status_ = false;
        float fDamage_;
        float fSpeed_;
        float fFlying_Range_;
    };
}

#endif
