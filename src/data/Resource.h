#pragma once

#include <string>
#include <memory>

#include "core/ObjectBase.h"
#include "common/HashString.h"

namespace CGE
{
	class Resource : public ObjectBase
	{
	public:
		Resource(HashString inId);
		virtual ~Resource();
	
		virtual void OnInitialize() override;
		virtual void OnDestroy() override;
		HashString GetResourceId();
	
		virtual bool Create() = 0;
		virtual bool Destroy();
		bool IsValid();
	protected:
		HashString id;
	
		void SetValid(bool inValid);
	private:
		bool isValidFlag = false;
	
		Resource();
	};
	
	typedef std::shared_ptr<Resource> ResourcePtr;
	
	
}
