#include "messages/MessageBus.h"

#include "messages/MessageHandler.h"


namespace CGE
{

	MessageBus* MessageBus::m_instance = nullptr;

	MessageBus::MessageBus(uint32_t threadCount)
		: m_threadCount(threadCount)
	{

	}

	MessageBus::~MessageBus()
	{

	}

	void MessageBus::InitInstance(uint32_t threadCount)
	{
		if (!m_instance)
		{
			m_instance = new MessageBus(threadCount);
		}
	}

	void MessageBus::DestroyInstance()
	{
		if (m_instance)
		{
			delete m_instance;
			m_instance = nullptr;
		}
	}

	MessageBus* MessageBus::GetInstance()
	{
		return m_instance;
	}

	void MessageBus::Unregister(IMessageHandler* handler)
	{
		if (m_handlerMessageIds.find(handler) != m_handlerMessageIds.end())
		{
			for (IIdentifiable::Id msgId : m_handlerMessageIds[handler])
			{
				auto& handlersVec = m_messageIdHandlers[msgId];
				auto it = std::find(handlersVec.cbegin(), handlersVec.cend(), handler);
				if (it != handlersVec.cend())
				{
					handlersVec.erase(it);
				}
			}
			m_handlerMessageIds.erase(handler);
		}
	}

	void MessageBus::NotifyHandlers(std::shared_ptr<IIdentifiable> message)
	{
		for (IMessageHandler* handler : m_messageIdHandlers[message->GetId()])
		{
			if (handler->isEnabled)
			{
				handler->Handle(message);
			}
		}
	}

}

