#ifndef _MT_VECTOR_H_
#define _MT_VECTOR_H_

#include <mutex>
#include <atomic>

namespace CGE
{

	template<typename T>
	class MTArrayWrapper
	{
	public:
		MTArrayWrapper(uint64_t size)
			: m_array(nullptr)
			, m_size(size)
		{
			m_counter.store(0);
			m_array = new T[m_size];
		}
		~MTArrayWrapper()
		{
			delete[] m_array;
		}

		uint64_t AssignNext(T item)
		{
			uint64_t index = m_counter.fetch_add(1);
			m_array[index] = item;
			return index;
		}

		void CopyFrom(const MTArrayWrapper& other)
		{
			m_size = other.m_size;
			m_counter = other.m_counter;
			memcpy(m_array, other.m_array, m_counter);
		}
		void CopyTo(MTArrayWrapper& other) const
		{
			other.CopyFrom(*this);
		}

		T* GetArray() { return m_array; }
		uint64_t GetSize() { return m_size; }
		void ResetCounter() { m_counter.store(0); }
		uint64_t GetCounter() { return m_counter.load(); }

		void Set(uint64_t index, const T& item)		{ m_array[index] = item; }
		void Set(uint64_t index, T&& item)			{ m_array[index] = item; }
		const T& Get(uint64_t index) const			{ return m_array[index]; }
		T& Get(uint64_t index)						{ return m_array[index]; }
		const T& operator[] (uint64_t index) const	{ return m_array[index]; }
		T& operator[] (uint64_t index)				{ return m_array[index]; }

		operator T* ()								{ return m_array; }
	private:
		T* m_array;
		uint64_t m_size;
		std::atomic<uint64_t> m_counter;
	};

}

#endif
