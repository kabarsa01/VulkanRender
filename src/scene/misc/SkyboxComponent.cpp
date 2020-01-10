#include "SkyboxComponent.h"
#include <array>

SkyboxComponent::SkyboxComponent(std::shared_ptr<SceneObjectBase> Parent)
	: SceneObjectComponent(Parent)
{

}

SkyboxComponent::~SkyboxComponent()
{
}

void SkyboxComponent::SetMeshData(MeshDataPtr InMeshData)
{
	MeshData = InMeshData;
}

void SkyboxComponent::OnInitialize()
{
	SceneObjectComponent::OnInitialize();
	// default quad mesh

}
