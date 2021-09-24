#ifndef _MESSAGES_H_
#define _MESSAGES_H_

#include "utils/Identifiable.h"

namespace CGE
{

	struct GlobalInitMessage : Identifiable<GlobalInitMessage>
	{
		bool someRandomShitflag;
	};

	// per frame update called before scene and render update routine
	struct GlobalPreUpdateMessage : Identifiable<GlobalPreUpdateMessage>
	{
		float deltaTime;
		GlobalPreUpdateMessage(float inDeltaTime) : deltaTime(inDeltaTime) {}
	};

	// per frame update called after scene and render update routine completion
	struct GlobalUpdateMessage : Identifiable<GlobalUpdateMessage>
	{
		float deltaTime;
		GlobalUpdateMessage(float inDeltaTime) : deltaTime(inDeltaTime) {}
	};

	// frame flip at the end of the frame processing after present was called
	struct GlobalFlipMessage : Identifiable<GlobalFlipMessage>
	{
		uint64_t frameCount;
		GlobalFlipMessage(uint64_t inFrameCount) : frameCount(inFrameCount) {}
	};

	struct SceneProcessingFinishedMessage : Identifiable<SceneProcessingFinishedMessage>
	{
		bool poop;
	};

}

#endif
