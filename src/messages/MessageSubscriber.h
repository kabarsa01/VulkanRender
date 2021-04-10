#ifndef _MESSAGE_SUBSCRIBER_H_
#define _MESSAGE_SUBSCRIBER_H_

#include <vector>
#include <map>
#include "messages/MessageHandler.h"

namespace CGE
{

	class MessageSubscriber
	{
	public:
		~MessageSubscriber();

		template<typename PayloadType, typename HandlerType, typename ...MessagesCodes>
		void AddHandler(HandlerType* handler, typename DelegateMessageHandler<PayloadType, HandlerType>::FuncPtr func, MessagesCodes... codes)
		{
			IMessageHandler* delegateHandler = new DelegateMessageHandler(handler, func);
			m_handlers.push_back(delegateHandler);
			(m_codes[delegateHandler].push_back(codes), ...);
		}

		void RegisterHandlers();
		void UnregisterHandlers();
	private:
		std::map<IMessageHandler*, std::vector<MessageCode>> m_codes;
		std::vector<IMessageHandler*> m_handlers;
	};

}

#endif
