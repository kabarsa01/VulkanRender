#ifndef _MESSAGE_HANDLER_H_
#define _MESSAGE_HANDLER_H_

#include "messages/Messages.h"

namespace CGE
{

	class IMessageHandler
	{
	public:
		virtual ~IMessageHandler() {}
		virtual void Handle(MessageCode msgCode) = 0;
	};

}

#endif
