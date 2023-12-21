#pragma once
#include "Communication.h"
#include<Windows.h>
#include<stdio.h>

//Describe the operations that can be handled by the driver
class DriverInterface {
public:
	HANDLE hDriver;
	DWORD BytesReturned=0;
	//Create a UserSpace Objet to interface with the driver

	DriverInterface(LPCSTR RegistryPath)
	{
		hDriver = CreateFileA(RegistryPath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
	}

	//API to send the PID of the debugged process to the driver
	int SendPidToDriver(ULONG64 pid) {
		if (hDriver == INVALID_HANDLE_VALUE) {
			return -1;
		}
		else {
			//IO_GET_PID_TO_ATTTACH represents the IOCTL code for the operation
			bool response_from_driver=DeviceIoControl(hDriver, IO_GET_PID_TO_ATTTACH, &pid, sizeof(ULONG64), NULL, NULL, &BytesReturned, NULL);
			if (response_from_driver)
			{
				printf("PID sent Sucessful\n");
				return 0;
			}

		}
		return 0;
	}

	//API to send the addres of the detoured return address for any system call
	int SendCave(ULONG64 address) {
		if (hDriver == INVALID_HANDLE_VALUE) {
			return -1;
		}
		else {

			//IO_ENABLE_NEXT_SYSCALL represents the IOCTL code for the operation
			bool response_from_driver = DeviceIoControl(hDriver, IO_ENABLE_NEXT_SYSCALL, &address, sizeof(ULONG64), NULL, NULL, &BytesReturned, NULL);
			if (response_from_driver)
			{
				printf("Address of hook received\n");
				return 0;
			}

		}
		return 0;
	}
};