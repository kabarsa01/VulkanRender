#pragma once
#include <vector>
#include "data/MeshData.h"
#include "data/TextureData.h"

namespace CGE
{
	class TransferList
	{
	public:
		static TransferList* GetInstance();
	
		void PushBuffer(BufferDataPtr inBuffer);
		void PushBuffers(MeshDataPtr inData);
		void PushImage(TextureDataPtr inImage);
	
		const std::vector<BufferDataPtr>& GetBuffers() { return buffers; }
		const std::vector<TextureDataPtr>& GetImages() { return images; }
	
		void ClearBuffers();
		void ClearImages();
	private:
		static TransferList instance;
	
		std::vector<BufferDataPtr> buffers;
		std::vector<TextureDataPtr> images;
	
		TransferList();
		TransferList(const TransferList& inOther);
		void operator=(const TransferList& inOther);
		virtual ~TransferList();
	};
}
