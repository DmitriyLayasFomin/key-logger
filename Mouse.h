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
	void setPositionX(int x);
	void setPositionY(int y);
	void setMouseCursor(int x, int y);
private:
	int positionX;
	int positionY;
};

