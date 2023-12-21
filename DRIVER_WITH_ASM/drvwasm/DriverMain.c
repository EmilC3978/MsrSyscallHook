#pragma once
#pragma warning (disable: 4028)
#include "DriverMain.h"
#include "message.h"
#include "Data.h"
#include "Communication.h"
//Implement the operations executed by the driver when it is loaded/unloaded from memory

//When the driver is loaded in memory
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryPath)
{
	//Assign the pointer the UnloadDriver function in the driver object
	UNREFERENCED_PARAMETER(pRegistryPath);
	pDriverObject->DriverUnload = UnloadDriver;

	//Initialize the names by which the driver is referred in kernel/user space
	RtlInitUnicodeString(&dev, L"\\Device\\hookdriver");
	RtlInitUnicodeString(&dos, L"\\??\\hookdriver");

	//Create a virtual device fot the driver
	IoCreateDevice(pDriverObject, 0, &dev, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &pDeviceObject);

	//Make a symbolic Link to the device to make it accessible from user space
	IoCreateSymbolicLink(&dos, &dev);

	//Initialize default operations (not used for this scope)
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = CreateCall;
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = CloseCall;
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IoControl;

	//Mode of interaction with the virtual device
	pDeviceObject->Flags |= DO_DIRECT_IO;

	pDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

	//The driver needs to dynamically resolve the address of some Kernel functions that will be used later
	//Initialize the name of the respective functions
	DebugMessage("Driver loaded with status");
	RtlInitUnicodeString(&protect, L"ZwProtectVirtualMemory");
	RtlInitUnicodeString(&nameIoAllocateMdl, L"IoAllocateMdl");
	RtlInitUnicodeString(&nameIoFreeMdl, L"IoFreeMdl");
	RtlInitUnicodeString(&nameMYMmMapLockedPagesSpecifyCache, L"MmMapLockedPagesSpecifyCache");
	RtlInitUnicodeString(&nameMmProbeAndLockPages, L"MmProbeAndLockPages");
	RtlInitUnicodeString(&nameMmUnlockPages, L"MmUnlockPages");
	RtlInitUnicodeString(&nameMmProtectMdlSystemAddress, L"MmProtectMdlSystemAddress");
	RtlInitUnicodeString(&nameKeRaiseIrqlToDpcLevel, L"KeRaiseIrqlToDpcLevel");
	RtlInitUnicodeString(&nameKeLowerIrql, L"KeLowerIrql");

	//Use MmGetSystemRoutineAddress function to resolve the address of the functions
	zwprotect = (ZWPROTECTVIRTUALMEMORY)MmGetSystemRoutineAddress(&protect);
	MYMmUnlockPages = (MMUNLOCKPAGES)MmGetSystemRoutineAddress(&nameMmUnlockPages);
	MYIoAllocateMdl = (IOALLOCATEMDL)MmGetSystemRoutineAddress(&nameIoAllocateMdl);
	MYIoFreeMdl = (IOFREEMDL)MmGetSystemRoutineAddress(&nameIoFreeMdl);
	MYMmMapLockedPagesSpecifyCache = (MMMAPLOCKEDPAGESSPECIFYCACHE)MmGetSystemRoutineAddress(&nameMYMmMapLockedPagesSpecifyCache);
	MYMmProbeAndLockPages = (MMPROBEANDLOCKPAGES)MmGetSystemRoutineAddress(&nameMmProbeAndLockPages);
	MYMmProtectMdlSystemAddress = (MMPROTECTMDLSYSTEMADDRESS)MmGetSystemRoutineAddress(&nameMmProtectMdlSystemAddress);
	MYKeRaiseIrqlToDpcLevel = (KERAISEIRQLTODPCLEVEL)MmGetSystemRoutineAddress(&nameKeRaiseIrqlToDpcLevel);
	MYKeLowerIrql = (KELOWERIRQL)MmGetSystemRoutineAddress(&nameKeLowerIrql);

	//Debug messages to check MmGetSystemRoutineAddress return values
	DebugMessage("protectaddr:%llu\n", (DWORD64)zwprotect);
	DebugMessage("MYMmUnlockPages:%llu\n", (DWORD64)MYMmUnlockPages);
	DebugMessage("MYIoAllocateMdl:%llu\n", (DWORD64)MYIoAllocateMdl);
	DebugMessage("MYIoFreeMdl:%llu\n", (DWORD64)MYIoFreeMdl);
	DebugMessage("MmMapLockedPagesSpecifyCache:%llu\n", (DWORD64)MYMmMapLockedPagesSpecifyCache);
	DebugMessage("MYMmProbeAndLockPages:%llu\n", (DWORD64)MYMmProbeAndLockPages);
	DebugMessage("MYMmProtectMdlSystemAddress:%llu\n", (DWORD64)MYMmProtectMdlSystemAddress);
	DebugMessage("MYKeRaiseIrqlToDpcLevel:%llu\n", (DWORD64)MYKeRaiseIrqlToDpcLevel);
	DebugMessage("MYKeLowerIrql:%llu\n", (DWORD64)MYKeLowerIrql);
	return STATUS_SUCCESS;
}

//Function tat get's called whan the diver is unloaded from memory
NTSTATUS UnloadDriver(PDRIVER_OBJECT pDriverObject)
{
	IoDeleteSymbolicLink(&dos);
	IoDeleteDevice(pDriverObject->DeviceObject);
	DebugMessage("Driver unloaded");

	//Restore the changes made to certain Kernel data structures
	RestoreLstar();
	return STATUS_SUCCESS;
}