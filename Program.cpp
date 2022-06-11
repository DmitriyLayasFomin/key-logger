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
		{"MOUSE_VALUE_L", SQLiteField("MOUSE_VALUE_L","INT")},
		{"MOUSE_VALUE_R", SQLiteField("MOUSE_VALUE_R","INT")},
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
				this->mutex.unlock();
			}
			if (this->getInputControl()->isPressedCombination(VK_LCONTROL, 0x37)) {
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
		while (true) {
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
	string mouseIntValueL,
	string mouseIntValueR,
	string time
)
{
	mouseFields.find("POSITION_X")->second.setValue(positionX);
	mouseFields.find("POSITION_Y")->second.setValue(positionY);
	mouseFields.find("MOUSE_VALUE_L")->second.setValue(mouseIntValueL);
	mouseFields.find("MOUSE_VALUE_R")->second.setValue(mouseIntValueR);
	mouseFields.find("TIME")->second.setValue(time);
	this->queueWriter.push(std::async(launch::async, [&](map<string, SQLiteField> mouseFields) mutable {
		SQLiteORM sqlORM;
		sqlORM.setTable("MOUSE")->open()->insert(this->getValues(mouseFields))->close();
	}, mouseFields));
}
void Program::recordInputControl(int virtualFirst, int virtualSecond)
{
	while (!this->inputControl.isPressedCombination(virtualFirst, virtualSecond) ) {
		
		if (this->logInterval > 0) {
			Sleep(this->logInterval);
		}
		int lastVirtual = this->inputControl.getLastPressedVirtualKey();
		if (lastVirtual == 0) {
			continue;
		}
		int time = this->getTime();
		this->writeKey(this->keyboarFields, std::to_string(lastVirtual), std::to_string(time));
		Mouse mouse = this->inputControl.getMouse().getLastState();
		this->writeMouse(
			this->mouseFields,
			std::to_string(mouse.getPositionX()),
			std::to_string(mouse.getPositionY()),
			std::to_string(mouse.isLeftPressed()),
			std::to_string(mouse.isRightPressed()),
			std::to_string(time)
		);
	}
}
void Program::loggedRun(int first, int second)
{
	SQLiteORM sqlORM;
	
	vector <map<string, string>>* keyBoardVector = new vector <map<string, string>>;
	vector <map<string, string>>* mouseVector = new vector <map<string, string>>;
	map <string, int> time = { {"MOUSE",0}, {"KEYBOARD",0} };

	this->queueReader.push(std::async(launch::async, [](vector <map<string, string>>* mouseVector){
		SQLiteORM sqlORM;
		sqlORM.setTable("MOUSE")->open()->selectAll(mouseVector, (string)"TIME", (string)"DESC");
		sqlORM.close();
		return true;
	}, mouseVector));
	this->queueReader.push(std::async(launch::async, [](vector <map<string, string>>* keyBoardVector){
		SQLiteORM sqlORM;
		sqlORM.setTable("KEYBOARD")->open()->selectAll(keyBoardVector, (string)"TIME", (string)"DESC");
		sqlORM.close();
		return true;
	}, keyBoardVector));
	
	for (int i = 0; i < this->queueReader.size(); i++) {
		this->queueReader.front().wait();
	}

	thread kbrdThread([&time,&keyBoardVector](InputControl* inputControl) mutable {
		time.find("KEYBOARD")->second = stoi(keyBoardVector->front().find("TIME")->second);
		for (map<string, string> keyboard : *keyBoardVector) {
			if (stoi(keyboard.find("TIME")->second) < time.find("MOUSE")->second) {
				while (stoi(keyboard.find("TIME")->second) >= time.find("MOUSE")->second) {
					continue;
				}
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
	}, this->getInputControl());
	thread msThread([&time, &mouseVector](InputControl* inputControl) mutable {
		time.find("MOUSE")->second = stoi(mouseVector->front().find("TIME")->second);
		for (map<string, string> mouse : *mouseVector) {
			if (stoi(mouse.find("TIME")->second) < time.find("KEYBOARD")->second) {
				while (stoi(mouse.find("TIME")->second) >= time.find("KEYBOARD")->second) {
					continue;
				}
			}
			time.find("MOUSE")->second = stoi(mouse.find("TIME")->second);

		}
	}, this->getInputControl());
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
