#include "Program.h"


Program::Program()
{
	this->getInputControl().init();
	this->keyboarFields = {
		{"KEY_VALUE", SQLiteField("KEY_VALUE","INT")},
		{"TIME", SQLiteField("TIME","INT")},
		{"RELEASED", SQLiteField("RELEASED","INT")}
	};
	this->mouseFields = {
		{"POSITION_X", SQLiteField("POSITION_X","INT")},
		{"POSITION_Y", SQLiteField("POSITION_Y","INT")},
		{"TIME", SQLiteField("TIME","INT")},
	};
	SQLiteORM sqlORM;
	sqlORM.setTable("KEYBOARD")->open()->createTable(getValues(this->keyboarFields))->close();
	sqlORM.setTable("MOUSE")->open()->createTable(getValues(this->mouseFields))->close();
	for (int i = 0; i < this->getInputControl().countChars; i++) {
		this->keyReleaseWait.insert({ i, false });
	}
	
}
void Program::start()
{
	Sleep(2000);
	BYTE a[256] = {};
	GetKeyboardState(a);
	thread mainBackground = thread([=]() mutable {
		while (!this->getInputControl().isPressedCombination(VK_CONTROL, 0x37)) {
			int b = GetAsyncKeyState(VK_CONTROL);
			int c = GetAsyncKeyState(0x32);
			int a = this->getInputControl().getLastPressedVirtualKey();
			if (this->getInputControl().isPressedCombination(VK_CONTROL, 0x32)) {
				this->mx.lock();
				cout << "Kyelogger start." << endl;
				MessageBeep(MB_ICONINFORMATION);
				this->mx.unlock();
				this->recordInputControl(VK_CONTROL, 0x32);
				this->mx.lock();
				cout << "Kyelogger stop." << endl;
				MessageBeep(MB_ICONERROR);
				this->mx.unlock();
			}
			if (this->getInputControl().isPressedCombination(VK_CONTROL, 0x33)) {
				this->mx.lock();
				cout << "logged repeat start." << endl;
				MessageBeep(MB_ICONINFORMATION);
				this->mx.unlock();
				this->loggedRun(VK_CONTROL, 0x33);
				this->mx.lock();
				cout << "logged repeat stop." << endl;
				MessageBeep(MB_ICONERROR);
				Sleep(1000);
				this->mx.unlock();
			}
			if (this->getInputControl().isPressedCombination(VK_CONTROL, 0x37)) {
				for (int i = 0; i < this->queueWriter.size(); i++) {
					this->queueWriter.front().wait();
				}
				for (int i = 0; i < this->queueReader.size(); i++) {
					this->queueReader.front().wait();
				}
				this->mx.lock();
				this->exit = true;
				cout << "shutdown." << endl;
				this->mx.unlock();
				break;
			}
		}
	});
	if(!mainBackground.joinable())
		mainBackground.join();

	while (mainBackground.joinable())
	{

	}
}
InputControl& Program::getInputControl()
{
	return this->inputControl;
}
vector<SQLiteField> Program::getValues(map<string, SQLiteField> map)
{
	vector<SQLiteField> result;
	for (auto i = map.begin(); i != map.end(); i++) {
		result.push_back(i->second);
	}
	return result;
}
void Program::writeKeyInVector(vector <map<string, SQLiteField>>* vec, map<string, SQLiteField> keyboarFields, string keyIntValue, string time)
{
	{
		std::lock_guard<std::mutex> guard(this->getMutex());
		keyboarFields.find("KEY_VALUE")->second.setValue(keyIntValue);
		keyboarFields.find("TIME")->second.setValue(time);
		keyboarFields.find("RELEASED")->second.setValue(string("0"));
		vec->push_back(keyboarFields);
	}
	if (!this->getKeyReleaseWait(atoi(keyIntValue.c_str()))) {
		this->queueWriter.push(std::async(launch::async, [&](vector <map<string, SQLiteField>>* vec, map<string, SQLiteField> keyboarFieldsCopy) {
			string key = keyboarFieldsCopy.find("KEY_VALUE")->second.getValue();
			this->setKeyReleaseWait(atoi(key.c_str()), true);
			keyboarFieldsCopy.find("RELEASED")->second.setValue("1");
			cout << "queueWriter" << endl;
			while (!this->exit && !this->getInputControl().isPressedCombination(VK_CONTROL, 0x32)) {
				if (!(GetAsyncKeyState(atoi(key.c_str())) < 0)) {
					{
						std::lock_guard<std::mutex> guard(this->getMutex());
						UINT time = getTime();
						keyboarFieldsCopy.find("TIME")->second.setValue(to_string(time));
						vec->push_back(keyboarFieldsCopy);
						this->setKeyReleaseWait(atoi(key.c_str()), false);
					}
					return;
				}
			}
		}, vec, keyboarFields));
	}
	
}
void Program::writeKey(vector <map<string, SQLiteField>> vec)
{
	for (map<string, SQLiteField> keyboarFields : vec) {
		this->queueWriter.push(std::async(launch::async, [&](map<string, SQLiteField> keyboarFields) {
			SQLiteORM sqlORM;
			sqlORM.setTable("KEYBOARD")->open()->insert(this->getValues(keyboarFields))->close();
		}, keyboarFields));
	}
}
void Program::writeMouse(vector <map<string, SQLiteField>> vec)
{
	for (map<string, SQLiteField> mouseFields : vec) {
		this->queueWriter.push(std::async(launch::async, [=](map<string, SQLiteField> mouseFields) {
			SQLiteORM sqlORM;
			sqlORM.setTable("MOUSE")->open()->insert(this->getValues(mouseFields))->close();
		}, mouseFields));
	}
}
void Program::writeMouseInVector(vector <map<string, SQLiteField>>* vec, map<string, SQLiteField> mouseFields, string positionX, string positionY, string time)
{
	mouseFields.find("POSITION_X")->second.setValue(positionX);
	mouseFields.find("POSITION_Y")->second.setValue(positionY);
	mouseFields.find("TIME")->second.setValue(time);
	vec->push_back(mouseFields);
}
void Program::recordInputControl(int virtualFirst, int virtualSecond)
{
	Sleep(1000);
	bool* stop = new bool(false);
	int saveFile[] = { 0, 0 };
	vector <map<string, SQLiteField>> keyboardVec;
	vector <map<string, SQLiteField>> mouseVec;
	thread th = thread([=]() {
		while (!this->inputControl.isPressedCombination(virtualFirst, virtualSecond) && !*stop) {}
		*stop = true;
		for (int i = 0; i < this->queueWriter.size(); i++) {
			this->queueWriter.pop();
		}
	});
	thread thkbrd = thread([=, &keyboardVec]() mutable {
		while (!*stop) {
			if (this->logIntervalKey > 0) {
				Sleep(this->logIntervalKey);
			}
			int lastVirtual = this->inputControl.getLastPressedVirtualKey();
			UINT time = getTime();
			if (!keyboardVec.empty()) {
				int lastInVector = atoi(keyboardVec.back().find("KEY_VALUE")->second.getValue().c_str());
				if (lastVirtual == lastInVector) {
					if (time - atoi(keyboardVec.back().find("TIME")->second.getValue().c_str()) < 200) {
						continue;
					}
				}
			}
			if (lastVirtual != 0) {
				this->writeKeyInVector(&keyboardVec, this->keyboarFields, std::to_string(lastVirtual), std::to_string(time));
			}
		}
	});
	thread thmouse = thread([=, &mouseVec]() mutable {
		while (!*stop) {
			if (this->logIntervalCursor > 0) {
				Sleep(this->logIntervalCursor);
			}
			Mouse mouse = this->inputControl.getMouse().getLastState();
			UINT time = getTime();
			this->writeMouseInVector(
				&mouseVec,
				this->mouseFields,
				std::to_string(mouse.getPositionX()),
				std::to_string(mouse.getPositionY()),
				std::to_string(time)
			);
		}
	});

	th.detach();
	if(!thkbrd.joinable())
	thkbrd.join();
	if (!thmouse.joinable())
	thmouse.join();

	while (!*stop && !this->getExit()) {}
	thkbrd.detach();
	thmouse.detach();
	thkbrd.~thread();
	thmouse.~thread();

	cout << "Saving log..." << endl;

	this->filterLog(keyboardVec);
	this->writeKey(keyboardVec);
	this->writeMouse(mouseVec);
	for (int i = 0; i < this->queueWriter.size(); i++) {
		this->queueWriter.front().wait();
		this->queueWriter.pop();
	}
	cout << "Log has been saved." << endl;

}
void Program::filterLog(vector <map<string, SQLiteField>>& keyboardVec) {
	vector <map<string, SQLiteField>> keyboardVecCopy;
	for (map<string, SQLiteField> keyboard : keyboardVec) {
		map<string, SQLiteField>* pressedKye = new map<string, SQLiteField>;
		map<string, SQLiteField>* releasedKye = new map<string, SQLiteField>;
		for (map<string, SQLiteField>& keyboardCopy : keyboardVecCopy) {

			if (keyboard.find("KEY_VALUE")->second.getValue() == keyboardCopy.find("KEY_VALUE")->second.getValue()) {
				if (keyboard.find("RELEASED")->second.getValue() == keyboardCopy.find("RELEASED")->second.getValue()) {
					if (keyboardCopy.find("RELEASED")->second.getValue() == "0") {
						pressedKye = &keyboardCopy;
					}
					if (keyboardCopy.find("RELEASED")->second.getValue() == "1") {
						releasedKye = &keyboardCopy;
					}

				}
			}
		}
		if (!releasedKye->empty()) {
			if (pressedKye->empty())
				if (keyboard.find("RELEASED")->second.getValue() == "1" && releasedKye->find("RELEASED")->second.getValue() == "1") {
					continue;
				}
		}
		if (!pressedKye->empty()) {
			if (releasedKye->empty())
				if (keyboard.find("RELEASED")->second.getValue() == "0" && pressedKye->find("RELEASED")->second.getValue() == "0") {
					continue;
				}
		}

		keyboardVecCopy.push_back(keyboard);
	}
	keyboardVec.clear();
	map<string, SQLiteField> keyboardLast = keyboardVecCopy.front();
	keyboardVecCopy.begin()++;
	int i = 1;
	keyboardVec.push_back(keyboardLast);
	for (map<string, SQLiteField> keyboardCopy : keyboardVecCopy) {
		if (atoi(keyboardCopy.find("TIME")->second.getValue().c_str()) - atoi(keyboardLast.find("TIME")->second.getValue().c_str()) > 200) {
			keyboardVec.push_back(keyboardCopy);
		}
		else if (i == 1) {
			keyboardVec.clear();
		}
		keyboardLast = keyboardCopy;
		i++;
	}
}
void Program::loggedRun(int first, int second)
{
	bool* stop = new bool(false);

	SQLiteORM sqlORM;
	queue <std::future<void>> kbrdThread;
	queue <std::future<void>> msThread;
	thread thkbrd;
	thread thms;
	thread th;
	vector <map<string, string>>* keyBoardVector = new vector <map<string, string>>;
	vector <map<string, string>>* mouseVector = new vector <map<string, string>>;
	map <string, UINT> time = { {"MOUSE",0}, {"KEYBOARD",0} };
	this->queueReader.push(std::async(launch::async, [](vector <map<string, string>>* mouseVector) {
		SQLiteORM sqlORM;
		sqlORM.setTable("MOUSE")->open()->selectAll(mouseVector, (string)"TIME", (string)"DESC");
		sqlORM.close();
		return true;
	}, mouseVector));
	this->queueReader.push(std::async(launch::async, [](vector <map<string, string>>* keyBoardVector) {
		SQLiteORM sqlORM;
		sqlORM.setTable("KEYBOARD")->open()->selectAll(keyBoardVector, (string)"TIME", (string)"DESC");
		sqlORM.close();
		return true;
	}, keyBoardVector));

	for (int i = 0; i < this->queueReader.size(); i++) {
		this->queueReader.front().wait();
	}


	UINT timeDifference = getTime() - stoul(mouseVector->front().find("TIME")->second.c_str());

	th = thread([=]() mutable {
		std::mutex mx;
		if (*stop) {
			return;
		}
		while (!this->getInputControl().isPressedCombination(VK_LCONTROL, 0x33) && !*stop) {
		}
		*stop = true;
		mx.lock();
		std::cout << "Stopping play... please wait." << endl;
		mx.unlock();

	});

	thkbrd = thread([=]() mutable {
		std::mutex mx;
		while (!*stop && !this->getExit())
		{
			mx.lock();
			this->keyBoardPlay(time, &timeDifference, keyBoardVector, this->getInputControl(), *stop);
			mx.unlock();
		}
	});
	thms = thread([=]() mutable {
		std::mutex mx;
		while (!*stop && !this->getExit())
		{
			mx.lock();
			this->mousePlay(time, &timeDifference, mouseVector, this->getInputControl(), *stop);
			timeDifference = getTime() - stoul(mouseVector->front().find("TIME")->second.c_str());
			mx.unlock();
		}
	});

	th.detach();

	if (!thms.joinable())
		thms.join();

	if (!thkbrd.joinable())
		thkbrd.join();

	while (thms.joinable() && thkbrd.joinable() && !*stop && !this->getExit()) {}
	th.~thread();
	thms.detach();
	thms.~thread();
	thkbrd.detach();
	thkbrd.~thread();
}
void Program::mousePlay(map<string, UINT>& time, UINT* timeDifference, vector <map<string, string>>* mouseVector, InputControl& inputControl, bool& stop)
{
	this->mousePlayStatusStopped = false;
	if (mouseVector->empty() || stop) {
		this->mousePlayStatusStopped = true;
		return;
	}

	int dScreenX = 65535 / GetSystemMetrics(SM_CXVIRTUALSCREEN);
	int dScreenY = 65535 / GetSystemMetrics(SM_CYVIRTUALSCREEN);
	int count = 0;
	map<string, string>* prevmouse = &mouseVector->front();
	time.find("MOUSE")->second = stoi(mouseVector->front().find("TIME")->second);
	for (map<string, string> mouse : *mouseVector) {
		if (time.find("KEYBOARD")->second > 0 && !this->keyboardPlayStatusStopped) {
			while (stoi(mouse.find("TIME")->second) > time.find("KEYBOARD")->second) {};
		}

		while (stoi(mouse.find("TIME")->second) > (getTime() - *timeDifference)) {};

		if (stop) {
			return;
		}
		time.find("MOUSE")->second = stoi(mouse.find("TIME")->second);
		inputControl.getMouse().setMouseCursor(
			dScreenX * stoi(mouse.find("POSITION_X")->second),
			dScreenY * stoi(mouse.find("POSITION_Y")->second)
		);

	}
	this->mousePlayStatusStopped = true;
}
void Program::keyBoardPlay(map<string, UINT>& time, UINT* timeDifference, vector <map<string, string>>* keyBoardVector, InputControl& inputControl, bool& stop)
{
	this->keyboardPlayStatusStopped = false;
	if (keyBoardVector->empty() || stop) {
		this->keyboardPlayStatusStopped = true;
		return;
	}
	time.find("KEYBOARD")->second = stoi(keyBoardVector->front().find("TIME")->second);
	for (map<string, string> keyboard : *keyBoardVector) {
		if (time.find("MOUSE")->second > 0 && !this->mousePlayStatusStopped) {
			while (stoi(keyboard.find("TIME")->second) > time.find("MOUSE")->second) {};
		}

		while (stoi(keyboard.find("TIME")->second) > (getTime() - *timeDifference)) {};

		if (stop) {
			return;
		}
		time.find("KEYBOARD")->second = stoi(keyboard.find("TIME")->second);
		Key key = inputControl.getKeyByVirtualValue(stoi(keyboard.find("KEY_VALUE")->second));
		if (key.getVirtualKeyValue() != 0) {
			if (stoi(keyboard.find("RELEASED")->second) < 1) {
				key.press();
			}
			else {
				key.release();
			}
		}
	}
	this->keyboardPlayStatusStopped = true;
}
int Program::deleteFile()
{
	SQLiteORM sqlORM;
	return 1;//DeleteFileA(sqlORM.getFileName());
}
UINT Program::getTime()
{
	return static_cast<UINT>(time(0));
}

bool Program::getExit()
{
	return this->exit;
}
mutex& Program::getMutex()
{
	return this->mx;
}
void Program::setKeyReleaseWait(int key, bool isRuning)
{
	this->keyReleaseWait.find(key)->second = isRuning;
}
bool Program::getKeyReleaseWait(int key)
{
	return this->keyReleaseWait.find(key)->second;
}
