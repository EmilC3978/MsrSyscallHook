//Contains the logic of the LSTAR hooking process, implemented partly in this file and in inline.asm file

#include"Communication.h"
#include "Data.h"
#include "message.h"
#include <Ntddk.h>
#pragma warning (disable: 4028)
//global variables

//Debugged process for which the return address from system calls are re-routed after completion
ULONG64 TargetProcessId = MAXULONG64;

//Initial value of the lstar register
DWORD64 truemsr =0;

//Location in the kernel where the driver will return after hook subroutine is complete
DWORD64 fake_msr;

//Modified Address to which the system call will return in user-space
DWORD64 afake_return_address;

//Address of local variables
//DWORD64 hookfunction;
//DWORD64 astack_pointer;
//DWORD64 aoriginal_return;

//Address of assembly function that performs User->Kernel stack switching and filtering based on process ID
extern void inline MainAsm(void);

//Function that modifies LSTAR REGISTER
void HookLstar(){
	//hookfunction = (DWORD64)WorkerHook;

	//Raise priority so that the action is not interrupted by any process
	KIRQL kirql = KeRaiseIrqlToDpcLevel();
	if (truemsr == 0x0){

		//Read the LSTAR register Value
		truemsr = __readmsr(0xc0000082);
	}
	//The hook soubrutine will re-implement some instructions executed by the kernel, so the return to the normal sys-call
	//handler is displaced by an offset
	fake_msr = truemsr + 0x15;

	//Write the address of the hook subroutine in the LSTAR register
	__writemsr(0xc0000082, MainAsm);
	KeLowerIrql(kirql);
}

//Function that restores the original contents of the LSTAR register
void RestoreLstar() {
	KIRQL kirql = KeRaiseIrqlToDpcLevel();
	__writemsr(0xc0000082, truemsr);
	KeLowerIrql(kirql);
	return;
}

//Main function that performs redirection of the instruction flow after completion of a syscal to the address supplied by the user
//Note:
//In the current form the, function and it's local variables are located in the drivers, code/stack section
//In a multi thread/process environment, where a thread can be interrupted before it finishes a system call the application
//needs to use the respective's thread kernel stack to store any local variables.
//Due to this limitation, the code of the function is compiled, disassembled and written in the inline.asm file (where is actually executed)
//Global variables such as TargetProcessId remain the same, and can be stored in the driver's memory.
void WorkerHook() {

	DWORD64 stack_pointer=0x8888;
	DWORD64 original_return=0x7777;
	PMDL iomdlalloc = NULL;
	PVOID* base = NULL;
	DWORD64 aligned_stackpointer;
	KIRQL irql;
	DWORD64 sucess = 0;
	NTSTATUS result_protect;
	aligned_stackpointer = stack_pointer & (~0xfff);
	irql = MYKeRaiseIrqlToDpcLevel();

	//The mechanism for assuring that eventually the system call will hit the original return address implies writing it
	//on the user stack of the respective thread. The following steps are necessary to write form the kernel space to the
	//user stack of the thread that made the system call.
	iomdlalloc = MYIoAllocateMdl((PVOID)aligned_stackpointer, 0x1000, FALSE, 0, NULL);
	if (iomdlalloc != NULL) {
		MYMmProbeAndLockPages(iomdlalloc, KernelMode, IoModifyAccess);
		base = MYMmMapLockedPagesSpecifyCache(iomdlalloc, KernelMode, MmCached, NULL, FALSE, HighPagePriority);
		result_protect=MYMmProtectMdlSystemAddress(iomdlalloc, PAGE_READWRITE);
		if (base != NULL&&(NTSTATUS)result_protect>=0) {
			*(PDWORD64)(stack_pointer - 8 - aligned_stackpointer + (DWORD64)base) = original_return;
			sucess = 1;
		}
		MYMmUnlockPages(iomdlalloc);
		MYIoFreeMdl(iomdlalloc);
	}
	MYKeLowerIrql(irql);
	

	return;
}

//Default function implementation, not used in the project
NTSTATUS CreateCall(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

//Default function implementation, not used in the project
NTSTATUS CloseCall(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

//Function that receives the request from the user-mode application, parses it according to it's Control Code
NTSTATUS IoControl(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
	ULONG ControlCode = stack->Parameters.DeviceIoControl.IoControlCode;

	//If the user entered a PID then, TargetProcessId takes the value entered LSTAR register is modified
	if (ControlCode == IO_GET_PID_TO_ATTTACH) {
		PULONG64 aux_ptr = Irp->AssociatedIrp.SystemBuffer;
		DebugMessage("Receved PID:%llu", TargetProcessId);
		if(TargetProcessId==MAXULONG64)
		{
			TargetProcessId = *aux_ptr;
			//Hook the LSTAR Register
			HookLstar();
			DebugMessage("LSTAR HOOKED");
		}
		TargetProcessId = *aux_ptr;
		Status = STATUS_SUCCESS;
	}

	//If the user entered an an address, then afake_return_address is assigned to the respective address and any subsequent
	//system calls that the target process makes will return in userspace to afake_return_address
	else if (ControlCode==IO_ENABLE_NEXT_SYSCALL) {
		PULONG64 aux_ptr = Irp->AssociatedIrp.SystemBuffer;
		afake_return_address = *aux_ptr;
		Status = STATUS_SUCCESS;
	}
	Irp->IoStatus.Status = Status;
	Irp->IoStatus.Information = sizeof(ULONG);
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return Status;
}