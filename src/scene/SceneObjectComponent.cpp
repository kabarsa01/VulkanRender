#include "scene/SceneObjectComponent.h"
#include "scene/SceneObjectBase.h"


SceneObjectComponent::SceneObjectComponent(std::shared_ptr<SceneObjectBase> inParent)
	: ObjectBase()
	, parent(inParent)
{
}

SceneObjectComponent::SceneObjectComponent()
	: ObjectBase()
{

}

SceneObjectComponent::~SceneObjectComponent()
{

}

void SceneObjectComponent::OnInitialize()
{
	ObjectBase::OnInitialize();

	if (parent)
	{
		parent->RegisterComponent(get_shared_from_this<SceneObjectComponent>());
	}
}

std::shared_ptr<SceneObjectBase> SceneObjectComponent::GetParent()
{
	return parent;
}

void SceneObjectComponent::TickComponent(float inDeltaTime)
{
}

bool SceneObjectComponent::Register()
{
	return true;
}

