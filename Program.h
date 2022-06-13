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
	int deleteFile();
	static int getTime();
private:
	bool exit = false;
	InputControl inputControl;
	SQLiteORM keyboar;
	SQLiteORM mouse;
	map<string, SQLiteField> keyboarFields;
	map<string, SQLiteField> mouseFields;
	queue <std::future<void>> queueWriter;
	queue <std::future<void>> queueTask;
	queue <std::future<bool>> queueReader;
	mutex mutex;
	const int logIntervalKey = 150;
	const int logIntervalCursor = 30;

	void writeKey(map<string, SQLiteField> keyboarFields, string keyIntValue, string time);
	void writeMouse(map<string, SQLiteField> mouseFields, string positionX, string positionY, string time);
	void loggedRun(int first, int second);
	void mousePlay(map<string, int>& time, int& timeDifference, vector<map<string, string>>* mouseVector, InputControl* inputControl);
	void keyBoardPlay(map<string, int>& time, int& timeDifference, vector<map<string, string>>* keyBoardVector, InputControl* inputControl);
	vector<SQLiteField> getValues(map<string, SQLiteField> map);
};

