#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

#include <thread>
#include <mutex>
#include <deque>
#include <vector>

namespace CGE
{
	class IJob;

	class ThreadPool
	{
	public:
		static void InitInstance(uint32_t poolSize);
		static void DestroyInstance();
		static ThreadPool* GetInstance();

		void AddJob(std::shared_ptr<IJob> job)
		{
			std::scoped_lock lock(m_mutex);
			m_jobs.push_back(job);
			m_condition.notify_one();
		}
	private:
		static ThreadPool* m_instance;
	
		bool m_shouldExit = false;
		uint32_t m_poolSize;
		std::vector<std::thread> m_threads;
		std::deque<std::shared_ptr<IJob>> m_jobs;
		//synchronization
		std::mutex m_mutex;
		std::condition_variable m_condition;
	
		ThreadPool(uint32_t poolSize);
		ThreadPool(const ThreadPool&) = delete;
		ThreadPool(ThreadPool&&) = delete;
		ThreadPool& operator =(const ThreadPool&) = delete;
		ThreadPool& operator =(ThreadPool&&) = delete;
		~ThreadPool();

		void PoolThreadBody();
	};
}

#endif // THREAD_POOL_H_
