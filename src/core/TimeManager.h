#pragma once

class TimeManager
{
public:
	static TimeManager* GetInstance();

	void UpdateTime();
	float GetDeltaTime();
protected:
	static TimeManager* StaticInstance;

	float DeltaTime;
	double LastTimestamp;
private:
	TimeManager();
	virtual ~TimeManager();
};
