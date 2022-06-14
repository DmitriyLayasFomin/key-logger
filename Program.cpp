#include "Program.h"


Program::Program()
{
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
}
void Program::start()
{
	this->queueTask.push(std::async(std::launch::async, [&, this] {
		while (!(GetAsyncKeyState('0') & 0x80)) {
			if (this->getInputControl()->isPressedCombination(VK_LCONTROL, 0x31)) {
				this->mutex.lock();
				cout << "Set input start." << endl;
				this->getInputControl()->init();
				cout << "Set input codes is succsess." << endl;
				this->mutex.unlock();
			}
			if (this->getInputControl()->isPressedCombination(VK_LCONTROL, 0x32)) {
				this->mutex.lock();
				cout << "Kyelogger start." << endl;
				Sleep(1000);
				MessageBeep(MB_ICONINFORMATION);
				this->mutex.unlock();
				this->recordInputControl(VK_LCONTROL, 0x32);
				this->mutex.lock();
				cout << "Kyelogger stop." << endl;
				MessageBeep(MB_ICONERROR);
				this->mutex.unlock();
			}
			if (this->getInputControl()->isPressedCombination(VK_LCONTROL, 0x33)) {
				this->mutex.lock();
				cout << "logged repeat start." << endl;
				MessageBeep(MB_ICONINFORMATION);
				this->mutex.unlock();
				this->loggedRun(VK_LCONTROL, 0x33);
				this->mutex.lock();
				cout << "logged repeat stop." << endl;
				MessageBeep(MB_ICONERROR);
				Sleep(1000);
				this->mutex.unlock();
			}
			if (this->getInputControl()->isPressedCombination(VK_LCONTROL, 0x37)) {
				for (int i = 0; i < this->queueWriter.size(); i++) {
					this->queueWriter.front().wait();
				}
				for (int i = 0; i < this->queueReader.size(); i++) {
					this->queueReader.front().wait();
				}
				this->mutex.lock();
				this->exit = true;
				cout << "shutdown." << endl;
				this->mutex.unlock();
				break;
			}
		}
	}));
}
InputControl* Program::getInputControl()
{
	return &this->inputControl;
}
vector<SQLiteField> Program::getValues(map<string, SQLiteField> map)
{
	vector<SQLiteField> result;
	for (auto i = map.begin(); i != map.end(); i++) {
		result.push_back(i->second);
	}
	return result;
}
void Program::writeKey(map<string, SQLiteField> keyboarFields, string keyIntValue, string time)
{
	keyboarFields.find("KEY_VALUE")->second.setValue(keyIntValue);
	keyboarFields.find("TIME")->second.setValue(time);
	keyboarFields.find("RELEASED")->second.setValue(string("0"));
	this->queueWriter.push(std::async(launch::async, [&](map<string, SQLiteField> keyboarFields, string keyIntValue) {
		SQLiteORM sqlORM;
		sqlORM.setTable("KEYBOARD")->open()->insert(this->getValues(keyboarFields))->close();
		while (!this->exit && !this->getInputControl()->isPressedCombination(VK_LCONTROL, 0x32)) {
			if (!(GetAsyncKeyState(atoi(keyIntValue.c_str())) < 0)) {
				keyboarFields.find("TIME")->second.setValue(to_string(getTime()));
				keyboarFields.find("RELEASED")->second.setValue("1");
				sqlORM.setTable("KEYBOARD")->open()->insert(this->getValues(keyboarFields))->close();
				break;
			}
		}
	}, keyboarFields, keyIntValue));
}
void Program::writeMouse(
	map<string, SQLiteField> mouseFields,
	string positionX,
	string positionY,
	string time
)
{
	mouseFields.find("POSITION_X")->second.setValue(positionX);
	mouseFields.find("POSITION_Y")->second.setValue(positionY);
	mouseFields.find("TIME")->second.setValue(time);
	this->queueWriter.push(std::async(launch::async, [&](
		map<string, SQLiteField> mouseFields) {
		SQLiteORM sqlORM;
		sqlORM.setTable("MOUSE")->open()->insert(this->getValues(mouseFields))->close();
	}, mouseFields));
}
void Program::recordInputControl(int virtualFirst, int virtualSecond)
{
	bool stop = false;
	thread th = thread([this, &stop, &virtualFirst, &virtualSecond]() {
		while (!this->inputControl.isPressedCombination(virtualFirst, virtualSecond)) {
			
		}
		stop = true;
	});
	thread thkbrd = thread([this, &stop, &virtualFirst, &virtualSecond]() mutable {
		while (!stop) {
			if (this->logIntervalKey > 0) {
				Sleep(this->logIntervalKey);
			}
			int lastVirtual = this->inputControl.getLastPressedVirtualKey();
			int time = getTime();
			if (lastVirtual != 0) {
				this->writeKey(this->keyboarFields, std::to_string(lastVirtual), std::to_string(time));
			}
		}
	});
	thread thmouse = thread([this, &stop, &virtualFirst, &virtualSecond]() mutable {
		while (!stop) {
			if (this->logIntervalCursor > 0) {
				Sleep(this->logIntervalCursor);
			}
			Mouse mouse = this->inputControl.getMouse().getLastState();
			int time = getTime();
			this->writeMouse(
				this->mouseFields,
				std::to_string(mouse.getPositionX()),
				std::to_string(mouse.getPositionY()),
				std::to_string(time)
			);
		}
	});
	th.detach();
	thkbrd.join();
	thmouse.join();
	th.~thread();
	thkbrd.~thread();
	thmouse.~thread();
}
void Program::loggedRun(int first, int second)
{
	bool *stop = new bool(false);
	bool a = *stop;
	SQLiteORM sqlORM;
	queue <std::future<void>> kbrdThread;
	queue <std::future<void>> msThread;
	thread thkbrd;
	thread thms;
	thread th;
	vector <map<string, string>>* keyBoardVector = new vector <map<string, string>>;
	vector <map<string, string>>* mouseVector = new vector <map<string, string>>;
	map <string, int> time = { {"MOUSE",0}, {"KEYBOARD",0} };
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

	int timeDifference = getTime() - stoi(mouseVector->front().find("TIME")->second);

	auto fkbrd = [this, &stop, &time, &timeDifference, &keyBoardVector](InputControl* inputControl) mutable {
		this->keyBoardPlay(time, timeDifference, keyBoardVector, inputControl, stop);
	};
	auto fmsThread = [this, &stop, &time, &timeDifference, &mouseVector](InputControl* inputControl) mutable {
		this->mousePlay(time, timeDifference, mouseVector, inputControl, stop);
	};
	
	while (!this->exit && !*stop) {
		timeDifference = getTime() - stoi(mouseVector->front().find("TIME")->second);
		kbrdThread.push(std::async(std::launch::async, fkbrd, this->getInputControl()));
		msThread.push(std::async(std::launch::async, fmsThread, this->getInputControl()));
		th = thread([this, &stop, &kbrdThread,&msThread]() mutable {
			
			if (*stop) {
				return;
			}
			while (!this->getInputControl()->isPressedCombination(VK_LCONTROL, 0x33) && !*stop) {

			}
			*stop = true;
			this->mutex.lock();
			std::cout << "Stopping play... please wait." << endl;
			this->mutex.unlock();
			return;
		});
		thkbrd = thread([&kbrdThread, &stop]() mutable {
			while (kbrdThread.size() > 0)
			{
				if (*stop) {
					while (kbrdThread.size() > 0) {
						kbrdThread.pop();
					}
				}
				else {
					kbrdThread.front().wait();
					kbrdThread.pop();
				}
			}
		});
		thms = thread([&msThread, &stop]() mutable {
			while (msThread.size() > 0)
			{
				if (*stop) {
					while (msThread.size() > 0) {
						msThread.pop();
					}
				}
				else {
					msThread.front().wait();
					msThread.pop();
				}
			}
		});
		thms.join();
		thkbrd.join();
		th.detach();
		
		while(thms.joinable() || thkbrd.joinable()){}
	}
}
void Program::mousePlay(map<string, int>& time, int& timeDifference, vector <map<string, string>>* mouseVector, InputControl* inputControl, bool* stop)
{
	this->mousePlayStatusStopped = false;
	if (mouseVector->empty() || *stop) {
		this->mousePlayStatusStopped = true;
		return;
	}

	int dScreenX = 65535 / GetSystemMetrics(SM_CXVIRTUALSCREEN);
	int dScreenY = 65535 / GetSystemMetrics(SM_CYVIRTUALSCREEN);
	time.find("MOUSE")->second = stoi(mouseVector->front().find("TIME")->second);
	for (map<string, string> mouse : *mouseVector) {
		while (
			stoi(mouse.find("TIME")->second) > time.find("KEYBOARD")->second && 
			!(time.find("KEYBOARD")->second == 0) && !this->keyboardPlayStatusStopped &&
			stoi(mouse.find("TIME")->second) > getTime() - timeDifference
			) {
		}
		if (*stop) {
			return;
		}
		time.find("MOUSE")->second = stoi(mouse.find("TIME")->second);
		inputControl->getMouse().setMouseCursor(
			dScreenX * stoi(mouse.find("POSITION_X")->second),
			dScreenY * stoi(mouse.find("POSITION_Y")->second)
		);
	}
	this->mousePlayStatusStopped = true;
}
void Program::keyBoardPlay(map<string, int>& time, int& timeDifference, vector <map<string, string>>* keyBoardVector, InputControl* inputControl, bool* stop)
{
	this->keyboardPlayStatusStopped = false;
	if (keyBoardVector->empty() || *stop) {
		this->keyboardPlayStatusStopped = true;
		return;
	}
	time.find("KEYBOARD")->second = stoi(keyBoardVector->front().find("TIME")->second);
	for (map<string, string> keyboard : *keyBoardVector) {
		while (
			stoi(keyboard.find("TIME")->second) > time.find("MOUSE")->second &&
			!(time.find("MOUSE")->second == 0) &&
			!this->mousePlayStatusStopped &&
			stoi(keyboard.find("TIME")->second) > getTime() - timeDifference
			) {
		}
		if (*stop) {
			return;
		}
		time.find("KEYBOARD")->second = stoi(keyboard.find("TIME")->second);
		Key key = inputControl->getKeyByVirtualValue(stoi(keyboard.find("KEY_VALUE")->second));
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
int Program::getTime()
{
	using namespace std::chrono;
	return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}
