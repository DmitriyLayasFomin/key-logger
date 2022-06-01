#pragma once
#include "InputControl.h"
#include "SQLiteORM.h"
#include <map>
#include <queue>
#include <future>
#include <mutex>
#include <thread>
using namespace std;
class Program
{
public:
	Program(void);
	void start();
	InputControl* getInputControl();
	void recordInputControl(int virtualFirst, int virtualSecond);
private:
	InputControl inputControl;
	SQLiteORM keyboar;
	SQLiteORM mouse;
	map<string, SQLiteField> keyboarFields;
	map<string, SQLiteField> mouseFields;
	queue <std::future<void>> queueWriter;
	queue <std::future<void>> queueTask;
	mutex mutex;
	const int logInterval = 0;
	void writeKey(map<string, SQLiteField> keyboarFields, string keyIntValue, string time);
	void writeMouse(map<string, SQLiteField> mouseFields, string positionX, string positionY, string mouseIntValueL, string mouseIntValueR, string time);
	int getTime();
	vector<SQLiteField> getValues(map<string, SQLiteField> map);
};

