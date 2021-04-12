#ifndef _MESSAGE_BUS_H_
#define _MESSAGE_BUS_H_

#include <memory>
#include <map>
#include <vector>
#include "messages/Messages.h"
#include <any>

namespace CGE
{
	class IMessageHandler;

	class MessageBus
	{
	public:
		static void InitInstance(uint32_t threadCount);
		static void DestroyInstance();
		static MessageBus* GetInstance();

		template<typename ...MessagesTypes>
		void Register(IMessageHandler* handler);
		void Unregister(IMessageHandler* handler);

		template<typename ...MessagesTypes>
		void PublishSync(std::shared_ptr<MessagesTypes>... messages);
	private:
		static MessageBus* m_instance;

		uint32_t m_threadCount;
		std::map<IMessage::MessageId, std::vector<IMessageHandler*>> m_messageIdHandlers;
		std::map<IMessageHandler*, std::vector<IMessage::MessageId>> m_handlerMessageIds;

		MessageBus(uint32_t threadCount);
		MessageBus(const MessageBus&) = delete;
		MessageBus(MessageBus&&) = delete;
		MessageBus& operator=(const MessageBus&) = delete;
		MessageBus& operator=(MessageBus&&) = delete;
		~MessageBus();

		void NotifyHandlers(std::shared_ptr<IMessage> message);
	};

	template<typename ...MessagesTypes>
	void MessageBus::Register(IMessageHandler* handler)
	{
		(m_messageIdHandlers[MessagesTypes::Id()].push_back(handler), ...);
		(m_handlerMessageIds[handler].push_back(MessagesTypes::Id()), ...);
	}

	template<typename ...MessagesTypes>
	void MessageBus::PublishSync(std::shared_ptr<MessagesTypes>... messages)
	{
		(NotifyHandlers(messages),...);
	}

}

#endif