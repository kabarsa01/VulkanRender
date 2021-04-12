#pragma once

#include <atomic>
#include <mutex>
#include <string>
#include <memory>
#include <typeinfo>
#include <map>

#include "common/HashString.h"
#include <set>

namespace CGE
{
	//class ObjectBase;
	//
	//class Class
	//{
	//public:
	//	template<class T>
	//	static const Class& Get();
	//	static const Class& Get(ObjectBase* InObject);
	//
	//	const HashString& GetName() const;
	//
	//	bool operator==(const Class& Other) const;
	//	bool operator!=(const Class& Other) const;
	//	bool operator<(const Class& Other) const;
	//	bool operator>(const Class& Other) const;
	//	bool operator<=(const Class& Other) const;
	//	bool operator>=(const Class& Other) const;
	//private:
	//	friend class ObjectBase;
	//	friend class std::shared_ptr<Class>;
	//	friend class std::_Ref_count_obj<Class>;
	//
	//	static std::atomic<uint32_t> m_globalId;
	//	static std::set<Class*> m_classes;
	//	static std::map<uint32_t, Class*> m_classMap;
	//	static std::map<size_t, std::shared_ptr<Class>> Classes;
	//	HashString Name;
	//
	//	constexpr Class();
	//	constexpr Class(const std::string& InName);
	//	constexpr Class(const HashString& InName);
	//	Class(const Class& InClass);
	//	~Class();
	//
	//	Class& operator=(const Class& Other);
	//	const Class* operator&() const;
	//
	//	static std::shared_ptr<Class> GetClass(const HashString& InName);
	//	static std::shared_ptr<Class> GetClass(ObjectBase* InObject);
	//};
	//
	//typedef std::shared_ptr<Class> ClassPtr;
	//
	////==========================================================================================
	////==========================================================================================
	//
	//template<class T>
	//inline const Class & Class::Get()
	//{
	//	return * GetClass( HashString{ typeid(T).name() } );
	//}

	//==========================================================================================
	//==========================================================================================

	namespace {
		std::atomic<uint32_t> g_globalClassId = 0;
	}

	class ClassBase
	{
	public:
		using ClassId = uint32_t;

		bool Equals(ClassBase* other) { return m_id == other->m_id; }
		bool operator==(ClassBase* other) { return m_id == other->m_id; }
		uint32_t GetId() { return m_id; }
	protected:
		uint32_t m_id;
	};

	template<typename T>
	class Class : public ClassBase
	{
	public:
		static Class<T>* Get()
		{
			if (m_class)
			{
				return m_class;
			}
			else
			{
				std::scoped_lock lock(m_mutex);
				if (!m_class)
				{
					m_class = new Class<T>();
				}
				return m_class;
			}
		}

		static uint32_t Id() { return Get()->m_id; }
	private:
		static Class<T>* m_class;
		static std::mutex m_mutex;
//		static uint32_t m_id;

		Class() { m_id = g_globalClassId.fetch_add(1); }
		~Class() {}
	};


	template<typename T>
	Class<T>* Class<T>::m_class = nullptr;
	template<typename T>
	std::mutex Class<T>::m_mutex;

	template<typename T>
	class ClassType
	{
	public:
		//ClassType()
		//{
		//	ObjectBase* base = dynamic_cast<ObjectBase*>(this);
		//	if (base)
		//	{
		//		base->instanceClass = Class<T>::Get();
		//	}
		//}
		//Class<T>* GetClass(std::shared_ptr<T> = nullptr)
		//{
		//	return Class<T>::Get();
		//}
	};

}
