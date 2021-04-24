#ifndef _JOB_H_
#define _JOB_H_

#include <functional>
#include <type_traits>

namespace CGE
{
	class IJob
	{
	public:
		virtual ~IJob() {}
		virtual void Execute() = 0;
	};

	template<typename T>
	class Job : public IJob
	{
	public:
		Job(std::function<T>&& function)
			: m_function(function)
		{
		}
		void Execute() override
		{
			m_function();
		}
	private:
		std::function<T> m_function;
	};

	template<typename T>
	std::shared_ptr<Job<T>> CreateJobPtr(std::function<T>&& func)
	{
		return std::make_shared<Job<T>>(std::forward<std::function<T>>(func));

	}

}

#endif