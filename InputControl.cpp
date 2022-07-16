#define _CRT_SECURE_NO_WARNINGS
#include "InputControl.h"
#include <chrono>
#include <map>
#include <mutex>
using namespace std;

void InputControl::init()
{
	mouse.init();
	calibrationKeyboard();
}
int InputControl::getIntKeyByVirtual(int value)
{
	for (Key key : keyboard)
	{
		if (key.getVirtualKeyValue() == value) {
			return key.getHardValue();
		}
	}
	return 0;
}

void InputControl::calibrationKeyboard() {
	for (int i = 1; i < countChars; i++) {
		char virtualKey = MapVirtualKeyA(i, MAPVK_VSC_TO_VK);
		Key key = Key(virtualKey, i);
		keyboard.push_back(key);
	}
	keyboard.push_back(Key(VK_LBUTTON, 0));
	keyboard.push_back(Key(VK_RBUTTON, 0));

	
}
bool InputControl::releaseEvent(int virtualCode)
{
	std::mutex mx;
	mx.lock();
	while (GetAsyncKeyState(virtualCode) != Status::UP) {}
	mx.unlock();
	return true;
}
void InputControl::startListen()
{
	updateKeyboardStatus = thread([&, this]() mutable {
		std::mutex mx;
		while (!stop) {
			for (Key& key : keyboard)
			{
				int status = GetAsyncKeyState(key.getVirtualKeyValue());
				key.setStatus(status);
			}
		}
	});
	updateKeyboardStatus.detach();
}
void InputControl::stoptListen()
{
	stop = true;
	updateKeyboardStatus.~thread();
}
int InputControl::getVirtualKeyByHardValue(int value)
{
	for (Key key : keyboard)
	{
		if (key.getHardValue() == value) {
			return key.getVirtualKeyValue();
		}
	}
	return 0;
}
int InputControl::getLastVirtualKey(Status status)
{
	for (Key key : keyboard)
	{
		if (key.getStatus() == status) {
			return key.getVirtualKeyValue();
		}
	}
	return 0;
}
bool InputControl::isPressedCombination(int virtualFirst, int virtualSecond)
{
	if (getKeyByVirtualValue(virtualFirst).getStatus() == Status::PRESSED) {
		if (getKeyByVirtualValue(virtualSecond).getStatus() == Status::DOWN) {
			return true;
		}
	}
	
	return false;
}
int InputControl::getSizeKeyboard()
{
	return keyboard.size();
}
Key InputControl::getKeyByHardValue(int value)
{
	for (Key key : keyboard)
	{
		if (key.getHardValue() == value) {
			return key;
		}
	}
}
Key InputControl::getKeyByVirtualValue(int value)
{
	for (Key key : keyboard)
	{
		if (key.getVirtualKeyValue() == value) {
			return key;
		}
	}
	return Key(0, 0);
}
Mouse InputControl::getMouse()
{
	return mouse;
}
unsigned long int InputControl::getTime()
{
	return static_cast<unsigned long int>(time(0));
}