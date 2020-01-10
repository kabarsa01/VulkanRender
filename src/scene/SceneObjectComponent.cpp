#include "scene/SceneObjectComponent.h"
#include "scene/SceneObjectBase.h"


SceneObjectComponent::SceneObjectComponent(std::shared_ptr<SceneObjectBase> InParent)
	: ObjectBase()
	, Parent(InParent)
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

	if (Parent)
	{
		Parent->RegisterComponent(get_shared_from_this<SceneObjectComponent>());
	}
}

std::shared_ptr<SceneObjectBase> SceneObjectComponent::GetParent()
{
	return Parent;
}

void SceneObjectComponent::TickComponent(float DeltaTime)
{
}

bool SceneObjectComponent::Register()
{
	return true;
}

