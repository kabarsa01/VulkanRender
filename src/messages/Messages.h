#ifndef _MESSAGES_H_
#define _MESSAGES_H_

namespace CGE
{

	enum MessageCode : unsigned int
	{
		MSG_GLOBAL_INIT = 0,
		MSG_GLOBAL_UPDATE = 1,

		MSG_SCENE_PROCESSING_FINISHED = 100,

		MSG_MAX_COUNT
	};

}

#endif
