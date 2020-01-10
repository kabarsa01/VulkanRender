#include "TimeManager.h"

#include <GLFW/glfw3.h>

TimeManager* TimeManager::StaticInstance = new TimeManager();

TimeManager * TimeManager::GetInstance()
{
	return StaticInstance;
}

void TimeManager::UpdateTime()
{
	double CurrentTime = glfwGetTime();
	DeltaTime = CurrentTime - LastTimestamp;
	LastTimestamp = CurrentTime;
}

float TimeManager::GetDeltaTime()
{
	return DeltaTime;
}

TimeManager::TimeManager()
{
}

TimeManager::~TimeManager()
{
}
