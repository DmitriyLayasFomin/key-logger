#pragma once
#include <Windows.h>
#include <Winuser.h>
class Mouse
{
public:
	Mouse();
	void init();
	void lClick(int, int, int);
	void rClick(int, int, int);
	int getLButton();
	int getRButton();
	int getPositionX();
	int getPositionY();
	Mouse getLastState();
	int isLeftPressed();
	int isRightPressed();
	void setPositionX(int);
	void setPositionY(int);
private:
	void calibration();
	void release(int, int, int);
	void press(int, int, int);
	int lButton;
	int rButton;
	int positionX;
	int positionY;
	const int countChars = 128;
};

