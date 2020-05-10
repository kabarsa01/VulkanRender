#include "TimeManager.h"

TimeManager* TimeManager::staticInstance = new TimeManager();

TimeManager * TimeManager::GetInstance()
{
	return staticInstance;
}

void TimeManager::UpdateTime()
{
	auto currentTime = std::chrono::high_resolution_clock::now();

	time = std::chrono::duration<double, std::chrono::seconds::period>(currentTime - appStartTime).count();
	deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTimestamp).count();
	lastTimestamp = currentTime;
}

float TimeManager::GetDeltaTime()
{
	return deltaTime;
}

double TimeManager::GetTime()
{
	return time;
}

TimeManager::TimeManager()
{
	appStartTime = std::chrono::high_resolution_clock::now();
	lastTimestamp = appStartTime;
}

TimeManager::~TimeManager()
{
}
