#pragma once

#include <chrono>

class TimeManager
{
public:
	static TimeManager* GetInstance();

	void UpdateTime();
	float GetDeltaTime();
protected:
	static TimeManager* staticInstance;

	float deltaTime;
	std::chrono::time_point<std::chrono::steady_clock> lastTimestamp;
private:
	TimeManager();
	TimeManager(const TimeManager&) {}
	void operator=(const TimeManager&) {}
	virtual ~TimeManager();
};
