// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#ifndef EXTRA_FUNCTIONS
#define EXTRA_FUNCTIONS

namespace GLVM::Extra
{    
	enum ETypes
	{
		eNONE,
		eUNSIGNED_INT,
		eINT,
		eDOUBLE,
		eFLOAT,
		eBOOL,
		eSHORT,
	};

	template <class T>
	struct STypes
	{
		static const ETypes eType = eNONE;
	};

	template <>
	struct STypes<unsigned int>
	{
		static const ETypes eType = eUNSIGNED_INT;
	};

	template <>
	struct STypes<int>
	{
		static const ETypes eType = eINT;
	};

	template <>
	struct STypes<double>
	{
		static const ETypes eType = eDOUBLE;
	};

	template <>
	struct STypes<float>
	{
		static const ETypes eType = eFLOAT;
	};

	template <>
	struct STypes<bool>
	{
		static const ETypes eType = eBOOL;
	};

	template <>
	struct STypes<short>
	{
		static const ETypes eType = eSHORT;
	};

	template<typename T>
	ETypes TypeEvaluator(T _Value)
	{
		switch(STypes<T>::eType)
		{
		case eUNSIGNED_INT:
			return eUNSIGNED_INT;
		case eINT:
			return eINT;
		case eDOUBLE:
			return eDOUBLE;
		case eFLOAT:
			return eFLOAT;
		case eBOOL:
			return eBOOL;
		case eSHORT:
			return eSHORT;
		default:
			return eNONE;
		}
	}


} ///< namespace GLVM::Extra 

#endif
