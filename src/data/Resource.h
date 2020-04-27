#pragma once

#include <string>
#include <memory>

#include "core/ObjectBase.h"
#include "common/HashString.h"

class Resource : public ObjectBase
{
public:
	Resource(HashString inId);
	virtual ~Resource();

	virtual void OnInitialize() override;
	virtual void OnDestroy() override;
	HashString GetResourceId();

	virtual bool Load() = 0;
	virtual bool Cleanup() = 0;
	bool IsValid();
protected:
	HashString id;

	void SetValid(bool inValid);
private:
	bool isValidFlag = false;

	Resource();
};

typedef std::shared_ptr<Resource> ResourcePtr;


