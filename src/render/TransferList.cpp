#include "TransferList.h"

namespace CGE
{
	TransferList TransferList::instance;
	
	TransferList* TransferList::GetInstance()
	{
		return &instance;
	}
	
	void TransferList::PushBuffer(BufferDataPtr inBuffer)
	{
		buffers.push_back(inBuffer);
	}
	
	void TransferList::PushImage(TextureDataPtr inImage)
	{
		images.push_back(inImage);
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
	
}
