#include "Identifiable.h"

std::atomic<uint64_t> CGE::IIdentifiable::m_idCounter;
std::mutex CGE::IIdentifiable::m_mutex;

