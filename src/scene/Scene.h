#pragma once

#include <map>
#include <unordered_map>
#include <set>
#include <list>
#include <vector>
#include <memory>

#include "core/ObjectBase.h"
#include "core/Class.h"
#include "common/HashString.h"
#include "glm/fwd.hpp"
#include "glm/detail/type_mat4x4.hpp"

namespace CGE
{
	
	template<class T>
	class Octree;

	class SceneObjectBase;
	typedef std::shared_ptr<SceneObjectBase> SceneObjectBasePtr;
	class SceneObjectComponent;
	typedef std::shared_ptr<SceneObjectComponent> SceneObjectComponentPtr;
	class Material;
	typedef std::shared_ptr<Material> MaterialPtr;
	class MeshData;
	typedef std::shared_ptr<MeshData> MeshDataPtr;

	//=======================================================================================================
	//=======================================================================================================

	struct SceneObjectsPack
	{
		std::set<SceneObjectBasePtr> objectsList;
		std::unordered_map<HashString, std::set<SceneObjectBasePtr>> objectsMap;
		std::unordered_map<HashString, std::set<SceneObjectComponentPtr>> componentsMap;

		void Add(SceneObjectBasePtr object);
		void Remove(SceneObjectBasePtr object);
		void Clear();

		template<class T>
		std::vector<std::shared_ptr<T>> GetComponentsCast();
		template<class T>
		std::set<SceneObjectComponentPtr>& GetComponents();
	};

	template<class T>
	std::vector<std::shared_ptr<T>> SceneObjectsPack::GetComponentsCast()
	{
		HashString key = Class::Get<T>().GetName();
		std::vector<std::shared_ptr<T>> components;
		for (SceneObjectComponentPtr comp : componentsMap[key])
		{
			components.push_back(ObjectBase::Cast<T, SceneObjectComponent>(comp));
		}
		return components;
	}

	template<class T>
	inline std::set<SceneObjectComponentPtr>& SceneObjectsPack::GetComponents()
	{
		HashString key = Class::Get<T>().GetName();
		return componentsMap[key];
	}

	//=======================================================================================================
	//=======================================================================================================

	struct MatrixPair
	{
		glm::mat4 previousMatrix;
		glm::mat4 matrix;
	};

	//=======================================================================================================
	//=======================================================================================================
	
	class Scene
	{
	public:
		Scene();
		virtual ~Scene();
	
		void Init();
		
		void RegisterSceneObject(SceneObjectBasePtr inSceneObject);
		void RemoveSceneObject(SceneObjectBasePtr inSceneObject);
	
		void RegisterSceneObjectComponent(SceneObjectComponentPtr inSceneObjectComponent);
		void RemoveSceneObjectComponent(SceneObjectComponentPtr inSceneObjectComponent);	
	
		void PrepareObjectsLists();
		inline std::vector<HashString>& GetShadersList() { return m_shadersList; }
		inline std::unordered_map<HashString, std::vector<MaterialPtr>>& GetShaderToMaterial() { return m_shaderToMaterial; }
		inline std::unordered_map<HashString, std::vector<MeshDataPtr>>& GetMaterialToMeshData() { return m_materialToMeshData; }
		inline std::unordered_map<HashString, std::vector<MatrixPair>>& GetMeshDataToTransform(const HashString& materialId) { return m_matToMeshToTransform[materialId]; }
		inline std::unordered_map<HashString, uint32_t>& GetMeshDataToIndex(const HashString& materialId) { return m_materialToMeshDataToIndex[materialId]; }
		inline std::vector<glm::mat4>& GetModelMatrices() { return m_modelMatrices; }
		inline std::vector<glm::mat4>& GetPreviousModelMatrices() { return m_previousModelMatrices; }
		inline uint32_t GetRelevantMatricesCount() { return m_relevantMatricesCount; }
	
		void PerFrameUpdate();

		SceneObjectsPack& GetObjectsPack(bool isFrustumCulled);
	
		template<class T>
		std::set<SceneObjectComponentPtr> GetSceneComponents();
		template<class T>
		std::vector<std::shared_ptr<T>> GetSceneComponentsCast();
		template<class T>
		std::vector<std::shared_ptr<T>> GetSceneComponentsInFrustumCast();
		template<class T>
		std::shared_ptr<T> GetSceneComponent();
	protected:
		SceneObjectsPack m_primaryPack;
		SceneObjectsPack m_frustumPack;

		//std::set<SceneObjectBasePtr> sceneObjectsSet;
		//std::map<HashString, std::set<SceneObjectBasePtr>> sceneObjectsMap;
		//std::map<HashString, std::set<SceneObjectComponentPtr>> sceneObjectComponents;

		Octree<SceneObjectBasePtr>* m_sceneTree;
	
		// grouped ordered data for drawing stuff
		std::vector<HashString> m_shadersList;
		std::unordered_map<HashString, std::vector<MaterialPtr>> m_shaderToMaterial;
		std::unordered_map<HashString, std::vector<MeshDataPtr>> m_materialToMeshData;
		std::unordered_map<HashString, std::unordered_map<HashString, std::vector<MatrixPair>>> m_matToMeshToTransform;
		std::unordered_map<HashString, std::unordered_map<HashString, uint32_t>> m_materialToMeshDataToIndex;
		std::vector<glm::mat4> m_modelMatrices;
		std::vector<glm::mat4> m_previousModelMatrices;
		uint32_t m_relevantMatricesCount;

		void GatherObjectsInFrustum();

		template<class T>
		std::set<SceneObjectComponentPtr>& GetSceneComponents(SceneObjectsPack& objectPack);
		template<class T>
		std::vector<std::shared_ptr<T>> GetSceneComponentsCast(SceneObjectsPack& objectPack);
		template<class T>
		std::shared_ptr<T> GetSceneComponent(SceneObjectsPack& objectPack);
	};
	
	//=============================================================================================================
	// TEMPLATED DEFINITIONS
	//=============================================================================================================
	
	template<class T>
	inline std::set<SceneObjectComponentPtr>& Scene::GetSceneComponents(SceneObjectsPack& objectPack)
	{
		HashString Key = Class::Get<T>().GetName();
		return objectPack.componentsMap[Key];
	}

	//-------------------------------------------------------------------------------------------
	
	template<class T>
	std::set<SceneObjectComponentPtr> Scene::GetSceneComponents()
	{
		return GetSceneComponents<T>(m_primaryPack);
	}

	//-------------------------------------------------------------------------------------------
	
	template<class T>
	inline std::vector<std::shared_ptr<T>> Scene::GetSceneComponentsCast(SceneObjectsPack& objectPack)
	{
		HashString Key = Class::Get<T>().GetName();
		std::vector<std::shared_ptr<T>> Components;
		for (SceneObjectComponentPtr Comp : objectPack.componentsMap[Key])
		{
			Components.push_back(ObjectBase::Cast<T, SceneObjectComponent>( Comp ));
		}
		return Components;
	}

	//-------------------------------------------------------------------------------------------
	
	template<class T>
	std::vector<std::shared_ptr<T>> Scene::GetSceneComponentsCast()
	{
		return GetSceneComponentsCast<T>(m_primaryPack);
	}

	template<class T>
	std::vector<std::shared_ptr<T>>
		Scene::GetSceneComponentsInFrustumCast()
	{
		return GetSceneComponentsCast<T>(m_frustumPack);
	}

	//-------------------------------------------------------------------------------------------
	
	template<class T>
	inline std::shared_ptr<T> Scene::GetSceneComponent(SceneObjectsPack& objectPack)
	{
		HashString Key = Class::Get<T>().GetName();
		std::set<SceneObjectComponentPtr>& Components = objectPack.componentsMap[Key];
		if (Components.size() > 0)
		{
			return ObjectBase::Cast<T, SceneObjectComponent>( * Components.begin() );
		}
		return nullptr;
	}

	//-------------------------------------------------------------------------------------------

	template<class T>
	std::shared_ptr<T> Scene::GetSceneComponent()
	{
		return GetSceneComponent<T>(m_primaryPack);
	}

}
