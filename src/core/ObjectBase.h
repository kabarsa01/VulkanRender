#pragma once

#include <memory>
#include <type_traits>
#include "Class.h"

namespace CGE
{	
	#define THIS_PTR(Type) std::dynamic_pointer_cast<Type>(shared_from_this())
	
	class ObjectBase : public std::enable_shared_from_this<ObjectBase>, public ClassType<ObjectBase>
	{
	public:
		ObjectBase();
		virtual ~ObjectBase();
	
		template <typename Derived>
		std::shared_ptr<Derived> get_shared_from_this();

		ClassBase* GetClass() { return instanceClass; }
	
		// static object creation methods
		template <typename Type, typename ...ArgTypes>
		static std::shared_ptr<Type> NewObject(ArgTypes&& ...args);
		template <typename Type, typename OriginalType>
		static std::shared_ptr<Type> Cast(std::shared_ptr<OriginalType> InPointer);
	
		virtual void OnInitialize();
		virtual void OnDestroy();
	private:
		ClassBase* instanceClass;
	};
	
	//-----------------------------------------------------------------------------------
	// Templated definitions
	//-----------------------------------------------------------------------------------
	
	template <typename Derived>
	std::shared_ptr<Derived> ObjectBase::get_shared_from_this()
	{
		return std::dynamic_pointer_cast<Derived, ObjectBase>(shared_from_this());
	}
	
	template<typename Type, typename ...ArgTypes>
	inline std::shared_ptr<Type> ObjectBase::NewObject(ArgTypes&& ...args)
	{
		std::shared_ptr<Type> NewObjectPtr = std::make_shared<Type>(std::forward<ArgTypes>(args)...);
		NewObjectPtr->OnInitialize();
		return NewObjectPtr;
	}
	
	template<typename Type, typename OriginalType>
	inline std::shared_ptr<Type> ObjectBase::Cast(std::shared_ptr<OriginalType> InPointer)
	{
		return std::dynamic_pointer_cast<Type, OriginalType>(InPointer);
	}
	
}
