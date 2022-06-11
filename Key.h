#pragma once
#include <Windows.h>
#include <Winuser.h>
class Key
{
public:
	Key(int, int);
	void press();
	void release();
	void pressAndRelease(int);
	int getHardValue();
	int getVirtualKeyValue();
private:
	char virtualKey;
	int intValue;
};

