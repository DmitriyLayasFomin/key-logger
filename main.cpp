#define _CRT_SECURE_NO_WARNINGS
#include "Program.h"

int main()
{
	LPMSG lpmsg = new MSG;
	Program pr;
	cout << "start" << endl;
	pr.start();
	cout << "end" << endl;	
	return 0;
}
