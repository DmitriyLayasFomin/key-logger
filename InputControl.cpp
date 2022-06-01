#define _CRT_SECURE_NO_WARNINGS
#include "InputControl.h"
#include <chrono>

void InputControl::init()
{
	this->mouse.init();
	this->calibrationKeyboard();
	MessageBeep(MB_ICONINFORMATION);
}
int InputControl::getIntKeyByVirtual(int value)
{
	for (Key key : this->keyboard)
	{
		if (key.getVirtualKeyValue() == value) {
			return key.getIntValue();
		}
	}
	return 0;
}

void InputControl::calibrationKeyboard() {
	for (int i = 1; i < this->countChars; i++) {
		keybd_event(NULL,
			i,
			KEYEVENTF_EXTENDEDKEY | 0,
			0);
		int virtualKey = MapVirtualKeyA(i, 1);
		Key key = Key(virtualKey, i);
		this->keyboard.push_back(key);
		keybd_event(NULL,
			i,
			KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP,
			0);
	}
}
int InputControl::getVirtualKeyByInt(int value)
{
	for (Key key : this->keyboard)
	{
		if (key.getIntValue() == value) {
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
	if (GetAsyncKeyState(virtualFirst) && GetAsyncKeyState(virtualSecond) && this->getTime() - this->pressedCombination > this->combinationDelay) {

		this->pressedCombination = this->getTime();
		return true;

	}
	return false;
}
int InputControl::getSizeKeyboard() 
{
	return this->keyboard.size();
}
Mouse InputControl::getMouse()
{
	return this->mouse;
}
int InputControl::getTime()
{
	using namespace std::chrono;
	return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}