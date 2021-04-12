#ifndef _MESSAGES_H_
#define _MESSAGES_H_

#include "messages/Message.h"

namespace CGE
{

	struct GlobalInitMessage : MessageBase<GlobalInitMessage>
	{
		bool someRandomShitflag;
	};

	struct GlobalUpdateMessage : MessageBase<GlobalUpdateMessage>
	{
		float deltaTime;
	};

	struct SceneProcessingFinishedMessage : MessageBase<SceneProcessingFinishedMessage>
	{
		bool poop;
	};

}

#endif
