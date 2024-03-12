#include "Systems/ControllerSystem.hpp"
#include "Event.hpp"

namespace GLVM::core
{
	ControllerSystem::ControllerSystem(CStack& inputStack, CEvent& event) : inputStack_(inputStack), event_(event) {}

	void ControllerSystem::Update() {
	}
}
