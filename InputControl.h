#pragma once
#include <iostream>
#include <vector>
#include "Key.h"
#include "Mouse.h"
#include <thread>
using namespace std;
class InputControl
{
public:
	const int countChars = 256;
	const unsigned long int combinationDelay = 500;

	void init();
	void startListen();
	void stoptListen();
	bool isPressedCombination(int first, int second);
	int getSizeKeyboard();
	Key getKeyByHardValue(int value);
	Key getKeyByVirtualValue(int value);
	int getLastVirtualKey(Status status);
	bool releaseEvent(int virtualCode);
	Mouse getMouse();
private:
	static unsigned long int getTime();
	void calibrationKeyboard();
	int getVirtualKeyByHardValue(int);
	int getIntKeyByVirtual(int value);
	vector <Key> keyboard;
	Mouse mouse;
	unsigned long int pressedCombination;
	thread updateKeyboardStatus;
	bool stop = false;
};

