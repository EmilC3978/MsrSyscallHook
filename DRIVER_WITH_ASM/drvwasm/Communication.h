#pragma once
#include<ntifs.h>

//define the IO control codes for the main subroutines of the driver (receive process PID and detoured address) 
#define IO_GET_PID_TO_ATTTACH CTL_CODE(FILE_DEVICE_UNKNOWN,0x101,METHOD_BUFFERED,FILE_SPECIAL_ACCESS)
#define IO_ENABLE_NEXT_SYSCALL CTL_CODE(FILE_DEVICE_UNKNOWN,0x102,METHOD_BUFFERED,FILE_SPECIAL_ACCESS)

//Unused in the project
NTSTATUS IoControl(DEVICE_OBJECT, PIRP);
NTSTATUS CloseCall(DEVICE_OBJECT, PIRP);
NTSTATUS CreateCall(DEVICE_OBJECT, PIRP);

//The function which hooks LSTAR register to redirect the entry in Kernel space once a system call is executed by a process in userspace
void HookLstar();

//The function wihch implements the custom driver system cal routine, in the first phase of the project
void WorkerHook();

//Functions which restores the LSTAR register to the orginal value, thus restoring the hook
void RestoreLstar();

//Declare pointers to the kernel functions whose address was dynamically resoled in DriverMain.c
typedef NTSTATUS(NTAPI* ZWPROTECTVIRTUALMEMORY)(
    HANDLE ProcessHandle,
    PVOID64* BaseAddress,
    PULONG ProtectSize,
    ULONG NewProtect,
    PULONG OldProtect
    );

typedef PMDL(NTAPI* IOALLOCATEMDL)(
    _In_opt_ __drv_aliasesMem PVOID VirtualAddress,
    _In_ ULONG Length,
    _In_ BOOLEAN SecondaryBuffer,
    _In_ BOOLEAN ChargeQuota,
    _Inout_opt_ PIRP Irp
    );

typedef void(NTAPI* MMPROBEANDLOCKPAGES)(
    _Inout_ PMDL MemoryDescriptorList,
    _In_ KPROCESSOR_MODE AccessMode,
    _In_ LOCK_OPERATION Operation
    );

typedef PVOID(NTAPI* MMGETSYSTEMADDRESSFORMDLSAFE)(
    _Inout_ PMDL Mdl,
    _In_    ULONG Priority
    );

typedef void(NTAPI* MMUNLOCKPAGES)(
    _Inout_ PMDL MemoryDescriptorList
    );

typedef void (NTAPI* IOFREEMDL)(
    PMDL Mdl
    );
typedef PVOID(NTAPI* MMMAPLOCKEDPAGESSPECIFYCACHE)(
    _Inout_ PMDL MemoryDescriptorList,
    _In_ __drv_strictType(KPROCESSOR_MODE / enum _MODE, __drv_typeConst)
    KPROCESSOR_MODE AccessMode,
    _In_ __drv_strictTypeMatch(__drv_typeCond) MEMORY_CACHING_TYPE CacheType,
    _In_opt_ PVOID RequestedAddress,
    _In_     ULONG BugCheckOnFailure,
    _In_     ULONG Priority
    );

typedef NTSTATUS(NTAPI* MMPROTECTMDLSYSTEMADDRESS)(
    _In_ PMDL MemoryDescriptorList,
    _In_ ULONG NewProtect
    );

typedef KIRQL(NTAPI* KERAISEIRQLTODPCLEVEL)(
    VOID
    );

typedef VOID(NTAPI* KELOWERIRQL)(
    _In_ _Notliteral_ _IRQL_restores_ KIRQL NewIrql
    );
ZWPROTECTVIRTUALMEMORY zwprotect;
IOALLOCATEMDL MYIoAllocateMdl;
MMPROBEANDLOCKPAGES MYMmProbeAndLockPages;
MMMAPLOCKEDPAGESSPECIFYCACHE MYMmMapLockedPagesSpecifyCache;
MMUNLOCKPAGES MYMmUnlockPages;
IOFREEMDL MYIoFreeMdl;
MMPROTECTMDLSYSTEMADDRESS MYMmProtectMdlSystemAddress;
KERAISEIRQLTODPCLEVEL MYKeRaiseIrqlToDpcLevel;
KELOWERIRQL MYKeLowerIrql;
UNICODE_STRING nameMYMmMapLockedPagesSpecifyCache, protect, nameIoAllocateMdl, nameMmProbeAndLockPages, nameMmUnlockPages, nameIoFreeMdl,
nameMmProtectMdlSystemAddress, nameKeRaiseIrqlToDpcLevel, nameKeLowerIrql;
