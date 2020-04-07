#include "scene/SceneObjectBase.h"
#include "scene/SceneObjectComponent.h"
#include "scene/Scene.h"
#include "core/Engine.h"

SceneObjectBase::SceneObjectBase()
{
}

SceneObjectBase::~SceneObjectBase()
{
}

void SceneObjectBase::OnInitialize()
{
	ObjectBase::OnInitialize();

	ScenePtr scene = Engine::GetInstance()->GetScene();
	scene->RegisterSceneObject(get_shared_from_this<SceneObjectBase>());

	IntializeComponents();
}

bool SceneObjectBase::RegisterComponent(std::shared_ptr<SceneObjectComponent> inComponent)
{
	components.push_back(inComponent);

	ScenePtr scene = Engine::GetInstance()->GetScene();
	scene->RegisterSceneObjectComponent(inComponent);

	return true;
}

void SceneObjectBase::Tick(float inDeltaTime)
{
}

void SceneObjectBase::TickComponents(float inDeltaTime)
{
	for (SceneObjectComponentPtr comp : components)
	{
		if (comp->isTickEnabled)
		{
			comp->TickComponent(inDeltaTime);
		}
	}
}

std::vector<SceneObjectComponentPtr> SceneObjectBase::GetComponents() const
{
	return components;
}

void SceneObjectBase::OnDestroy()
{
	ScenePtr Scene = Engine::GetInstance()->GetScene();
	ObjectBase::OnDestroy();
}

void SceneObjectBase::IntializeComponents()
{

}
