#pragma once
#include<Windows.h>

//define the IO control codes for the main subroutines of the driver (receive process PID and detoured address) 
#define IO_GET_PID_TO_ATTTACH CTL_CODE(FILE_DEVICE_UNKNOWN,0x101,METHOD_BUFFERED,FILE_SPECIAL_ACCESS)
#define IO_ENABLE_NEXT_SYSCALL CTL_CODE(FILE_DEVICE_UNKNOWN,0x102,METHOD_BUFFERED,FILE_SPECIAL_ACCESS)
