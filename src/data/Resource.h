#pragma once

#include <string>
#include <memory>

#include "core/ObjectBase.h"
#include "common/HashString.h"

class Resource : public ObjectBase
{
public:
	Resource(HashString InId);
	virtual ~Resource();

	virtual void OnInitialize() override;
	virtual void OnDestroy() override;
	HashString GetResourceId();

	virtual bool Load() = 0;
	virtual bool Unload() = 0;
	bool IsValid();
protected:
	HashString Id;

	void SetValid(bool InValid);
private:
	bool IsValidFlag = false;

	Resource();
};

typedef std::shared_ptr<Resource> ResourcePtr;


