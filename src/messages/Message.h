#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include <atomic>
#include <mutex>

namespace CGE
{
	namespace {
		std::atomic<uint32_t> g_messageId = 0;
		std::mutex g_mutex;
	}

	struct IMessage
	{
	public:
		using MessageId = uint32_t;

		bool Equals(IMessage* other) { return GetId() == other->GetId(); }
		virtual uint32_t GetId() = 0;
	};

	template<typename T>
	struct MessageBase : public IMessage
	{
	public:
		MessageBase()
		{
			Id();
		}

		uint32_t GetId() override { return m_id; }
		static uint32_t Id()
		{
			if (m_id != UINT32_MAX)
			{
				return m_id;
			}
			else
			{
				std::scoped_lock lock(g_mutex);
				if (m_id == UINT32_MAX)
				{
					m_id = g_messageId.fetch_add(1);
				}
				return m_id;
			}
		}
	private:
		static uint32_t m_id;
	};

	template<typename T>
	uint32_t MessageBase<T>::m_id = UINT32_MAX;

}

#endif
