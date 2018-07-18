#include "stdafx.h"
#include <Windows.h>
#include "Debugger.h"
#include <map>
#include <iostream>
#include <sstream>

bool Debugger::run(std::string application)
{	
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));

	si.cb = sizeof(si);

	auto app = std::wstring(application.begin(), application.end());

	if (!CreateProcess(app.c_str(), NULL,
		NULL, NULL, FALSE,
		DEBUG_ONLY_THIS_PROCESS | CREATE_NEW_CONSOLE,
		NULL, NULL, &si, &pi))
	{
		onError(DebuggerErrorOccurred { "Additional info in system codes", static_cast<unsigned long>(GetLastError())});

		return false;
	}

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	onStarted(DebuggerStarted{ "Debugger Started correctly." });
	
	this->listenEvents();

	return true;
}

void Debugger::listenEvents()
{
	do {
		DEBUG_EVENT dbgEvent;
		WaitForDebugEvent(&dbgEvent, INFINITE);

		switch (dbgEvent.dwDebugEventCode)
		{
			case CREATE_PROCESS_DEBUG_EVENT: handle_process_created(dbgEvent);				break;
			case EXIT_PROCESS_DEBUG_EVENT:   handle_process_exited(dbgEvent);				break;
			case CREATE_THREAD_DEBUG_EVENT:  handle_thread_created(dbgEvent);				break;
			case EXIT_THREAD_DEBUG_EVENT:    handle_thread_exited(dbgEvent);				break;
			case LOAD_DLL_DEBUG_EVENT:       handle_dll_loaded(dbgEvent);					break;
			case UNLOAD_DLL_DEBUG_EVENT:     handle_dll_unloaded(dbgEvent);					break;
			case OUTPUT_DEBUG_STRING_EVENT:  handle_debug_output_string_received(dbgEvent); break;
			case EXCEPTION_DEBUG_EVENT:      handle_exception_thrown(dbgEvent);				break;
		}
		ContinueDebugEvent(dbgEvent.dwProcessId, dbgEvent.dwThreadId, DBG_CONTINUE);
	} while (!_processes.empty());
}

bool Debugger::setSingleStep(DWORD threadId)
{
	if (_threads.find(threadId) == _threads.end()) {
		std::stringstream ss;
		ss << "Thread with id: " << threadId << " does not exist.";

		auto ev = DebuggerErrorOccurred();
		ev.message = ss.str();

		onError(ev);

		return false;
	}

	CONTEXT ctx;
	unsigned int cTrapFlag = (1 << 8); // EFLAGS registry
	memset(&ctx, 0, sizeof(ctx));
	ctx.ContextFlags = CONTEXT_CONTROL;
	
	if (!GetThreadContext(_threads[threadId], &ctx)) {
		std::stringstream ss;
		ss << "Cannot get context for thread: " << threadId;

		auto ev = DebuggerErrorOccurred();
		ev.message = ss.str();
		ev.systemErrorCode = static_cast<unsigned long>(GetLastError());

		onError(ev);

		return false;
	}

	ctx.EFlags |= cTrapFlag;

	if (!SetThreadContext(_threads[threadId], &ctx)) {
		std::stringstream ss;
		ss << "Cannot set context for thread: " << threadId; 

		auto ev = DebuggerErrorOccurred();
		ev.message = ss.str();
		ev.systemErrorCode = static_cast<unsigned long>(GetLastError());

		onError(ev);

		return false;
	}

	return true;
}

bool Debugger::addBreakpoint(LPVOID address, HANDLE hProcess)
{
	BYTE originalInstruction;
	SIZE_T bytesRead;
	
	if (!ReadProcessMemory(hProcess, address, &originalInstruction, sizeof(BYTE), &bytesRead)) {
		std::cout << "Cannot read process mem. Refactor breakpoint" << std::endl;
		
		return false;
	}

	BYTE int3optcode = 0xCC;
	SIZE_T bytesWritten;

	if (!WriteProcessMemory(hProcess, address, &int3optcode, sizeof(BYTE), &bytesWritten)) {
		std::cout << "Cannot write process mem. Reafactor breakpoint" << std::endl;

		return false;
	}

	_breakpoints->add(address, originalInstruction);
	printf("Dodano breakpoint, Instrukcja: %#08x, adres: %#08x\n", originalInstruction, address);
	return true;
}

bool Debugger::removeBreakpoint(LPVOID address, HANDLE hProcess)
{
	//add error messages
	if (!_breakpoints->exist(address)) {
		std::cout << "Breakpoint not exist!!! removeBreakpoint" << std::endl;
		return false;
	}

	auto bp = _breakpoints->get(address);
	SIZE_T bytes_written;
	BYTE optcode = bp->instruction();

	if (!WriteProcessMemory(hProcess, bp->address(), &optcode, sizeof(BYTE), &bytes_written)) {
		std::cout << "Cannot write memory, remove breakpiont" << std::endl;

		return false;
	}

	printf("Usunieto breakpoint, Instrukcja: %#08x, adres: %#08x\n", optcode, address);
	_breakpoints->remove(address);

	return true;
}

void Debugger::handle_process_created(DEBUG_EVENT& dbgEvent)
{
	_processes[dbgEvent.dwProcessId] = dbgEvent.u.CreateProcessInfo.hProcess;
	_threads[dbgEvent.dwThreadId] = dbgEvent.u.CreateProcessInfo.hThread;

	auto ev = ProcessCreated();
	ev.message = "Process created";
	ev.processId = static_cast<unsigned long>(dbgEvent.dwProcessId);
	ev.baseAddress = reinterpret_cast<unsigned long*>(dbgEvent.u.CreateProcessInfo.lpBaseOfImage);

	if (dbgEvent.u.CreateProcessInfo.hFile != NULL) {
		CloseHandle(dbgEvent.u.CreateProcessInfo.hFile);
	}

	this->addBreakpoint(dbgEvent.u.CreateProcessInfo.lpStartAddress, dbgEvent.u.CreateProcessInfo.hProcess);

	onProcessCreated(ev);
}

void Debugger::handle_process_exited(DEBUG_EVENT& dbgEvent)
{
	//CloseHandle(_processes[dbgEvent.dwProcessId]);

	_processes.erase(dbgEvent.dwProcessId);
	auto ev = ProcessExited();
	ev.message = "Process finished. Handle closed.";
	ev.processId = static_cast<unsigned long>(dbgEvent.dwProcessId);
	ev.exitCode = static_cast<unsigned long>(dbgEvent.u.ExitProcess.dwExitCode);

	onProcessExited(ev);
}

void Debugger::handle_thread_created(DEBUG_EVENT & dbgEvent)
{
	_threads[dbgEvent.dwThreadId] = dbgEvent.u.CreateThread.hThread;
	
	auto ev = ThreadCreated();
	ev.message = "Thread created successfully";
	ev.threadId = static_cast<unsigned long>(dbgEvent.dwThreadId);

	onThreadCreated(ev);
}

void Debugger::handle_thread_exited(DEBUG_EVENT& dbgEvent)
{
	if (_threads[dbgEvent.dwThreadId] != NULL)
	{
		//CloseHandle(_threads[dbgEvent.dwThreadId]); somehting is wrong
	}

	_threads.erase(dbgEvent.dwThreadId);

	auto ev = ThreadExited();
	ev.message = "Thread exited";
	ev.threadId = static_cast<unsigned long>(dbgEvent.dwThreadId);
	ev.exitCode = static_cast<unsigned long>(dbgEvent.u.ExitThread.dwExitCode);

	onThreadExited(ev);
}

void Debugger::handle_dll_loaded(DEBUG_EVENT& dbgEvent)
{
	if (dbgEvent.u.LoadDll.hFile != NULL)
	{
		CloseHandle(dbgEvent.u.LoadDll.hFile);
	}

	auto ev = DllLoaded();
	ev.message = "Dll loaded";

	onDllLoaded(ev);
}

void Debugger::handle_dll_unloaded(DEBUG_EVENT& dbgEvent)
{
	auto ev = DllUnloaded();
	ev.message = "Dll unloaded";

	onDllUnloaded(ev);
}

void Debugger::handle_debug_output_string_received(DEBUG_EVENT& dbgEvent)
{
	DWORD bytes_read;
	BYTE buffer[2048];
	 
	ReadProcessMemory(_processes[dbgEvent.dwProcessId], dbgEvent.u.DebugString.lpDebugStringData, 
		buffer, dbgEvent.u.DebugString.nDebugStringLength, &bytes_read);

	auto ev = OutputDebugStringReveived();
	ev.message = "Output string received";
	ev.value = std::string(reinterpret_cast<char*>(buffer)); //refactor not safe

	onOutputStringReceived(ev);
}

void Debugger::handle_exception_thrown(DEBUG_EVENT& dbgEvent)
{
	std::cout << "First chance: " << dbgEvent.u.Exception.dwFirstChance << std::endl;

	if (!dbgEvent.u.Exception.dwFirstChance) {
		TerminateProcess(_processes[dbgEvent.dwProcessId], 1);
		std::cout << "Terminate process!" << std::endl;
		return;
	}

	DWORD debug_continue_status = DBG_EXCEPTION_NOT_HANDLED;

	switch (dbgEvent.u.Exception.ExceptionRecord.ExceptionCode)
	{
		case EXCEPTION_SINGLE_STEP:
			debug_continue_status = DBG_CONTINUE;

			if (_pendingBreakpoints->exist(dbgEvent.u.Exception.ExceptionRecord.ExceptionAddress)) {
				this->addBreakpoint(dbgEvent.u.Exception.ExceptionRecord.ExceptionAddress, _processes[dbgEvent.dwProcessId]);
				this->setSingleStep(dbgEvent.dwThreadId);
				_pendingBreakpoints->remove(dbgEvent.u.Exception.ExceptionRecord.ExceptionAddress);
			}

			std::cout << "Single step!!!!" << std::endl;

			break;
		case EXCEPTION_BREAKPOINT:
			debug_continue_status = DBG_CONTINUE;
			std::cout << "Breakpoint!" << std::endl;

			if (_breakpoints->exist(dbgEvent.u.Exception.ExceptionRecord.ExceptionAddress)) {
				this->removeBreakpoint(dbgEvent.u.Exception.ExceptionRecord.ExceptionAddress, _processes[dbgEvent.dwProcessId]);
				this->setSingleStep(dbgEvent.dwThreadId);
				_pendingBreakpoints->add(dbgEvent.u.Exception.ExceptionRecord.ExceptionAddress);

				CONTEXT ctx;
				memset(&ctx, 0, sizeof(ctx));
				ctx.ContextFlags = CONTEXT_CONTROL;

				if (!GetThreadContext(_threads[dbgEvent.dwThreadId], &ctx)) {
					std::cout << "Cannot get thread ctx in breakpoint." << std::endl;

					return;
				}

				ctx.Eip--;

				if (!SetThreadContext(_threads[dbgEvent.dwThreadId], &ctx)) {
					std::cout << "Cannot set thread context. in EXCEPTION BREAPOINT" << std::endl;

					return;
				}
			}

			break;
	}

	ContinueDebugEvent(dbgEvent.dwProcessId, dbgEvent.dwThreadId, debug_continue_status);
}


/**
	dopoki procesy.liczba > 0 :
		czekaj na zdarzenie deubggera
		obsluz zdarzenie:
			- utworzony proces
			- utworzony watek
			- dll zaladowany
			- dll odladowany
			- wyjatek:
				- jezeli exception breakpoint 
				  - sprawdz czy istnieje taki breakpoint:
				    - jezeli tak to dodaj do pending
					- jezeli tak to usun breakpoint i przywroc instrukcje
					- ustaw single step
					- ustaw status jako DBG_CONTINUE - info nizej
				- jezeli single step:
				  - sprawdz czy jest jakis pending breakpoint, 
				  - jezeli tak to przywroc go
				  - ustaw single step 
				  - ustaw status jako DBG_CONTINUE - info nizej
				- jezeli to wyjatek z kodem firtschance:
				  - ustaw status jako EXCEPTION_NOT_HANDLED tak aby apka mogla obsluzyc sobie wyjatek
			    - jezeli nie first chance 
				  - apka nie obsluzyla wyjatkui, nie ma procedury do tego, albo wyjatek szedl w gore i nic go nie obsluzylo
				  - ubij procecs
		kontynuuj nasluchiwanie na zdarzenia

*/

/*
W przypadku EXCEPTION_BREAKPOINT traktuj wszystkie wyjatki z gory jako EXCEPTION_NOT_HANDLED, bo trzeba dac mozliwosc programowi
obsluzyc dany wyjatek. Popatrz:
try {
	service->doIt() -> tutaj zostal rzucony wyjatek, jeeli ustawisz status DB_CONTINUE to blok catch nie zostanie obslozony, a to zle	
} catch (const std::exception& e) {
	log(e.message()); kiedy ustawisz continue status jako not_handled, ten kod sie wykona
	Stopapp 
	... 
}

W przypadku jezeli wyjatek jest typu EXCEPTION_BREAKPOINT albo SINGLE_STEP to trzeba ustawic status jako DBG_CONTINUE, a
to dlatego, zeby dac info do apki, ze oblsuga wyjatku jest juz zrobiona i zeby apka kontynuowala sobie wykonanie.
Innymi slowy - ten wyjatek zdarzyl sie tylko przez ingerencje debugera, dlatego debuger musi wziac za niego odpowiedzialnosc i
status ustawic jako dbg continuer
*/