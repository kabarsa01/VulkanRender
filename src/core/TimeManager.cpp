#include "TimeManager.h"

TimeManager* TimeManager::staticInstance = new TimeManager();

TimeManager * TimeManager::GetInstance()
{
	return staticInstance;
}

void TimeManager::UpdateTime()
{
	auto currentTime = std::chrono::high_resolution_clock::now();

	deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTimestamp).count();
	lastTimestamp = currentTime;
}

float TimeManager::GetDeltaTime()
{
	return deltaTime;
}

TimeManager::TimeManager()
{
	lastTimestamp = std::chrono::high_resolution_clock::now();
}

TimeManager::~TimeManager()
{
}
