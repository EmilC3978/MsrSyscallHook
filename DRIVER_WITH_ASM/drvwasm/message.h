#pragma once

//Wrapper for DbgPrintEx function (Used for debug proposes)
#define DebugMessage(x,...) DbgPrintEx(0,0,x,__VA_ARGS__)