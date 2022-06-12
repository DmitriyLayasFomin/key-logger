#pragma once
#include <Windows.h>
#include <Winuser.h>
class Mouse
{
public:
	Mouse();
	void init();
	int getPositionX();
	int getPositionY();
	Mouse getLastState();
	void setPositionX(int);
	void setPositionY(int);
private:
	void setMousePointer(int, int);
	int positionX;
	int positionY;
};

