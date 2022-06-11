#include "Mouse.h"
Mouse::Mouse() {}
void Mouse::init()
{
	POINT point;
	GetCursorPos(&point);
	this->positionX = point.x;
	this->positionY = point.y;
	this->calibration();
}
void Mouse::lClick(int x = 0, int y = 0, int delay = 100)
{
	this->press(this->lButton,x,y);
	Sleep(delay);
	this->release(this->lButton, x, y);
};
void Mouse::rClick(int x = 0, int y = 0, int delay = 100)
{
	this->press(this->rButton, x, y);
	Sleep(delay);
	this->release(this->rButton, x, y);
};
void Mouse::press(int button, int x, int y)
{
	if (x != 0 || y != 0) {
		if (button == this->lButton) {
			mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTDOWN, x, y, 0, 0);
		}
		else if (button == this->rButton) {
			mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_RIGHTDOWN, NULL, NULL, 0, 0);
		}
	}
	else {
		if (button == this->lButton) {
			mouse_event(MOUSEEVENTF_LEFTDOWN, NULL, NULL, 0, 0);
		}
		else if (button == this->rButton) {
			mouse_event(MOUSEEVENTF_RIGHTDOWN, NULL, NULL, 0, 0);
		}
	}
};
void Mouse::calibration()
{
	mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTDOWN, NULL, NULL, 0, 0);
	for (int i = 1; i < this->countChars; i++) {
		if (GetAsyncKeyState(i) < 0) {
			this->lButton = i;
			break;
		}
	}
	mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTUP, NULL, NULL, 0, 0);

	mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_RIGHTDOWN, NULL, NULL, 0, 0);
	for (int i = 1; i < this->countChars; i++) {
		if (GetAsyncKeyState(i)  < 0) {
			this->rButton = i;
			break;
		}
	}
	mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_RIGHTUP, NULL, NULL, 0, 0);

}
void Mouse::release(int button, int x, int y)
{
	if (x != 0 || y != 0) {
		if (button == this->lButton) {
			mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTUP, x, y, 0, 0);
		}
		else if (button == this->rButton) {
			mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_RIGHTUP, NULL, NULL, 0, 0);
		}
	}
	else {
		if (button == this->lButton) {
			mouse_event(MOUSEEVENTF_LEFTUP, NULL, NULL, 0, 0);
		}
		else if (button == this->rButton) {
			mouse_event(MOUSEEVENTF_RIGHTUP, NULL, NULL, 0, 0);
		}
	}
};
int Mouse::getLButton()
{
	return this->lButton;
};
int Mouse::getRButton()
{
	return this->rButton;
};
void Mouse::setPositionX(int positionX)
{
	this->positionX = positionX;
};
void Mouse::setPositionY(int positionY)
{
	this->positionY = positionY;
};
int Mouse::getPositionX()
{
	return this->positionX;
};
int Mouse::getPositionY()
{
	return this->positionY;
};
Mouse Mouse::getLastState()
{
	POINT p;
	if (GetCursorPos(&p))
	{
		this->positionX = p.x;
		this->positionY = p.y;
	}
	return *this;
}
int Mouse::isLeftPressed()
{
	if (GetAsyncKeyState(this->lButton) < 0) {
		return 1;
	}
	return 0;
}
int Mouse::isRightPressed()
{
	if (GetAsyncKeyState(this->rButton) < 0) {
		return 1;
	}
	return 0;
}