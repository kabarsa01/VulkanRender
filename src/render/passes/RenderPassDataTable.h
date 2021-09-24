#ifndef __RENDER_PASS_RESOURCE_TABLE_H__
#define __RENDER_PASS_RESOURCE_TABLE_H__

#include <unordered_map>

#include "utils/Identifiable.h"

namespace CGE
{

	class RenderPassDataTable
	{
	public:
		RenderPassDataTable();
		~RenderPassDataTable() {}

		template<typename T>
		void AddPassData(std::shared_ptr<Identifiable<T>> data);
		template<typename T>
		std::shared_ptr<T> GetPassData();
	private:
		std::unordered_map<IIdentifiable::Id, std::shared_ptr<IIdentifiable>> m_data;
	};

	template<typename T>
	void RenderPassDataTable::AddPassData(std::shared_ptr<Identifiable<T>> data)
	{
		m_data[data->GetId()] = data;
	}

	template<typename T>
	std::shared_ptr<T> RenderPassDataTable::GetPassData()
	{
		static_assert(std::is_base_of<Identifiable<T>, T>().value, "!!! Identifiable expected !!! Please use something derived from Identifiable<T>");
		return ObjectBase::template Cast<T>(m_data[T::Id()]);
	}

}

#endif
 