#include "Program.h"


Program::Program()
{
	this->keyboarFields = {
		{"KEY_VALUE", SQLiteField("KEY_VALUE","INT")},
		{"TIME", SQLiteField("TIME","INT")},
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
	this->queueWriter.push(std::async(std::launch::async, [&](map<string, SQLiteField> keyboarFields) {
		SQLiteORM sqlORM;
		sqlORM.setTable("KEYBOARD")->open()->insert(this->getValues(keyboarFields))->close();
	}, keyboarFields));
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
	this->queueWriter.push(std::async(std::launch::async, [&](map<string, SQLiteField> mouseFields) mutable {
		SQLiteORM sqlORM;
		sqlORM.setTable("MOUSE")->open()->insert(this->getValues(mouseFields))->close();
	}, mouseFields));
}
void Program::recordInputControl(int virtualFirst, int virtualSecond)
{
	while ((!(GetAsyncKeyState('0') & 0x80)) && !this->inputControl.isPressedCombination(virtualFirst, virtualSecond) ) {
		if (this->logInterval > 0) {
			Sleep(this->logInterval);
		}
		int lastVirtual = this->inputControl.getLastPressedVirtualKey();
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
int Program::deleteFile()
{
	SQLiteORM sqlORM;
	return DeleteFileA(sqlORM.getFileName());
}
int Program::getTime()
{
	using namespace std::chrono;
	return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}
