#pragma once

#include <string>
#include <memory>
#include <typeinfo>
#include <map>

#include "common/HashString.h"

class ObjectBase;

class Class
{
public:
	template<class T>
	static const Class& Get();
	static const Class& Get(ObjectBase* InObject);

	const HashString& GetName() const;

	bool operator==(const Class& Other) const;
	bool operator!=(const Class& Other) const;
	bool operator<(const Class& Other) const;
	bool operator>(const Class& Other) const;
	bool operator<=(const Class& Other) const;
	bool operator>=(const Class& Other) const;
private:
	friend class ObjectBase;
	friend class std::shared_ptr<Class>;
	friend class std::_Ref_count_obj<Class>;

	static std::map<size_t, std::shared_ptr<Class>> Classes;
	HashString Name;

	Class();
	Class(const std::string& InName);
	Class(const HashString& InName);
	Class(const Class& InClass);
	virtual ~Class();

	Class& operator=(const Class& Other);
	const Class* operator&() const;

	static std::shared_ptr<Class> GetClass(const HashString& InName);
	static std::shared_ptr<Class> GetClass(ObjectBase* InObject);
};

typedef std::shared_ptr<Class> ClassPtr;

//==========================================================================================
//==========================================================================================

template<class T>
inline const Class & Class::Get()
{
	return * GetClass( HashString{ typeid(T).name() } );
}




