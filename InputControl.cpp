#define _CRT_SECURE_NO_WARNINGS
#include "InputControl.h"
#include <chrono>

void InputControl::init()
{
	this->mouse.init();
	this->calibrationKeyboard();
}
int InputControl::getIntKeyByVirtual(int value)
{
	for (Key key : this->keyboard)
	{
		if (key.getVirtualKeyValue() == value) {
			return key.getHardValue();
		}
	}
	return 0;
}

void InputControl::calibrationKeyboard() {
	for (int i = 1; i < this->countChars; i++) {
		char virtualKey = MapVirtualKeyA(i, MAPVK_VSC_TO_VK);
		Key key = Key(virtualKey, i);
		key.release();
		this->keyboard.push_back(key);
	}
	this->keyboard.push_back(Key(VK_LBUTTON, 0));
	this->keyboard.push_back(Key(VK_RBUTTON, 0));
}
int InputControl::getVirtualKeyByHardValue(int value)
{
	for (Key key : this->keyboard)
	{
		if (key.getHardValue() == value) {
			return key.getVirtualKeyValue();
		}
	}
	return 0;
}
int InputControl::getLastPressedVirtualKey()
{
	for (Key key : this->keyboard)
	{
		if (GetAsyncKeyState(key.getVirtualKeyValue())) {
			return key.getVirtualKeyValue();
		}
	}
	return 0;
}
bool InputControl::isPressedCombination(int virtualFirst, int virtualSecond)
{
	BYTE keys[256] = {};
	if (!GetKeyboardState(keys)) {
		DWORD word = GetLastError();
	};
	bool a = keys[virtualFirst] > 0;
	bool b = keys[virtualSecond] > 0;
	if (keys[virtualFirst] > 0 && keys[virtualSecond] > 0) {
		return true;
	}
	return false;
}
int InputControl::getSizeKeyboard() 
{
	return this->keyboard.size();
}
Key InputControl::getKeyByHardValue(int value)
{
	for (Key key : this->keyboard)
	{
		if (key.getHardValue() == value) {
			return key;
		}
	}
}
Key InputControl::getKeyByVirtualValue(int value)
{
	for (Key key : this->keyboard)
	{
		if (key.getVirtualKeyValue() == value) {
			return key;
		}
	}
	return Key(0, 0);
}
Mouse InputControl::getMouse()
{
	return this->mouse;
}
UINT InputControl::getTime()
{
	return static_cast<UINT>(time(0));
}