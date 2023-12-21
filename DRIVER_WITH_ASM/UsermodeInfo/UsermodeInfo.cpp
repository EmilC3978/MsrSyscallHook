// UsermodeInfo.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "DriverInterface.h"
int main()
{
	ULONG64 pid = 0,useless=0,cave_addr;
	//Initialization fot the virtual device driver

	DriverInterface driver = DriverInterface("\\\\.\\hookdriver");

	//Read the breakpoint detoured addressand send it to thre driver
	printf("Set codecave value in hex: ");
	scanf_s("%llx", &cave_addr);
	driver.SendCave(cave_addr);
	//Read the process ID f the process being debugged and send it to the driver

	printf("Get Pid: ");
	scanf_s("%llu", &pid);
	int response = driver.SendPidToDriver(pid);
	while (TRUE) {
		scanf_s("%llu", &useless);

	}

}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
