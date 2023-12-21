#pragma once
#include<ntifs.h>
#include<Wdm.h>
#include<wdf.h>
//Declare 2 mandatory functions that need to be implemented in any driver
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryPath);
NTSTATUS UnloadDriver(PDRIVER_OBJECT pDriverObject);

