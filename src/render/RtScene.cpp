#include "RtScene.h"

namespace CGE
{

	RtScene::RtScene()
	{
		m_blasTable.reserve(1024 * 8);
		m_messageSubscriber.AddHandler<GlobalUpdateMessage>(this, &RtScene::HandleUpdate);
	}

	RtScene::~RtScene()
	{

	}

	void RtScene::HandleUpdate(std::shared_ptr<GlobalUpdateMessage> msg)
	{

	}

}

