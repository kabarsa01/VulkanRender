#ifndef _MESSAGES_H_
#define _MESSAGES_H_

#include "messages/Message.h"

namespace CGE
{

	struct GlobalInitMessage : MessageBase<GlobalInitMessage>
	{
		bool someRandomShitflag;
	};

	// per frame update called after scene and render update routine completion
	struct GlobalUpdateMessage : MessageBase<GlobalUpdateMessage>
	{
		float deltaTime;
	};

	// frame flip at the end of the frame processing after present was called
	struct GlobalFlipMessage : MessageBase<GlobalFlipMessage>
	{
		uint64_t frameCount;
	};

	struct SceneProcessingFinishedMessage : MessageBase<SceneProcessingFinishedMessage>
	{
		bool poop;
	};

}

#endif
