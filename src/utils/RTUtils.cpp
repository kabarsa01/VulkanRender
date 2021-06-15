#include "RTUtils.h"
#include "scene/SceneObjectComponent.h"
#include "scene/SceneObjectBase.h"
#include "scene/Transform.h"

namespace CGE
{

	vk::AccelerationStructureGeometryTrianglesDataKHR RTUtils::GetGeometryTrianglesData(MeshDataPtr meshData)
	{
		vk::AccelerationStructureGeometryTrianglesDataKHR trisData;
		trisData.setVertexFormat(vk::Format::eR32G32B32Sfloat);
		trisData.setVertexStride(sizeof(Vertex));
		trisData.setVertexData(meshData->GetVertexBuffer().GetDeviceAddress());
		trisData.setMaxVertex(meshData->GetVertexCount());
		trisData.setIndexType(vk::IndexType::eUint32);
		trisData.setIndexData(meshData->GetIndexBuffer().GetDeviceAddress());
		trisData.setTransformData({}); // identity

		return trisData;
	}

	vk::AccelerationStructureGeometryInstancesDataKHR RTUtils::GetGeometryInstancesData(MeshDataPtr meshData)
	{
		vk::AccelerationStructureGeometryInstancesDataKHR instancesData;
		//instancesData.set

		return instancesData;
	}

	vk::AccelerationStructureInstanceKHR RTUtils::GetAccelerationStructureInstance(MeshComponentPtr meshComp)
	{
		vk::AccelerationStructureInstanceKHR instance;

		glm::mat4x4 mat = meshComp->GetParent()->transform.GetMatrix();
		mat = glm::transpose(mat);
		memcpy(&instance.transform, &mat, sizeof(instance.transform));

		instance.setInstanceShaderBindingTableRecordOffset(0);
		instance.setInstanceCustomIndex(0);

		return instance;
	}

}

