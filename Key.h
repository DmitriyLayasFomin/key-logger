#pragma once
#include <Windows.h>
#include <Winuser.h>

enum Status { DOWN = -32767, PRESSED = -32768, UP = 0 };

class Key
{
public:
	Key(int virtualKey, int intValue);
	void press();
	void release();
	void pressAndRelease(int);
	int getHardValue();
	int getVirtualKeyValue();
	void setStatus(int value);
	Status getStatus();
private:
	int virtualKey = 0;
	int intValue = 0;
	Status status;
};

