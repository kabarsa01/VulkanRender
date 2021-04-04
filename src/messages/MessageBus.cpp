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
		if (m_handlerCodes.find(handler) != m_handlerCodes.end())
		{
			for (MessageCode code : m_handlerCodes[handler])
			{
				auto& handlersVec = m_codeHandlers[code];
				auto it = std::find(handlersVec.cbegin(), handlersVec.cend(), handler);
				if (it != handlersVec.cend())
				{
					handlersVec.erase(it);
				}
			}
			m_handlerCodes.erase(handler);
		}
	}

	void MessageBus::NotifyHandlers(MessageCode code)
	{
		for (IMessageHandler* handler : m_codeHandlers[code])
		{
			handler->Handle(code);
		}
	}

}

