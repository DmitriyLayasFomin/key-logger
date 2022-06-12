#include "Key.h"
Key::Key(int virtualKey, int intValue)
{
	this->intValue = intValue;
	this->virtualKey = virtualKey;
}
void Key::press()
{
	if (this->intValue > 0) {
		keybd_event(NULL,
			this->intValue,
			KEYEVENTF_EXTENDEDKEY | 0,
			0);
	}
	else {
		if (this->virtualKey == VK_LBUTTON) {
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
		}
		else if (this->virtualKey == VK_RBUTTON) {
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
		}
	}

};
void Key::release()
{
	if (this->intValue > 0) {
		keybd_event(NULL,
			this->virtualKey,
			KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP,
			0);
	}
	else {
		if (this->virtualKey == VK_LBUTTON) {
			mouse_event(MOUSEEVENTF_LEFTUP, NULL, NULL, 0, 0);
		}
		else if (this->virtualKey == VK_RBUTTON) {
			mouse_event(MOUSEEVENTF_RIGHTUP, NULL, NULL, 0, 0);
		}
	}
};
void Key::pressAndRelease(int delay = 500)
{
	this->press();
	Sleep(delay);
	this->release();
};
int Key::getHardValue()
{
	return this->intValue;
};
int Key::getVirtualKeyValue()
{
	return this->virtualKey;
};