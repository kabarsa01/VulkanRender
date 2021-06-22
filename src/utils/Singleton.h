#ifndef __SINGLETON_H__
#define __SINGLETON_H__

namespace CGE
{

	//------------------------------------------------------------------
	//------------------------------------------------------------------
	//------------------------------------------------------------------

	template<typename T>
	class Singleton
	{
	public:
		static T* GetInstance();
		static void DestroyInstance();
	protected:
		static T* m_instance;
		static std::mutex m_mutex;

		Singleton() {}
		~Singleton() {}
	};

	//------------------------------------------------------------------
	//------------------------------------------------------------------
	//------------------------------------------------------------------
	
	template<typename T>
	T* Singleton<T>::m_instance = nullptr;
	template<typename T>
	std::mutex Singleton<T>::m_mutex;

	template<typename T>
	T* Singleton<T>::GetInstance()
	{
		if (m_instance == nullptr)
		{
			std::scoped_lock<std::mutex> lock(m_mutex);
			if (m_instance == nullptr)
			{
				m_instance = new T();
			}
		}
		return m_instance;
	}

	//------------------------------------------------------------------

	template<typename T>
	void Singleton<T>::DestroyInstance()
	{
		if (m_instance)
		{
			std::scoped_lock<std::mutex> lock(m_mutex);
			if (m_instance)
			{
				delete m_instance;
				m_instance = nullptr;
			}
		}
	}

	//------------------------------------------------------------------
	//------------------------------------------------------------------
	//------------------------------------------------------------------

}

#endif
