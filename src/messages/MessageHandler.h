#ifndef _MESSAGE_HANDLER_H_
#define _MESSAGE_HANDLER_H_

#include <any>
#include "messages/Messages.h"

namespace CGE
{

//---------------------------------------------------------------------------------------

	// Base interface for message handler
	class IMessageHandler
	{
	public:
		bool isEnabled = true;

		virtual ~IMessageHandler() {}
		virtual void Handle(MessageCode msgCode, std::any& payload) = 0;
	};

//---------------------------------------------------------------------------------------

	// templated message handler wrapper to use already type cast payload
	template<typename T>
	class MessageHandler : public IMessageHandler
	{
	public:
		void Handle(MessageCode msgCode, std::any& payload) override
		{
			HandleMessage(msgCode, std::any_cast<T>(payload));
		}
	protected:
		void HandleMessage(MessageCode msgCode, T payload) = 0;
	};

//---------------------------------------------------------------------------------------

	// templated message handler wrapper to use pointer to member function
	template<typename PayloadType, typename HandlerType>
	class DelegateMessageHandler : public IMessageHandler
	{
	public:
		using FuncPtr = void (HandlerType::*)(MessageCode, PayloadType);

		DelegateMessageHandler(HandlerType* handler, FuncPtr func)
			: m_handler(handler)
			, m_func(func)
		{}

		void Handle(MessageCode msgCode, std::any& payload) override
		{
			(m_handler->*m_func)(msgCode, std::any_cast<PayloadType>(payload));
		}
	protected:
		HandlerType* m_handler;
		FuncPtr m_func;
	};

}

#endif
