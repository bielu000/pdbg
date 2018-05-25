#include "stdafx.h"
#include "PDbg.h"
#include <iostream>
#include <WinBase.h>
#include <Psapi.h>

#pragma comment(lib, "psapi.lib")

bool PDbg::StartDebugActiveProcess(DWORD processId)
{
	if (!DebugActiveProcess(processId))
	{
		std::cout << "Cannot debug active process with id: " << processId << std::endl;

		return false;
	}

	this->_startupProcessId = processId;

	this->run();

	return true;
}

bool PDbg::StartDebugNewProcess(LPTSTR processName)
{
	PROCESS_INFORMATION pi;
	STARTUPINFO si;

	ZeroMemory(&pi, sizeof(pi));
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);

	if (!CreateProcess(processName, NULL,
		NULL, NULL, FALSE,
		DEBUG_ONLY_THIS_PROCESS | CREATE_NEW_CONSOLE,
		NULL, NULL, &si, &pi))
	{
		std::cout << "Cannot create process" << std::endl;

		return FALSE;
	}

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	this->_startupProcessId = pi.dwProcessId;
	this->run();
}

bool PDbg::Shutdown()
{
	DebugActiveProcessStop(this->_startupProcessId);

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

	this->AddBreakpoint((LPVOID)dbgEvent->u.CreateProcessInfo.lpStartAddress, this->_processes[dbgEvent->dwProcessId], NULL);
	
	this->_image_base = dbgEvent->u.CreateProcessInfo.lpBaseOfImage;
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


	LPVOID excp_address = dbgEvent->u.Exception.ExceptionRecord.ExceptionAddress;
	DWORD con_status = DBG_EXCEPTION_NOT_HANDLED;

	switch (dbgEvent->u.Exception.ExceptionRecord.ExceptionCode)
	{
		case EXCEPTION_BREAKPOINT:
			std::cout << "EXCEPTION_BREAKPOINT" << std::endl;

			if (this->_breakpoints.find(excp_address) != this->_breakpoints.end()) // user defined breakpoint
			{
				if (this->_breakpoints[excp_address]->handler != NULL) 
				{
					this->_breakpoints[excp_address]->handler(this->_breakpoints[excp_address],dbgEvent, &con_status);
				}

				this->SetThreadTrapFlag(this->_threads[dbgEvent->dwThreadId]);
			
				this->_pending_breakpoints[dbgEvent->dwThreadId] = excp_address;
			
				this->RemoveBreakpoint(dbgEvent->u.Exception.ExceptionRecord.ExceptionAddress, this->_processes[dbgEvent->dwProcessId]);
			}
			else // default kernel exception	
			{
				std::cout << "Default process exception. SizeOfImage captured." << std::endl;

				MODULEINFO module_info;	

				GetModuleInformation(this->_processes[dbgEvent->dwProcessId], (HMODULE)this->_image_base, &module_info, sizeof(module_info));
				
				this->_image_size = module_info.SizeOfImage;
				con_status = DBG_CONTINUE;
				this->SetThreadTrapFlag(this->_threads[dbgEvent->dwThreadId]);
			}


		break;

		case STATUS_SINGLE_STEP:
			if (this->_pending_breakpoints.find(dbgEvent->dwThreadId) != this->_pending_breakpoints.end())
			{
				std::cout << "Single step. Pending..." << std::endl;
				this->AddBreakpoint(this->_pending_breakpoints[dbgEvent->dwThreadId], this->_processes[dbgEvent->dwProcessId], NULL);
				this->_processes.erase(dbgEvent->dwThreadId);
			}
		break;
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

bool PDbg::AddBreakpoint(LPVOID address, HANDLE hProcess, PBreakpointHandler pbreakpoint_handler)
{
	if (this->_breakpoints.find(address) != this->_breakpoints.end())
	{
		return TRUE;
	}

	BYTE byte; 
	SIZE_T bytes_read;

	if (!ReadProcessMemory(hProcess, address, &byte, sizeof(BYTE), &bytes_read))
	{
		std::cout << "Cannot read instruction byte.";
		std::cout << "Error " << GetLastError() << std::endl;

		return FALSE;
	}

	BYTE int_3_instr = 0xcc;
	
	SIZE_T bytes_written;
	if (!WriteProcessMemory(hProcess, address, &int_3_instr, sizeof(BYTE), &bytes_written))
	{
		std::cout << "Cannot write debug instruction byte." << std::endl;

		return FALSE;
	}

	printf("Instruckaj %#010x\n", byte);

	auto bp = new Breakpoint;
	bp->address = address;
	bp->byte = byte;
	bp->handler = pbreakpoint_handler;
	
	this->_breakpoints[address] = bp;
}

bool PDbg::RemoveBreakpoint(LPVOID address, HANDLE hProcess)
{
	if (this->_breakpoints.find(address) == this->_breakpoints.end())
	{
		return FALSE;
	}

	SIZE_T bytes_processed;
	auto* breakpoint = this->_breakpoints[address];
	
	if (!WriteProcessMemory(hProcess, address, &breakpoint->byte, sizeof(BYTE), &bytes_processed))
	{
		std::cout << "Remove breakpoint: Cannot write original byte." << std::endl;
		return FALSE;
	}

	delete this->_breakpoints[address];
	this->_breakpoints.erase(address);

	return TRUE;
}

bool PDbg::SetThreadTrapFlag(HANDLE hThread)
{
	const unsigned int k86trapflag = (1 << 8);
	CONTEXT ctx;
	memset(&ctx, 0, sizeof(ctx));
	
	ctx.ContextFlags = CONTEXT_CONTROL;
	
	if (!GetThreadContext(hThread, &ctx))
	{
		std::cout << "Cannot get thread context." << std::endl;

		return FALSE;
	}

	ctx.EFlags |= k86trapflag;

	if (!SetThreadContext(hThread, &ctx))
	{
		std::cout << "Cannot set thread context." << std::endl;
		
		return FALSE;
	}

	return TRUE;
}