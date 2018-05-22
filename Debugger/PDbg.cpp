#include "stdafx.h"
#include "PDbg.h"
#include <iostream>

bool PDbg::StartDebugActiveProcess(DWORD processId)
{
	if (!DebugActiveProcess(processId))
	{
		std::cout << "Cannot debug active process with id: " << processId << std::endl;

		return false;
	}

	this->run();

	return true;
}

void PDbg::run()
{
	std::cout << "Debbuging started(run)..." << std::endl;

	DEBUG_EVENT dbgEvent;

	do
	{
		WaitForDebugEvent(&dbgEvent, INFINITE);

		switch (dbgEvent.dwDebugEventCode)
		{
			case CREATE_PROCESS_DEBUG_EVENT:	this->handle_create_process_debug_event(&dbgEvent); break;
			case CREATE_THREAD_DEBUG_EVENT:		this->handle_create_thread_debug_event(&dbgEvent); break;
			case EXCEPTION_DEBUG_EVENT:			this->handle_exception_debug_event(&dbgEvent); break;
			case LOAD_DLL_DEBUG_EVENT:			this->handle_load_dll_debug_event(&dbgEvent); break;
			case UNLOAD_DLL_DEBUG_EVENT:		this->handle_unload_dll_debug_event(&dbgEvent); break;
			case OUTPUT_DEBUG_STRING_EVENT:		this->handle_output_debug_string(&dbgEvent); break;
			case EXIT_THREAD_DEBUG_EVENT:		this->handle_exit_thread_debug_event(&dbgEvent); break;
			case EXIT_PROCESS_DEBUG_EVENT:		this->handle_exit_process_debug_event(&dbgEvent); break;
		}

		ContinueDebugEvent(dbgEvent.dwProcessId, dbgEvent.dwThreadId, DBG_CONTINUE);

	} while (!this->_processes.empty());
}

void PDbg::handle_create_process_debug_event(DEBUG_EVENT * dbgEvent)
{
	printf("Event: Create process, PID: %u, Base address: %p, Start address: %p\n",
		dbgEvent->dwProcessId, dbgEvent->u.CreateProcessInfo.lpBaseOfImage, dbgEvent->u.CreateProcessInfo.lpStartAddress);

	if (dbgEvent->u.CreateProcessInfo.hFile != NULL)
	{
		CloseHandle(dbgEvent->u.CreateProcessInfo.hFile);
	}

	this->_processes[dbgEvent->dwProcessId] = dbgEvent->u.CreateProcessInfo.hProcess;
	this->_threads[dbgEvent->dwThreadId] = dbgEvent->u.CreateThread.hThread;
}

void PDbg::handle_create_thread_debug_event(DEBUG_EVENT * dbgEvent)
{
	printf("Event: Create thread, ThreadID: %u, Thread local base address: %p, Thread start address: %p\n",
		dbgEvent->dwThreadId, dbgEvent->u.CreateThread.lpThreadLocalBase, dbgEvent->u.CreateThread.lpStartAddress);

	this->_threads[dbgEvent->dwThreadId] = dbgEvent->u.CreateThread.hThread;
}

void PDbg::handle_exception_debug_event(DEBUG_EVENT * dbgEvent)
{
	printf("Event: Exception, FirstChance: %u, Address: %p, Code: %x\n", 
		dbgEvent->u.Exception.dwFirstChance, 
		dbgEvent->u.Exception.ExceptionRecord.ExceptionAddress, 
		dbgEvent->u.Exception.ExceptionRecord.ExceptionCode);

	if (!dbgEvent->u.Exception.dwFirstChance)
	{
		TerminateProcess(this->_processes[dbgEvent->dwProcessId], 0);
	}

	ContinueDebugEvent(dbgEvent->dwProcessId, dbgEvent->dwThreadId, DBG_EXCEPTION_NOT_HANDLED);
}

void PDbg::handle_load_dll_debug_event(DEBUG_EVENT * dbgEvent)
{
	printf("Event: DLL loaded, Address: %p\n", dbgEvent->u.LoadDll.lpBaseOfDll);

	if (dbgEvent->u.LoadDll.hFile != NULL)
	{
		CloseHandle(dbgEvent->u.LoadDll.hFile);
	}
}

void PDbg::handle_unload_dll_debug_event(DEBUG_EVENT * dbgEvent)
{
	printf("Event: Unload DLL, Address: %p\n", dbgEvent->u.UnloadDll.lpBaseOfDll);
}

void PDbg::handle_output_debug_string(DEBUG_EVENT * dbgEvent)
{
	printf("Event: Debug Output String, Address: %p, Length: %x", 
		dbgEvent->u.DebugString.lpDebugStringData, 
		dbgEvent->u.DebugString.nDebugStringLength);

	DWORD bytes_read;
	BYTE buffer[2048];
	memset(buffer, '\0', sizeof(buffer));

	if (!ReadProcessMemory(this->_processes[dbgEvent->dwProcessId], dbgEvent->u.DebugString.lpDebugStringData,
		&buffer, dbgEvent->u.DebugString.nDebugStringLength,&bytes_read)
	)
	{
		printf(", Message: cannot read message\n");
	}
	else
	{
		printf(", Message: %s\n", buffer);
	}
}

void PDbg::handle_exit_thread_debug_event(DEBUG_EVENT * dbgEvent)
{
	printf("Event: Exit thread, ThreadId: %u, Exit code: %x\n", dbgEvent->dwThreadId, dbgEvent->u.ExitThread.dwExitCode);

	CloseHandle(this->_threads[dbgEvent->dwThreadId]);

	this->_threads.erase(dbgEvent->dwThreadId);
}

void PDbg::handle_exit_process_debug_event(DEBUG_EVENT * dbgEvent)
{
	printf("Event: Exit process, PID: %u, Exit code: %x\n", dbgEvent->dwProcessId, dbgEvent->u.ExitProcess.dwExitCode);

	CloseHandle(this->_processes[dbgEvent->dwProcessId]);
	CloseHandle(this->_threads[dbgEvent->dwThreadId]);

	this->_processes.erase(dbgEvent->dwProcessId);
	this->_threads.erase(dbgEvent->dwThreadId);
}
