// Debugger.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include <Psapi.h>
#include <wchar.h>
#include <iostream>
#include <string>
#include "PDbg.h"
#include <thread>

DWORD GetActiveProcessId(LPTSTR applicationName)
{
	DWORD processIds[1024];
	DWORD bytesReturned;

	if (!EnumProcesses(processIds, sizeof(processIds), &bytesReturned))
	{
		std::cout << "Error while enumerating processes." << std::endl;
		exit(EXIT_FAILURE);
	}

	DWORD totalProcessAmount = bytesReturned / sizeof(DWORD);

	for (auto i = 0; i < bytesReturned; i++)
	{
		HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processIds[i]);

		if (hProcess == NULL)
		{
			continue;
		}

		TCHAR appName[MAX_PATH] = TEXT("<none>");
		
		GetProcessImageFileName(hProcess, appName, sizeof(appName));
		if (wcsstr(appName, applicationName) != NULL)
		{
			std::cout << "Process found!!! ID:" << processIds[i] << std::endl;
			_tprintf(TEXT("%s"), appName);
			CloseHandle(hProcess);
			
			return processIds[i];
		}
	}

	return 0;
}


int main()
{
	TCHAR process[] = TEXT("C:\\Users\\pb\\source\\repos\\Debugger\\Release\\SimpleApp.exe");

	DWORD processId = GetActiveProcessId(process);

	auto dbg = new PDbg();	
	dbg->StartDebugNewProcess(process);
	return 0;
}

