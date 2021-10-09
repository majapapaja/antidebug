#include <windows.h>
#include <stdio.h>
#include <Psapi.h>

// The size of a page 
#define PAGE_SIZE 0x1000

// A pseudo-handle that represents the current process.
#define NtCurrentProcess() ((HANDLE)-1)

// Multi-byte int 3.
BYTE multiByte[] = { 0xCD, 0x03 };

int main(void)
{
	int status = -1;

	//int3
	
	// allocate 2 pages of executable virtual memory.
	PBYTE Memory = (PBYTE)VirtualAlloc(NULL, (PAGE_SIZE * 2), (MEM_COMMIT | MEM_RESERVE), PAGE_EXECUTE_READWRITE);
	if (!Memory)
	{
		fprintf(stderr, "Memory ERROR\n");
		goto CleanupRoutine;
	}

    //location of multi-byte int 3 at the end of the first page
	PBYTE locationMultiByte = &Memory[PAGE_SIZE - sizeof(multiByte)];
	//location of the second page
	PBYTE locationPageTwo = &Memory[PAGE_SIZE];

	// Add the int 3 to the very end of page 1.
	memcpy(locationMultiByte, multiByte, sizeof(multiByte));
 
	PSAPI_WORKING_SET_EX_INFORMATION wsi;
	wsi.VirtualAddress = locationPageTwo;

	if (!QueryWorkingSetEx(NtCurrentProcess(), &wsi, sizeof(wsi)))
	{
		fprintf(stderr, "QueryWorkingSetEx ERROR\n");
		goto CleanupRoutine;
	}

	if (wsi.VirtualAttributes.Valid)
	{
		fprintf(stderr, "Page ERROR\n");
		goto CleanupRoutine;
	}
	
	__try
	{
		// Invoke the long form of int 3.
		((void(*)())locationMultiByte)();
	}
	__except (EXCEPTION_EXECUTE_HANDLER) { }

	if (!QueryWorkingSetEx(NtCurrentProcess(), &wsi, sizeof(wsi)))
	{
		fprintf(stderr, "QueryWorkingSetEx ERROR\n");
		goto CleanupRoutine;
	}

    if (wsi.VirtualAttributes.Valid){
		printf("debugger detected using int3\n");
	}else{
		printf("hello, Im just a programm!\n");
	}

	//trap flag

	BOOL trapFlag = TRUE;
	__try
	{
    	__asm
    	{
       		pushfd
        	or dword ptr[esp], 0x100 // set the Trap Flag 
        	popfd                    // Load the value into FLAGS register
        	nop
    	}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
    	// If an exception has been raised – debugger is not present
    	trapFlag = FALSE;
	}
	
	if (trapFlag)
	{
    	printf("debugger detected using Trap-Flag\n");
	}else{
		printf("doing normal programm stuff\n");
	}

	//isdebuggerpresent

	if (IsDebuggerPresent())
    {
        printf("debugger detected using isDebuggerPresent()\n");
    }else{
		printf("everything is fine!\n");
	}

	status = 0;

CleanupRoutine:
	
	// Free allocated memory.
	if (Memory)
	{
		VirtualFree(Memory, 0, MEM_FREE);
		Memory = NULL;
	}

	return status;
}