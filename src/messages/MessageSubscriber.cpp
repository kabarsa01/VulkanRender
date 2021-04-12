#include "messages/MessageSubscriber.h"
#include "messages/MessageBus.h"

CGE::MessageSubscriber::~MessageSubscriber()
{
	UnregisterHandlers();
	for (IMessageHandler* h : m_handlers)
	{
		delete h;
	}
}

void CGE::MessageSubscriber::EnableHandlers(bool enableFlag)
{
	for (IMessageHandler* h : m_handlers)
	{
		h->isEnabled = enableFlag;
	}
}

void CGE::MessageSubscriber::UnregisterHandlers()
{
	for (IMessageHandler* h : m_handlers)
	{
		MessageBus::GetInstance()->Unregister(h);
	}
}

