#include "Mouse.h"
Mouse::Mouse() {}
void Mouse::init()
{
	POINT point;
	GetCursorPos(&point);
	this->positionX = point.x;
	this->positionY = point.y;
}
void Mouse::setMouseCursor(int x, int y)
{
	mouse_event(MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE, x, y, 0, 0);
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