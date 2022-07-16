#include "Program.h"

Program::Program()
{
	getInputControl().init();
	inputControl.startListen();
	keyboarFields = {
		{"KEY_VALUE", SQLiteField("KEY_VALUE","INT")},
		{"TIME", SQLiteField("TIME","INT")},
		{"RELEASED", SQLiteField("RELEASED","INT")}
	};
	mouseFields = {
		{"POSITION_X", SQLiteField("POSITION_X","INT")},
		{"POSITION_Y", SQLiteField("POSITION_Y","INT")},
		{"TIME", SQLiteField("TIME","INT")},
	};
	SQLiteORM sqlORM;
	sqlORM.setTable("KEYBOARD")->open()->createTable(getValues(keyboarFields))->close();
	sqlORM.setTable("MOUSE")->open()->createTable(getValues(mouseFields))->close();
	for (int i = 0; i < getInputControl().countChars; i++) {
		keyReleaseWait.insert({ i, false });
	}

}
Program::~Program()
{
	inputControl.stoptListen();
}
void Program::start()
{
	while (!getInputControl().isPressedCombination(VK_CONTROL, 0x37)) {
		if (getInputControl().isPressedCombination(VK_CONTROL, 0x32)) {
			cout << "Kyelogger start." << endl;
			MessageBeep(MB_ICONINFORMATION);
			recordInputControl(VK_CONTROL, 0x32);
			cout << "Kyelogger stop." << endl;
			MessageBeep(MB_ICONERROR);
		}
		if (getInputControl().isPressedCombination(VK_CONTROL, 0x33)) {
			cout << "logged repeat start." << endl;
			MessageBeep(MB_ICONINFORMATION);
			Sleep(1000);
			loggedRun(VK_CONTROL, 0x33);
			cout << "logged repeat stop." << endl;
			MessageBeep(MB_ICONERROR);
		}
	}
	exit = true;
	cout << "shutdown." << endl;
}
InputControl& Program::getInputControl()
{
	return inputControl;
}
vector<SQLiteField> Program::getValues(map<string, SQLiteField> map)
{
	vector<SQLiteField> result;
	for (auto i = map.begin(); i != map.end(); i++) {
		result.push_back(i->second);
	}
	return result;
}
void Program::writeKeyInVector(vector <map<string, SQLiteField>>* vec, map<string, SQLiteField> keyboarFields, string keyIntValue, string released, string time)
{
	keyboarFields.find("KEY_VALUE")->second.setValue(keyIntValue);
	keyboarFields.find("RELEASED")->second.setValue(released);
	keyboarFields.find("TIME")->second.setValue(time);
	std::lock_guard<std::mutex> guard(getMutex());
	vec->push_back(keyboarFields);
}
void Program::writeMouseInVector(vector <map<string, SQLiteField>>* vec, map<string, SQLiteField> mouseFields, string positionX, string positionY, string time)
{
	mouseFields.find("POSITION_X")->second.setValue(positionX);
	mouseFields.find("POSITION_Y")->second.setValue(positionY);
	mouseFields.find("TIME")->second.setValue(time);
	std::lock_guard<std::mutex> guard(getMutex());
	vec->push_back(mouseFields);
}
void Program::writeKey(vector <map<string, SQLiteField>> vec)
{
	SQLiteORM sqlORM;
	sqlORM.setTable("KEYBOARD");
	for (map<string, SQLiteField> keyboarFields : vec) {
		std::lock_guard<std::mutex> guard(getMutex());
		sqlORM.open()->insert(getValues(keyboarFields))->close();
	}
}
void Program::writeMouse(vector <map<string, SQLiteField>> vec)
{
	SQLiteORM sqlORM;
	sqlORM.setTable("MOUSE");
	for (map<string, SQLiteField> mouseFields : vec) {
		std::lock_guard<std::mutex> guard(getMutex());
		sqlORM.open()->insert(getValues(mouseFields))->close();
	}
}
void Program::recordInputControl(int virtualFirst, int virtualSecond)
{
	bool stop = false;

	vector <map<string, SQLiteField>> keyboardVec;
	vector <map<string, SQLiteField>> mouseVec;
	thread th = thread([&, this]() {
		while (!getInputControl().isPressedCombination(virtualFirst, virtualSecond)) {}
		stop = true;
	});
	thread thkbrd = thread([&, this]() mutable {
		while (!stop) {
			int lastVirtual = inputControl.getLastVirtualKey(Status::DOWN);
			if (lastVirtual != 0) {
				writeKeyInVector(&keyboardVec, keyboarFields, std::to_string(lastVirtual), "0", std::to_string(getTime()));
				std::async(launch::async, [&, this](int lastVirtual, map<string, SQLiteField> keyboarFields) {
					if (inputControl.releaseEvent(lastVirtual)) {
						writeKeyInVector(&keyboardVec, keyboarFields, std::to_string(lastVirtual), "1", std::to_string(getTime()));
					}
				}, lastVirtual, keyboarFields);
			}
		}
	});
	thread thmouse = thread([&, this]() mutable {
		while (!stop) {
			if (logIntervalCursor > 0) {
				std::lock_guard<std::mutex> guard(getMutex());
				Sleep(logIntervalCursor);
			}
			Mouse mouse = getInputControl().getMouse().getLastState();
			writeMouseInVector(
				&mouseVec,
				mouseFields,
				std::to_string(mouse.getPositionX()),
				std::to_string(mouse.getPositionY()),
				std::to_string(getTime())
			);
		}
	});

	th.detach();
	thkbrd.detach();
	thmouse.detach();
	while (!stop && !getExit()) {}
	
	th.~thread();
	thkbrd.~thread();
	thmouse.~thread();

	cout << "Saving log..." << endl;
	int writingStop[2] = { 0,0 };
	auto thWriterKeyboard = thread([&, this]() mutable {writeKey(keyboardVec);  writingStop[0] = 1; });
	auto thWriterMouse = thread([&, this]() mutable {writeMouse(mouseVec);   writingStop[1] = 1; });
	while (writingStop[0] != 1 && writingStop[1] != 1){}
	thWriterKeyboard.detach();
	thWriterMouse.detach();
	thWriterKeyboard.~thread();
	thWriterMouse.~thread();
	cout << "Log has been saved." << endl;
}
void Program::filterLog(vector <map<string, SQLiteField>>& keyboardVec) {

}
void Program::loggedRun(int first, int second)
{
	bool stop = false;

	thread thkbrd;
	thread thms;
	thread th;
	vector <map<string, string>>* keyBoardVector = new vector <map<string, string>>;
	vector <map<string, string>>* mouseVector = new vector <map<string, string>>;
	map <string, unsigned long> time = { {"MOUSE",0}, {"KEYBOARD",0} };
	std::future<bool> moseData = std::async(launch::async, [](vector <map<string, string>>* mouseVector) {
		SQLiteORM sqlORM;
		sqlORM.setTable("MOUSE")->open()->selectAll(mouseVector, (string)"TIME", (string)"ASC");
		sqlORM.close();
		return true;
	}, mouseVector);
	std::future<bool> keyboardData =  std::async(launch::async, [](vector <map<string, string>>* keyBoardVector) {
		SQLiteORM sqlORM;
		sqlORM.setTable("KEYBOARD")->open()->selectAll(keyBoardVector, (string)"TIME", (string)"ASC");
		sqlORM.close();
		return true;
	}, keyBoardVector);

	moseData.wait();
	keyboardData.wait();

	unsigned long timeDifference = 0;

	th = thread([&, this]() mutable {
		std::mutex mx;
		while (!inputControl.isPressedCombination(VK_CONTROL, 0x33) && !stop) {}
		stop = true;
		mx.lock();
		std::cout << "Stopping play... please wait." << endl;
		mx.unlock();
	});

	thkbrd = thread([&, this]() mutable {
		std::mutex mx;
		while (!stop && !getExit())
		{
			keyBoardPlay(time, timeDifference, keyBoardVector, getInputControl(), stop);
		}
	});
	thms = thread([&, this]() mutable {
		std::mutex mx;
		while (!stop && !getExit())
		{
			mousePlay(time, timeDifference, mouseVector, getInputControl(), stop);
		}
	});

	th.detach();

	if (!thms.joinable())
		thms.join();

	if (!thkbrd.joinable())
		thkbrd.join();

	while (thms.joinable() && thkbrd.joinable() && !stop && !getExit()) {}
	thms.detach();
	thkbrd.detach();
	th.~thread();
	thms.~thread();
	thkbrd.~thread();
}
void Program::mousePlay(map<string, unsigned long int>& time, unsigned long& timeDifference, vector <map<string, string>>* mouseVector, InputControl& inputControl, bool& stop)
{
	if (mouseVector->empty() || stop) return;

	int dScreenX = 65535 / GetSystemMetrics(SM_CXVIRTUALSCREEN);
	int dScreenY = 65535 / GetSystemMetrics(SM_CYVIRTUALSCREEN);

	time.find("MOUSE")->second = stoul(mouseVector->front().find("TIME")->second.c_str());
	timeDifference = getTime() - stoul(mouseVector->front().find("TIME")->second.c_str());

	for (map<string, string> mouse : *mouseVector) {
		int diff = getTime() - stoul(mouse.find("TIME")->second.c_str()) - timeDifference;
		if (diff > logIntervalCursor) {
			std::lock_guard<std::mutex> guard(mx);
			timeDifference = getTime() - stoul(mouse.find("TIME")->second.c_str());
		}

		while (stoul(mouse.find("TIME")->second.c_str()) > (getTime() - timeDifference)) {}

		if (stop) return;
		
		time.find("MOUSE")->second = stoul(mouse.find("TIME")->second.c_str());
		inputControl.getMouse().setMouseCursor(
			dScreenX * stoul(mouse.find("POSITION_X")->second.c_str()),
			dScreenY * stoul(mouse.find("POSITION_Y")->second.c_str())
		);

	}
}
void Program::keyBoardPlay(map<string, unsigned long int>& time, unsigned long& timeDifference, vector <map<string, string>>* keyBoardVector, InputControl& inputControl, bool& stop)
{
	if (keyBoardVector->empty() || stop) return;

	time.find("KEYBOARD")->second = atol(keyBoardVector->front().find("TIME")->second.c_str());

	for (map<string, string> keyboard : *keyBoardVector) {

		while (stoul(keyboard.find("TIME")->second.c_str()) > time.find("MOUSE")->second && time.find("MOUSE")->second < 1) {};

		while (stoul(keyboard.find("TIME")->second.c_str()) > (getTime() - timeDifference)) {};
		int diff = getTime() - stoul(keyboard.find("TIME")->second.c_str()) - timeDifference;
		if (diff > logIntervalCursor) {
			std::lock_guard<std::mutex> guard(mx);
			timeDifference = getTime() - stoul(keyboard.find("TIME")->second.c_str());
		}
		if (stop) return;

		time.find("KEYBOARD")->second = stoul(keyboard.find("TIME")->second.c_str());
		Key key = inputControl.getKeyByVirtualValue(atoi(keyboard.find("KEY_VALUE")->second.c_str()));
		if (key.getVirtualKeyValue() != 0) {
			if (atoi(keyboard.find("RELEASED")->second.c_str()) < 1) key.press();
			else key.release();
		}
	}
}
int Program::deleteFile()
{
	SQLiteORM sqlORM;
	return 1;//DeleteFileA(sqlORM.getFileName());
}
unsigned long int Program::getTime()
{
	using std::chrono::duration_cast;
	using std::chrono::milliseconds;
	using std::chrono::system_clock;
	return static_cast<unsigned long int>(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
}

bool Program::getExit()
{
	return exit;
}
mutex& Program::getMutex()
{
	return mx;
}
void Program::setKeyReleaseWait(int key, bool isRuning)
{
	keyReleaseWait.find(key)->second = isRuning;
}
bool Program::getKeyReleaseWait(int key)
{
	return keyReleaseWait.find(key)->second;
}
