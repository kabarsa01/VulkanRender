#ifndef _JOB_H_

#include <functional>

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
}

#endif