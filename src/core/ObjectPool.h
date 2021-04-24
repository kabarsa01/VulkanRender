#ifndef _OBJECT_POOL_H_
#define _OBJECT_POOL_H_

#include <set>
#include <mutex>

namespace CGE
{

	//============================================================================================================
	// Simple object pool for different needs
	//============================================================================================================

	template<typename T>
	class ObjectPool
	{
	public:
		ObjectPool(uint32_t poolSize);
		~ObjectPool();

		T* Get(uint32_t offset) { return m_objects + offset; }
		uint32_t GetIndex(T* base) { return base - m_objects; }

		T* Acquire(uint32_t amount);
		void Release(T* objects, uint32_t amount);
	private:
		struct PoolBlockRecord
		{
			uint32_t position;
			mutable uint32_t size;

			PoolBlockRecord(uint32_t pos, uint32_t initSize = 0) : position(pos), size(initSize) {}
			bool operator<(const PoolBlockRecord& other) const { return position < other.position; }
		};

		using BlockIter = typename std::set<PoolBlockRecord>::iterator;

		uint32_t m_poolSize;
		T* m_objects;
		std::set<PoolBlockRecord> m_freeBlocks;
		std::mutex m_mutex;

		BlockIter MergeBlocks(BlockIter& first, BlockIter& second);
	};

	//============================================================================================================
	//============================================================================================================

	template<typename T>
	ObjectPool<T>::ObjectPool(uint32_t poolSize)
		: m_poolSize(poolSize)
	{
		m_objects = new T[m_poolSize];
		m_freeBlocks.emplace(0, m_poolSize);
	}

	//------------------------------------------------------------------------------------------------------------

	template<typename T>
	ObjectPool<T>::~ObjectPool()
	{
		delete[] m_objects;
	}

	//------------------------------------------------------------------------------------------------------------

	template<typename T>
	T* ObjectPool<T>::Acquire(uint32_t amount)
	{
		std::scoped_lock lock(m_mutex);

		assert(amount <= m_poolSize);

		uint32_t pos = UINT32_MAX;
		for (PoolBlockRecord rec : m_freeBlocks)
		{
			if (rec.size < amount)
			{
				continue;
			}

			pos = rec.position;
			rec.position += amount;
			rec.size -= amount;

			if (rec.size > 0)
			{
				m_freeBlocks.insert(rec);
			}

			break;
		}
		if (pos != UINT32_MAX)
		{
			m_freeBlocks.erase(pos);
			return m_objects + pos;
		}

		return nullptr;
	}

	//------------------------------------------------------------------------------------------------------------

	template<typename T>
	void ObjectPool<T>::Release(T* objects, uint32_t amount)
	{
		std::scoped_lock lock(m_mutex);

		assert(objects);
		uint32_t pos = static_cast<uint32_t>(objects - m_objects);
		assert(pos < m_poolSize);

		auto iter = m_freeBlocks.emplace(pos, amount).first;
		if (iter != m_freeBlocks.begin())
		{
			auto prevIter = iter;
			iter = MergeBlocks(--prevIter, iter);
		}
		auto nextIter = iter;
		++nextIter;
		if (nextIter != m_freeBlocks.end())
		{
			MergeBlocks(iter, nextIter);
		}
	}

	//------------------------------------------------------------------------------------------------------------

	template<typename T>
	typename ObjectPool<T>::BlockIter ObjectPool<T>::MergeBlocks(BlockIter& first, BlockIter& second)
	{
		if ((first->position + first->size) == second->position)
		{
			uint32_t newSize = first->size + second->size;
			m_freeBlocks.erase(second);
			first->size = newSize;
			return first;
		}
		return second;
	}

	//============================================================================================================
	//============================================================================================================

}

#endif
