#pragma once
#include <vector>
#include "resources/VulkanBuffer.h"
#include "resources/VulkanImage.h"
#include "data/MeshData.h"

class TransferList
{
public:
	static TransferList* GetInstance();

	void PushBuffer(VulkanBuffer* inBuffer);
	void PushBuffers(MeshDataPtr inData);
	void PushImage(VulkanImage* inImage);

	const std::vector<VulkanBuffer*>& GetBuffers();
	const std::vector<VulkanImage*>& GetImages();

	void ClearBuffers();
	void ClearImages();
private:
	static TransferList instance;

	std::vector<VulkanBuffer*> buffers;
	std::vector<VulkanImage*> images;

	TransferList();
	TransferList(const TransferList& inOther);
	void operator=(const TransferList& inOther);
	virtual ~TransferList();
};
