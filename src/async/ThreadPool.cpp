#include <condition_variable>

#include "async/ThreadPool.h"
#include "Job.h"

namespace CGE
{

	ThreadPool* ThreadPool::m_instance = nullptr;

	void ThreadPool::InitInstance(uint32_t poolSize)
	{
		if (!m_instance)
		{
			m_instance = new ThreadPool(poolSize);
		}
	}

	void ThreadPool::DestroyInstance()
	{
		if (m_instance)
		{
			delete m_instance;
			m_instance = nullptr;
		}
	}

	ThreadPool* ThreadPool::GetInstance()
	{
		return m_instance;
	}

	ThreadPool::ThreadPool(uint32_t poolSize)
		: m_poolSize(poolSize)
		, m_shouldExit(false)
	{
		for (uint32_t idx = 0; idx < m_poolSize; idx++)
		{
			m_threads.emplace_back(std::thread(&ThreadPool::PoolThreadBody, this));
		}
	}

	ThreadPool::~ThreadPool()
	{
		m_shouldExit = true;
		m_condition.notify_one();
		for (std::thread& thread : m_threads)
		{
			thread.join();
		}
	}

	void ThreadPool::PoolThreadBody()
	{
		while (true)
		{
			std::shared_ptr<IJob> job;
			{
				std::unique_lock<std::mutex> lock(m_mutex);
				if (m_jobs.size() == 0)
				{
					m_condition.wait(lock, [this]() -> bool { return (m_jobs.size() > 0) || m_shouldExit; });
				}

				if (m_shouldExit)
				{
					std::notify_all_at_thread_exit(m_condition, std::move(lock));
					return;
				}

				job = m_jobs.front();
				m_jobs.pop_front();
			}
			job->Execute();
		}
	}

}

