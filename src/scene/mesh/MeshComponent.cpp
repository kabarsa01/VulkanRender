#include "scene/mesh/MeshComponent.h"
#include <array>

// default mesh data is a quad
//namespace
//{
	float vertices[] = {
		// coords              // color             // uv coords
		 0.5f,  0.5f, 0.0f,    1.0f, 0.0f, 0.0f,    1.0f, 1.0f,  // top right
		 0.5f, -0.5f, 0.0f,    0.0f, 1.0f, 0.0f,    1.0f, 0.0f,  // bottom right
		-0.5f, -0.5f, 0.0f,    0.0f, 0.0f, 1.0f,    0.0f, 0.0f,  // bottom left
		-0.5f,  0.5f, 0.0f,    1.0f, 1.0f, 0.0f,    0.0f, 1.0f   // top left 
	};

	unsigned int indices[] = {
		0, 1, 3,   // first triangle
		1, 2, 3    // second triangle
	};
//}

MeshComponent::MeshComponent(std::shared_ptr<SceneObjectBase> Parent)
	: SceneObjectComponent(Parent)
{

}

MeshComponent::~MeshComponent()
{
}

void MeshComponent::SetMeshData(MeshDataPtr InMeshData)
{
	MeshData = InMeshData;
}

void MeshComponent::OnInitialize()
{
	SceneObjectComponent::OnInitialize();
	// default quad mesh

}
