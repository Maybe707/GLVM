// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#ifndef ANIMATION_SYSTEM
#define ANIMATION_SYSTEM

#include "ComponentManager.hpp"
#include "Event.hpp"
#include "Vector.hpp"
#include "VertexData.hpp"
#include "Stack.hpp"
#include "ISystem.hpp"
#include "GLPointer.h"
#include "Components/AnimationMoveComponent.hpp"
#include "Components/VertexComponent.hpp"
#include "ComponentManager.hpp"

#define ANIM_PER_AXIS_NUMBER 4

namespace GLVM::ecs
{

	class CAnimationSystem : public ecs::ISystem
	{
	public:
		core::EEvents eSave_Event_;
		static const int anim_index_array = 3;
		int anim_count = 0;
//		core::CStack& _Inputs;
		core::EEvents eEvent_;
		double Animation_Delta;
		double Delta_Time;
		float* Vertex_Animation[ANIM_PER_AXIS_NUMBER][anim_index_array] =
		{
			{vertices, vertices2, vertices3},
			{vertices4, vertices5, vertices6},
			{vertices7, vertices8, vertices9},
			{vertices10, vertices11, vertices12}
		};

        ///< Write one side of cube(if vertex component had vertices for 3D cube).
		void ArrayCopy(float* _aArray_Source, float* _aArray_Destination, unsigned int _u_iRange)
		{
			for(unsigned int i = 0; i < _u_iRange; ++i)
				_aArray_Destination[i] = _aArray_Source[i];
		}
		
		void Update() override
		{
            CComponentManager* pComponent_Manager = CComponentManager::GetInstance();
            core::vector<unsigned int>* pEntity_Container_refAnimationMove =
                ecs::GetEntityContainer<ecs::animation>(*pComponent_Manager);
            unsigned int uiVector_AnimationMove_Size = pEntity_Container_refAnimationMove->GetSize();

            
			for(int i = 0, iSize = uiVector_AnimationMove_Size; i < iSize; ++i)
			{
                unsigned int uiEntity_refAnimationMove = (*pEntity_Container_refAnimationMove)[i];
                                
				if(eEvent_ != eSave_Event_)
					Animation_Delta = 0.31f;
				eSave_Event_ = eEvent_;
				if(eEvent_ == core::eMOVE_BACKWARD)
				{
					Animation_Delta += Delta_Time;
					if(Animation_Delta > 0.3f)
					{
						switch(anim_count)
						{
						case 0:
							ArrayCopy(Vertex_Animation[0][anim_count], pComponent_Manager->GetComponent<ecs::vertex>(uiEntity_refAnimationMove).aVertex_.data(), 30);
							break;
						case 1:
							ArrayCopy(Vertex_Animation[0][anim_count], pComponent_Manager->GetComponent<ecs::vertex>(uiEntity_refAnimationMove).aVertex_.data(), 30);
							break;
						case 2:
							ArrayCopy(Vertex_Animation[0][anim_count], pComponent_Manager->GetComponent<ecs::vertex>(uiEntity_refAnimationMove).aVertex_.data(), 30);
							break;
						default:
							break;
						}
						++anim_count;
						if(anim_count == 3)
							anim_count = 0;
						Animation_Delta = 0;
						}
					}
					else if(eEvent_ == core::EEvents::eMOVE_LEFT)
					{
						Animation_Delta += Delta_Time;
						if(Animation_Delta > 0.3f)
						{
							switch(anim_count)
							{
							case 0:
								ArrayCopy(Vertex_Animation[1][anim_count], pComponent_Manager->GetComponent<ecs::vertex>(uiEntity_refAnimationMove).aVertex_.data(), 30);
								break;
							case 1:
								ArrayCopy(Vertex_Animation[1][anim_count], pComponent_Manager->GetComponent<ecs::vertex>(uiEntity_refAnimationMove).aVertex_.data(), 30);
								break;
							case 2:
								ArrayCopy(Vertex_Animation[1][anim_count], pComponent_Manager->GetComponent<ecs::vertex>(uiEntity_refAnimationMove).aVertex_.data(), 30);
								break;
							default:
								break;
							}
							++anim_count;
							if(anim_count == 3)
								anim_count = 0;
							Animation_Delta = 0;
						}
					}
					else if(eEvent_ == core::EEvents::eMOVE_RIGHT)
					{
						Animation_Delta += Delta_Time;
						if(Animation_Delta > 0.3f)
						{
							switch(anim_count)
							{
							case 0:
								ArrayCopy(Vertex_Animation[2][anim_count], pComponent_Manager->GetComponent<ecs::vertex>(uiEntity_refAnimationMove).aVertex_.data(), 30);
								break;
							case 1:
								ArrayCopy(Vertex_Animation[2][anim_count], pComponent_Manager->GetComponent<ecs::vertex>(uiEntity_refAnimationMove).aVertex_.data(), 30);
								break;
							case 2:
								ArrayCopy(Vertex_Animation[2][anim_count], pComponent_Manager->GetComponent<ecs::vertex>(uiEntity_refAnimationMove).aVertex_.data(), 30);
								break;
							default:
								break;
							}
							++anim_count;
							if(anim_count == 3)
								anim_count = 0;
							Animation_Delta = 0;
						}
					}
					else if(eEvent_ == core::EEvents::eMOVE_FORWARD)
					{
						Animation_Delta += Delta_Time;
						if(Animation_Delta > 0.3f)
						{
							switch(anim_count)
							{
							case 0:
								ArrayCopy(Vertex_Animation[3][anim_count], pComponent_Manager->GetComponent<ecs::vertex>(uiEntity_refAnimationMove).aVertex_.data(), 30);
								break;
							case 1:
								ArrayCopy(Vertex_Animation[3][anim_count], pComponent_Manager->GetComponent<ecs::vertex>(uiEntity_refAnimationMove).aVertex_.data(), 30);
								break;
							case 2:
								ArrayCopy(Vertex_Animation[3][anim_count], pComponent_Manager->GetComponent<ecs::vertex>(uiEntity_refAnimationMove).aVertex_.data(), 30);
								break;
							default:
								break;
							}
							++anim_count;
							if(anim_count == 3)
								anim_count = 0;
							Animation_Delta = 0;
						}
					}
				}
			}
		};

	}

#endif
