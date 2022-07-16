
#include "Program.h"


int main()
{
	Program programm;
	
	cout << "Key logger working!" << endl;
	cout << "CTRL + 2 - switch start / stop logging." << endl;
	cout << "CTRL + 3 - logged record play." << endl;
	cout << "CTRL + 7 exit." << endl;
	programm.start();
	programm.deleteFile();
	return 0;
}
