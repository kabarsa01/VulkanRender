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
		void DestroyHint();
		bool IsValid();
	protected:
		friend class DataManager;
		HashString m_id;
	
		void SetValid(bool inValid);
		virtual bool Destroy() = 0;
	private:
		bool m_isValidFlag = false;
	
		Resource() = delete;
	};
	
	typedef std::shared_ptr<Resource> ResourcePtr;
	
	
}
