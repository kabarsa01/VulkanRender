#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include <atomic>
#include <mutex>

namespace CGE
{

	struct IMessage
	{
	public:
		using MessageId = uint32_t;

		bool Equals(IMessage* other) { return GetId() == other->GetId(); }
		virtual uint32_t GetId() = 0;
	protected:
		static std::atomic<uint32_t> m_messageId;
		static std::mutex m_mutex;
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
			if (m_id == UINT32_MAX)
			{
				std::scoped_lock lock(m_mutex);
				if (m_id == UINT32_MAX)
				{
					m_id = m_messageId.fetch_add(1);
				}
			}
			return m_id;
		}
	private:
		static uint32_t m_id;
	};

	template<typename T>
	uint32_t MessageBase<T>::m_id = UINT32_MAX;

}

#endif
