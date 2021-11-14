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
	struct GlobalPreFrameMessage : Identifiable<GlobalPreFrameMessage>
	{
		float deltaTime;
		GlobalPreFrameMessage(float inDeltaTime) : deltaTime(inDeltaTime) {}
	};

	struct GlobalPreSceneMessage : Identifiable<GlobalPreSceneMessage>
	{
		float deltaTime;
		GlobalPreSceneMessage(float inDeltaTime) : deltaTime(inDeltaTime) {}
	};

	struct GlobalPostSceneMessage : Identifiable<GlobalPostSceneMessage>
	{
		float deltaTime;
		GlobalPostSceneMessage(float inDeltaTime) : deltaTime(inDeltaTime) {}
	};

	// per frame update called after scene and render update routine completion
	struct GlobalUpdateMessage : Identifiable<GlobalUpdateMessage>
	{
		float deltaTime;
		GlobalUpdateMessage(float inDeltaTime) : deltaTime(inDeltaTime) {}
	};

	struct GlobalPreRenderMessage : Identifiable<GlobalPreRenderMessage>
	{
		float deltaTime;
		GlobalPreRenderMessage(float inDeltaTime) : deltaTime(inDeltaTime) {}
	};

	struct GlobalPostRenderMessage : Identifiable<GlobalPostRenderMessage>
	{
		float deltaTime;
		GlobalPostRenderMessage(float inDeltaTime) : deltaTime(inDeltaTime) {}
	};

	// frame flip at the end of the frame processing after present was called
	struct GlobalPostFrameMessage : Identifiable<GlobalPostFrameMessage>
	{
		uint64_t frameCount;
		GlobalPostFrameMessage(uint64_t inFrameCount) : frameCount(inFrameCount) {}
	};

	struct SceneProcessingFinishedMessage : Identifiable<SceneProcessingFinishedMessage>
	{
		bool poop;
	};

}

#endif
