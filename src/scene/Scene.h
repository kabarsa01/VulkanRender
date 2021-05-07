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
	
	class Scene : public ObjectBase
	{
	public:
		Scene();
		virtual ~Scene();
	
		virtual void OnInitialize() override;
	
		void Init();
		
		void RegisterSceneObject(SceneObjectBasePtr inSceneObject);
		void RemoveSceneObject(SceneObjectBasePtr inSceneObject);
	
		void RegisterSceneObjectComponent(SceneObjectComponentPtr inSceneObjectComponent);
		void RemoveSceneObjectComponent(SceneObjectComponentPtr inSceneObjectComponent);	
	
		void PrepareObjectsLists();
		inline std::vector<HashString>& GetShadersList() { return shadersList; }
		inline std::unordered_map<HashString, std::vector<MaterialPtr>>& GetShaderToMaterial() { return shaderToMaterial; }
		inline std::unordered_map<HashString, std::vector<MeshDataPtr>>& GetMaterialToMeshData() { return materialToMeshData; }
		inline std::unordered_map<HashString, std::vector<glm::mat4>>& GetMeshDataToTransform(const HashString& materialId) { return matToMeshToTransform[materialId]; }
		inline std::unordered_map<HashString, uint32_t>& GetMeshDataToIndex(const HashString& materialId) { return materialToMeshDataToIndex[materialId]; }
		inline std::vector<glm::mat4>& GetModelMatrices() { return modelMatrices; }
		inline uint32_t GetRelevantMatricesCount() { return relevantMatricesCount; }
	
		void PerFrameUpdate();
	
		template<class T>
		std::set<SceneObjectComponentPtr> GetSceneComponents();
		template<class T>
		std::vector<std::shared_ptr<T>> GetSceneComponentsCast();
		template<class T>
		std::shared_ptr<T> GetSceneComponent();
	protected:
		std::set<SceneObjectBasePtr> sceneObjectsSet;
		std::map<HashString, std::set<SceneObjectBasePtr>> sceneObjectsMap;
		std::map<HashString, std::set<SceneObjectComponentPtr>> sceneObjectComponents;

		Octree<SceneObjectBasePtr>* sceneTree;
	
		// grouped ordered data for drawing stuff
		std::vector<HashString> shadersList;
		std::unordered_map<HashString, std::vector<MaterialPtr>> shaderToMaterial;
		std::unordered_map<HashString, std::vector<MeshDataPtr>> materialToMeshData;
		std::unordered_map<HashString, std::unordered_map<HashString, std::vector<glm::mat4>>> matToMeshToTransform;
		std::unordered_map<HashString, std::unordered_map<HashString, uint32_t>> materialToMeshDataToIndex;
		std::vector<glm::mat4> modelMatrices;
		uint32_t relevantMatricesCount;

		std::list<SceneObjectBasePtr> GatherObjectsInFrustum();
	};
	
	typedef std::shared_ptr<Scene> ScenePtr;
	
	//=============================================================================================================
	// TEMPLATED DEFINITIONS
	//=============================================================================================================
	
	template<class T>
	inline std::set<SceneObjectComponentPtr> Scene::GetSceneComponents()
	{
		HashString Key = Class::Get<T>().GetName();
		return sceneObjectComponents[Key];
	}
	
	//-------------------------------------------------------------------------------------------
	
	template<class T>
	inline std::vector<std::shared_ptr<T>> Scene::GetSceneComponentsCast()
	{
		HashString Key = Class::Get<T>().GetName();
		std::vector<std::shared_ptr<T>> Components;
		for (SceneObjectComponentPtr Comp : sceneObjectComponents[Key])
		{
			Components.push_back(ObjectBase::Cast<T, SceneObjectComponent>( Comp ));
		}
		return Components;
	}
	
	//-------------------------------------------------------------------------------------------
	
	template<class T>
	inline std::shared_ptr<T> Scene::GetSceneComponent()
	{
		HashString Key = Class::Get<T>().GetName();
		std::set<SceneObjectComponentPtr>& Components = sceneObjectComponents[Key];
		if (Components.size() > 0)
		{
			return ObjectBase::Cast<T, SceneObjectComponent>( * Components.begin() );
		}
		return nullptr;
	}
}
