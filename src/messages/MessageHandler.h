#ifndef _MESSAGE_HANDLER_H_
#define _MESSAGE_HANDLER_H_

#include <memory>
#include "messages/Messages.h"
#include "utils/Identifiable.h"

namespace CGE
{

//---------------------------------------------------------------------------------------

	// Base interface for message handler
	class IMessageHandler
	{
	public:
		bool isEnabled = true;

		virtual ~IMessageHandler() {}
		virtual void Handle(std::shared_ptr<IIdentifiable> message) = 0;
	};

//---------------------------------------------------------------------------------------

	// templated message handler wrapper to use already type cast payload
	template<typename T>
	class MessageHandler : public IMessageHandler
	{
	public:
		void Handle(std::shared_ptr<IIdentifiable> message) override
		{
			HandleMessage(std::dynamic_pointer_cast<T>(message));
		}
	protected:
		void HandleMessage(std::shared_ptr<T> message) = 0;
	};

//---------------------------------------------------------------------------------------

	// templated message handler wrapper to use pointer to member function
	template<typename MessageType, typename HandlerType>
	class DelegateMessageHandler : public IMessageHandler
	{
	public:
		using FuncPtr = void (HandlerType::*)(std::shared_ptr<MessageType>);

		DelegateMessageHandler(HandlerType* handler, FuncPtr func)
			: m_handler(handler)
			, m_func(func)
		{}

		void Handle(std::shared_ptr<IIdentifiable> message) override
		{
			(m_handler->*m_func)(std::dynamic_pointer_cast<MessageType>(message));
		}
	protected:
		HandlerType* m_handler;
		FuncPtr m_func;
	};

}

#endif
