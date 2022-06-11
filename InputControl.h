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
	Key getKeyByHardValue(int value);
	Key getKeyByVirtualValue(int value);
	Mouse getMouse();
	int getTime();
	int getLastPressedVirtualKey();
private:
	void calibrationKeyboard();
	int getVirtualKeyByHardValue(int);
	int getIntKeyByVirtual(int value);
	vector <Key> keyboard;
	Mouse mouse;
	int pressedCombination;
	const int countChars = 256;
	const int combinationDelay = 1000;
};

