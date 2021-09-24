#ifndef __IDENTIFIABLE_H__ 
#define __IDENTIFIABLE_H__

#include <mutex>
#include <atomic>

namespace CGE
{

	class IIdentifiable
	{
	public:
		using Id = uint64_t;

		bool Equals(IIdentifiable* other) { return GetId() == other->GetId(); }
		virtual uint64_t GetId() = 0;
	protected:
		static std::atomic<uint64_t> m_idCounter;
		static std::mutex m_mutex;
	};

	template<typename T>
	struct Identifiable : public IIdentifiable
	{
	public:
		Identifiable()
		{
			Id();
		}

		uint64_t GetId() override { return m_id; }
		static uint64_t Id()
		{
			if (m_id == UINT64_MAX)
			{
				std::scoped_lock lock(m_mutex);
				if (m_id == UINT64_MAX)
				{
					m_id = m_idCounter.fetch_add(1);
				}
			}
			return m_id;
		}
	private:
		static uint64_t m_id;
	};

	template<typename T>
	uint64_t Identifiable<T>::m_id = UINT64_MAX;

}

#endif
