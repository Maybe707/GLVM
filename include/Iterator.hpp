// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#ifndef I_ITERATOR
#define I_ITERATOR

namespace GLVM::core
{
	template <class T>
	class Iterator
	{
	public:
		virtual ~Iterator() {}

	    virtual bool Next()        = 0;
		virtual bool ValidStatus() = 0;
		virtual T& Current()       = 0;
		virtual T& Last()          = 0;
	};
}

#endif
