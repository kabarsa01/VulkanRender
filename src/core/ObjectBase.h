#pragma once

#include <memory>

class Class;

#define THIS_PTR(Type) std::dynamic_pointer_cast<Type>(shared_from_this())

class ObjectBase : public std::enable_shared_from_this<ObjectBase>
{
public:
	ObjectBase();
	virtual ~ObjectBase();

	const Class& GetClass();

	template <typename Derived>
	std::shared_ptr<Derived> get_shared_from_this();

	// static object creation methods
	template <typename Type, typename ...ArgTypes>
	static std::shared_ptr<Type> NewObject(ArgTypes ...Args);
	template <typename Type, typename OriginalType>
	static std::shared_ptr<Type> Cast(std::shared_ptr<OriginalType> InPointer);

	virtual void OnInitialize();
	virtual void OnDestroy();
private:
	std::shared_ptr<Class> InstanceClass;
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
inline std::shared_ptr<Type> ObjectBase::NewObject(ArgTypes ...Args)
{
	std::shared_ptr<Type> NewObjectPtr = std::make_shared<Type>(Args...);
	NewObjectPtr->OnInitialize();
	return NewObjectPtr;
}

template<typename Type, typename OriginalType>
inline std::shared_ptr<Type> ObjectBase::Cast(std::shared_ptr<OriginalType> InPointer)
{
	return std::dynamic_pointer_cast<Type, OriginalType>(InPointer);
}




