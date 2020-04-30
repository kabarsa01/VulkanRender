#include "HashString.h"

namespace
{
	std::string NONEString("NONE");
	size_t NONEHash = std::hash<std::string>{}(NONEString);
}

//--------------------------------------------------------------------------------
// static stuff
//--------------------------------------------------------------------------------
HashString HashString::NONE = HashString();
std::map<size_t, std::string> HashString::stringsMap { /*{ ::NONEHash, ::NONEString }*/ };
//--------------------------------------------------------------------------------

HashString::HashString()
	: hashValue{ NONEHash }
	, cachedString{ &NONEString }
{
}

HashString::HashString(const std::string& inString)
	: hashValue{ NONEHash }
	, cachedString{ &NONEString }
{
	std::hash<std::string> hash;
	hashValue = hash(inString);

	if (stringsMap.find(hashValue) == stringsMap.end())
	{
		stringsMap.insert(std::pair<size_t, std::string> ( hashValue, inString ) );
	}
	cachedString = &stringsMap[hashValue];
}

HashString::HashString(const char* inString)
	: HashString( std::string(inString) )
{
}

HashString::~HashString()
{
}

const size_t HashString::GetHash() const
{
	return hashValue;
}

const std::string & HashString::GetString() const
{
	return * cachedString;
}

bool HashString::operator==(const HashString & rhs) const noexcept
{
	return this->hashValue == rhs.hashValue;
}

bool HashString::operator!=(const HashString & rhs) const noexcept
{
	return this->hashValue != rhs.hashValue;
}

bool HashString::operator<(const HashString & other) const noexcept
{
	return this->hashValue < other.hashValue;
}

bool HashString::operator>(const HashString & other) const noexcept
{
	return this->hashValue > other.hashValue;
}

bool HashString::operator<=(const HashString & other) const noexcept
{
	return this->hashValue <= other.hashValue;
}

bool HashString::operator>=(const HashString & other) const noexcept
{
	return this->hashValue >= other.hashValue;
}

HashString HashString::operator+(const HashString& other) const noexcept
{
	return HashString(*cachedString + *other.cachedString);
}

HashString HashString::operator+(const std::string& other) const noexcept
{
	return HashString(*cachedString + other);
}

const std::string & HashString::operator*() const
{
	return * cachedString;
}


