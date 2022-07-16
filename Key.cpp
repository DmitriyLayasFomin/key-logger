#include "Key.h"

Key::Key(int virtualKey, int intValue)
{
	this->intValue = intValue;
	this->virtualKey = virtualKey;
}
void Key::press()
{
	if (intValue > 0) {
		keybd_event(virtualKey,
			intValue,
			KEYEVENTF_EXTENDEDKEY | 0,
			0);
	}
	else {
		if (virtualKey == VK_LBUTTON) {
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
		}
		else if (virtualKey == VK_RBUTTON) {
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
		}
	}

};
void Key::release()
{
	if (intValue > 0) {
		keybd_event(virtualKey,
			intValue,
			KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP,
			0);
	}
	else {
		if (virtualKey == VK_LBUTTON) {
			mouse_event(MOUSEEVENTF_LEFTUP, NULL, NULL, 0, 0);
		}
		else if (virtualKey == VK_RBUTTON) {
			mouse_event(MOUSEEVENTF_RIGHTUP, NULL, NULL, 0, 0);
		}
	}
};
void Key::pressAndRelease(int delay = 500)
{
	press();
	Sleep(delay);
	release();
};
int Key::getHardValue()
{
	return intValue;
};
int Key::getVirtualKeyValue()
{
	return virtualKey;
};
void Key::setStatus(int value)
{
	status = static_cast<Status>(value);
}
Status Key::getStatus()
{
	return status;
}