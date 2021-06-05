#ifndef _MESSAGE_SUBSCRIBER_H_
#define _MESSAGE_SUBSCRIBER_H_

#include <vector>
#include <map>
#include "messages/MessageHandler.h"
#include "messages/MessageBus.h"

namespace CGE
{

	class MessageSubscriber
	{
	public:
		~MessageSubscriber();

		template<typename MessageType, typename HandlerType>
		void AddHandler(HandlerType* handler, typename DelegateMessageHandler<MessageType, HandlerType>::FuncPtr func)
		{
			IMessageHandler* delegateHandler = new DelegateMessageHandler(handler, func);
			m_handlers.push_back(delegateHandler);
			MessageBus::GetInstance()->Register<MessageType>(delegateHandler);
		}

		void EnableHandlers(bool enableFlag);
		void UnregisterHandlers();
	private:
		std::vector<IMessageHandler*> m_handlers;
	};

}

#endif
