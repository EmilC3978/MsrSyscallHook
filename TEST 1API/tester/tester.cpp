// tester.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include<stdio.h>
#include<stdlib.h>
#include<Windows.h>
int main()
{
	unsigned char alfa[120];
	DWORD64 addresses[12];
	std::cin >> alfa;
	for (int i = 0; i < 10; i++)
	{
		//printf_s("Runda %d", i);
		addresses[i] = (DWORD64)VirtualAlloc(NULL, 0x1000, 0x00001000, 0x40);
	}
	return 0;
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
