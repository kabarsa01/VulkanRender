#ifndef _MESSAGES_H_
#define _MESSAGES_H_

#include "messages/Message.h"

namespace CGE
{

	struct GlobalInitMessage : MessageBase<GlobalInitMessage>
	{
		bool someRandomShitflag;
	};

	// per frame update called before scene and render update routine
	struct GlobalPreUpdateMessage : MessageBase<GlobalPreUpdateMessage>
	{
		float deltaTime;
		GlobalPreUpdateMessage(float inDeltaTime) : deltaTime(inDeltaTime) {}
	};

	// per frame update called after scene and render update routine completion
	struct GlobalUpdateMessage : MessageBase<GlobalUpdateMessage>
	{
		float deltaTime;
		GlobalUpdateMessage(float inDeltaTime) : deltaTime(inDeltaTime) {}
	};

	// frame flip at the end of the frame processing after present was called
	struct GlobalFlipMessage : MessageBase<GlobalFlipMessage>
	{
		uint64_t frameCount;
		GlobalFlipMessage(uint64_t inFrameCount) : frameCount(inFrameCount) {}
	};

	struct SceneProcessingFinishedMessage : MessageBase<SceneProcessingFinishedMessage>
	{
		bool poop;
	};

}

#endif
