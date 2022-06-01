#pragma once
#include <iostream>
#include <vector>
#include "Key.h"
#include "Mouse.h"
using namespace std;
class InputControl
{
public:
	void init();
	bool isPressedCombination(int first, int second);
	int getSizeKeyboard();
	Mouse getMouse();
	int getTime();
	int getLastPressedVirtualKey();
private:
	void calibrationKeyboard();
	int getVirtualKeyByInt(int);
	int getIntKeyByVirtual(int value);
	vector <Key> keyboard;
	Mouse mouse;
	int pressedCombination;
	const int countChars = 128;
	const int combinationDelay = 1000;
};

