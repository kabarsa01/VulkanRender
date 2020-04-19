#include "TransferList.h"

TransferList TransferList::instance;

TransferList* TransferList::GetInstance()
{
	return &instance;
}

void TransferList::PushBuffer(VulkanBuffer* inBuffer)
{
	buffers.push_back(inBuffer);
}

void TransferList::PushBuffers(MeshDataPtr inData)
{
	buffers.push_back(&inData->GetVertexBuffer());
	buffers.push_back(&inData->GetIndexBuffer());
}

void TransferList::PushImage(VulkanImage* inImage)
{
	images.push_back(inImage);
}

const std::vector<VulkanBuffer*>& TransferList::GetBuffers()
{
	return buffers;
}

const std::vector<VulkanImage*>& TransferList::GetImages()
{
	return images;
}

void TransferList::ClearBuffers()
{
	buffers.clear();
}

void TransferList::ClearImages()
{
	images.clear();
}

TransferList::TransferList()
{

}

TransferList::TransferList(const TransferList& inOther)
{

}

TransferList::~TransferList()
{

}

void TransferList::operator=(const TransferList& inOther)
{

}

