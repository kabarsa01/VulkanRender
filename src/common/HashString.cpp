#include "HashString.h"

namespace
{
	std::string NONE("NONE");
	size_t NONEHash = std::hash<std::string>{}(NONE);
}

std::shared_ptr<std::string> WrapString(const std::string& InStr)
{
	return std::make_shared<std::string>( InStr );
}

std::map<size_t, std::shared_ptr<std::string>> HashString::StringsMap { { NONEHash, WrapString(::NONE) } };

HashString::HashString(std::string InString)
	: HashValue{ NONEHash }
	, CachedString{ StringsMap[NONEHash] }
{
	std::hash<std::string> Hash;
	HashValue = Hash(InString);

	if (StringsMap.find(HashValue) == StringsMap.end())
	{
		StringsMap.insert(std::pair<size_t, std::shared_ptr<std::string>> ( HashValue, WrapString(InString) ) );
	}
	CachedString = StringsMap[HashValue];
}

HashString::HashString(const char* InString)
	: HashString( std::string(InString) )
{
}

HashString::~HashString()
{
}

HashString HashString::NONE()
{
	return HashString(::NONE);
}

const size_t HashString::GetHash() const
{
	return HashValue;
}

const std::string & HashString::GetString() const
{
	return * CachedString;
}

bool HashString::operator==(const HashString & rhs) const noexcept
{
	return this->HashValue == rhs.HashValue;
}

bool HashString::operator!=(const HashString & rhs) const noexcept
{
	return this->HashValue != rhs.HashValue;
}

bool HashString::operator<(const HashString & Other) const noexcept
{
	return this->HashValue < Other.HashValue;
}

bool HashString::operator>(const HashString & Other) const noexcept
{
	return this->HashValue > Other.HashValue;
}

bool HashString::operator<=(const HashString & Other) const noexcept
{
	return this->HashValue <= Other.HashValue;
}

bool HashString::operator>=(const HashString & Other) const noexcept
{
	return this->HashValue >= Other.HashValue;
}

const std::string & HashString::operator*() const
{
	return * CachedString;
}

HashString::HashString()
	: HashValue{ NONEHash }
	, CachedString{ StringsMap[NONEHash] }
{
}
