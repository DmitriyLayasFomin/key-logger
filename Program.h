#pragma once
#include "InputControl.h"
#include "SQLiteORM.h"
#include <map>
#include <queue>
#include <future>
#include <mutex>
#include <thread>
#include <chrono>
using namespace std;
class Program
{
public:
	Program();
	~Program();
	void start();
	InputControl& getInputControl();
	void recordInputControl(int virtualFirst, int virtualSecond);
	void filterLog(vector<map<string, SQLiteField>>& keyboardVec);

	int deleteFile();
	static unsigned long int getTime();
	mutex& getMutex();
	void setKeyReleaseWait(int key, bool isRuning);
	bool getKeyReleaseWait(int key);
private:
	bool exit = false;
	bool keyboardPlayStatusStopped = false;
	bool mousePlayStatusStopped = false;
	InputControl inputControl;
	SQLiteORM keyboar;
	SQLiteORM mouse;
	map<string, SQLiteField> keyboarFields;
	map<string, SQLiteField> mouseFields;
	map <int, bool> keyReleaseWait;
	mutex mx;
	const int logIntervalKey = 0;
	const int logIntervalCursor = 15;

	bool getExit();
	void writeKey(vector <map<string, SQLiteField>> vec);
	void writeMouse(vector <map<string, SQLiteField>> vec);
	void writeMouseInVector(vector<map<string, SQLiteField>>* vec, map<string, SQLiteField> mouseFields, string positionX, string positionY, string time);
	void writeKeyInVector(vector<map<string, SQLiteField>>* vec, map<string, SQLiteField> keyboarFields, string keyIntValue, string released, string time);
	void loggedRun(int first, int second);
	void mousePlay(map<string, unsigned long> &time, unsigned long &timeDifference, vector<map<string, string>>* mouseVector, InputControl& inputControl, bool& stop);
	void keyBoardPlay(map<string, unsigned long> &time, unsigned long &timeDifference, vector<map<string, string>>* keyBoardVector, InputControl& inputControl, bool& stop);
	vector<SQLiteField> getValues(map<string, SQLiteField> map);
};

