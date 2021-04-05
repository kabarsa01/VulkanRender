#ifndef _MESSAGE_BUS_H_
#define _MESSAGE_BUS_H_

#include <memory>
#include <map>
#include <vector>
#include "messages/Messages.h"

namespace CGE
{
	class IMessageHandler;

	class MessageBus
	{
	public:
		static void InitInstance(uint32_t threadCount);
		static void DestroyInstance();
		static MessageBus* GetInstance();

		template<typename ...MessageCodes>
		void Register(IMessageHandler* handler, MessageCodes... codes);
		void Unregister(IMessageHandler* handler);

		template<typename ...MessageCodes>
		void PublishSync(MessageCodes... messageCodes);
	private:
		static MessageBus* m_instance;

		uint32_t m_threadCount;
		std::map<MessageCode, std::vector<IMessageHandler*>> m_codeHandlers;
		std::map<IMessageHandler*, std::vector<MessageCode>> m_handlerCodes;

		MessageBus(uint32_t threadCount);
		MessageBus(const MessageBus&) = delete;
		MessageBus(MessageBus&&) = delete;
		MessageBus& operator=(const MessageBus&) = delete;
		MessageBus& operator=(MessageBus&&) = delete;
		~MessageBus();

		void NotifyHandlers(MessageCode code);
	};

	template<typename ...MessageCodes>
	void MessageBus::Register(IMessageHandler* handler, MessageCodes... codes)
	{
		(m_codeHandlers[codes].push_back(handler), ...);
		(m_handlerCodes[handler].push_back(codes), ...);
	}

	template<typename  ...MessageCodes>
	void MessageBus::PublishSync(MessageCodes... messageCodes)
	{
		(NotifyHandlers(messageCodes),...);
	}

}

#endif