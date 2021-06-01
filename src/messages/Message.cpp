#include <atomic>
#include <mutex>
#include "Message.h"

namespace CGE
{

	std::atomic<uint32_t> IMessage::m_messageId;
	std::mutex IMessage::m_mutex;

}

