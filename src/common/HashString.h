#pragma once

#include <memory>
#include <string>
#include <map>
#include <unordered_map>

namespace CGE
{
	
	// Hashed string stores it's hash value and uses one global hash table to store
	// the actual strings. It means that there's only one instance of every string
	// used as a hash string. All the comparisons are made using stored hash which
	// makes them very cheap.
	class HashString
	{
	public:
		static HashString NONE;
	
		HashString();
		HashString(const std::string& inString);
		HashString(const char* inString);
		HashString(const HashString& inOther);
		HashString(HashString&& inOther);
		virtual ~HashString();
	
		const size_t GetHash() const;
		const std::string& GetString() const;
	
		HashString& operator= (const HashString& rhs) noexcept;
		HashString& operator= (HashString&& rhs) noexcept;
	
		bool operator== (const HashString& rhs) const noexcept;
		bool operator!= (const HashString& rhs) const noexcept;
		bool operator<(const HashString& other) const noexcept;
		bool operator>(const HashString& other) const noexcept;
		bool operator<=(const HashString& other) const noexcept;
		bool operator>=(const HashString& other) const noexcept;
	
		HashString operator+(const HashString& other) const noexcept;
		HashString operator+(const std::string& other) const noexcept;
		const std::string& operator*() const;
	private:
		static std::unordered_map<size_t, std::string> stringsMap;
	
		size_t hashValue;
		std::string* cachedString;
	};
	
}

namespace std {

	template <>
	struct hash<CGE::HashString>
	{
		std::size_t operator()(const CGE::HashString& inHashString) const
		{
			return inHashString.GetHash();
		}
	};

}
