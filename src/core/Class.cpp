#include "core/Class.h"
#include "core/ObjectBase.h"

std::map<size_t, std::shared_ptr<Class>> Class::Classes { };

const Class & Class::Get(ObjectBase * InObject)
{
	return * GetClass(HashString{ typeid(*InObject).name() });
}

const HashString & Class::GetName() const
{
	return Name;
}

bool Class::operator==(const Class & Other) const
{
	return this->Name == Other.Name;
}

bool Class::operator!=(const Class & Other) const
{
	return this->Name != Other.Name;
}

bool Class::operator<(const Class & Other) const
{
	return this->Name < Other.Name;
}

bool Class::operator>(const Class & Other) const
{
	return this->Name > Other.Name;
}

bool Class::operator<=(const Class & Other) const
{
	return this->Name <= Other.Name;
}

bool Class::operator>=(const Class & Other) const
{
	return this->Name >= Other.Name;
}

Class::Class()
	: Name( HashString::NONE() )
{
}

Class::Class(const std::string & InName)
	: Name( InName )
{
}

Class::Class(const HashString & InName)
	: Name( InName )
{
}

Class::Class(const Class & InClass)
	: Name( InClass.Name )
{
}

Class::~Class()
{
}

Class & Class::operator=(const Class & other)
{
	return *this;
}

const Class * Class::operator&() const
{
	return this;
}

std::shared_ptr<Class> Class::GetClass(const HashString & InName)
{
	if (Classes.find(InName.GetHash()) == Classes.end())
	{
		Classes.insert(std::pair<size_t, std::shared_ptr<Class>>(InName.GetHash(), std::make_shared<Class>(InName)));
	}

	return Classes.at(InName.GetHash());
}

std::shared_ptr<Class> Class::GetClass(ObjectBase * InObject)
{
	return GetClass(HashString{ typeid(*InObject).name() });
}
