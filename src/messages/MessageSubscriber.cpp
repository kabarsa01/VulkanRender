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

void CGE::MessageSubscriber::RegisterHandlers()
{
	for (IMessageHandler* h : m_handlers)
	{
		for (MessageCode code : m_codes[h])
		{
			MessageBus::GetInstance()->Register(h, code);
		}
	}
}

void CGE::MessageSubscriber::UnregisterHandlers()
{
	for (IMessageHandler* h : m_handlers)
	{
		MessageBus::GetInstance()->Unregister(h);
	}
}

