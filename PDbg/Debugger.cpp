#include "stdafx.h"
#include <Windows.h>
#include "Debugger.h"
#include <map>
#include <iostream>
#include <sstream>
#include "EventCodes.h"
#include "ErrorCodes.h"

//DEBUGGER FUNCIONALITY

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
		auto ev = DebuggerErrorOccurred();
		ev.systemErrorCode = static_cast<unsigned long>(GetLastError());
		onError(ev);

		return false;
	}

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	auto ev = DebuggerStarted();
	onStarted(ev);
	
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

bool Debugger::setSingleStep(DWORD threadId, bool raiseEvent)
{
	if (_threads.find(threadId) == _threads.end()) {
		auto ev = DebuggerErrorOccurred();
		ev.debuggerErrorCode = error_codes::thread_not_exits;
		onError(ev);

		return false;
	}

	CONTEXT ctx;
	unsigned int cTrapFlag = (1 << 8); // EFLAGS registry
	memset(&ctx, 0, sizeof(ctx));
	ctx.ContextFlags = CONTEXT_CONTROL;
	
	if (!GetThreadContext(_threads[threadId], &ctx)) {
		auto ev = DebuggerErrorOccurred();
		ev.systemErrorCode = static_cast<unsigned long>(GetLastError());
		ev.debuggerErrorCode = error_codes::cannot_set_context;

		onError(ev);

		return false;
	}

	ctx.EFlags |= cTrapFlag;

	if (!SetThreadContext(_threads[threadId], &ctx)) { 
		auto ev = DebuggerErrorOccurred();
		ev.debuggerErrorCode = error_codes::cannot_set_context;
		ev.systemErrorCode = static_cast<unsigned long>(GetLastError());
		onError(ev);

		return false;
	}

	if (raiseEvent) {
		auto ev = SingleStepSet();
		ev.threadId = static_cast<unsigned long>(threadId);
		onSingleStepSet(ev);
	}

	return true;
}

bool Debugger::addBreakpoint(LPVOID address, HANDLE hProcess, DWORD threadId)
{
	if (_bpManager->add(address, hProcess, threadId)) {
		auto ev = BreakpointAdded();
		onBreakpointAdded(ev);

		return true;
	}

	auto ev = DebuggerErrorOccurred();
	ev.eventCode = error_codes::cannot_add_breakpoint;
	ev.systemErrorCode = GetLastError();

	onError(ev);
	return false;
}

bool Debugger::removeBreakpoint(LPVOID address, HANDLE hProcess)
{
	return true;
}

std::vector<std::shared_ptr<nBreakpoint>> Debugger::getBreakpoints()
{
	return _bpManager->getAll();
}

std::vector<DWORD> Debugger::getThreads()
{
	std::vector<DWORD> v;

	for (auto &x : this->_threads) {
		v.push_back(x.first);
	}

	return v;
}





/// DEBUG EVENTS 

void Debugger::handle_process_created(DEBUG_EVENT& dbgEvent)
{
	_processes[dbgEvent.dwProcessId] = dbgEvent.u.CreateProcessInfo.hProcess;
	_threads[dbgEvent.dwThreadId] = dbgEvent.u.CreateProcessInfo.hThread;

	auto ev = ProcessCreated();
	ev.processId = static_cast<unsigned long>(dbgEvent.dwProcessId);
	ev.baseAddress = reinterpret_cast<unsigned long*>(dbgEvent.u.CreateProcessInfo.lpBaseOfImage);

	if (dbgEvent.u.CreateProcessInfo.hFile != NULL) {
		CloseHandle(dbgEvent.u.CreateProcessInfo.hFile);
	}

	this->addBreakpoint(dbgEvent.u.CreateProcessInfo.lpStartAddress, dbgEvent.u.CreateProcessInfo.hProcess, dbgEvent.dwThreadId);

	onProcessCreated(ev);
}


void Debugger::handle_process_exited(DEBUG_EVENT& dbgEvent)
{
	//CloseHandle(_processes[dbgEvent.dwProcessId]); //here is something wrong

	_processes.erase(dbgEvent.dwProcessId);
	auto ev = ProcessExited();
	ev.processId = static_cast<unsigned long>(dbgEvent.dwProcessId);
	ev.exitCode = static_cast<unsigned long>(dbgEvent.u.ExitProcess.dwExitCode);
	
	onProcessExited(ev);
}

void Debugger::handle_thread_created(DEBUG_EVENT & dbgEvent)
{
	_threads[dbgEvent.dwThreadId] = dbgEvent.u.CreateThread.hThread;
	
	auto ev = ThreadCreated();
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
	onDllLoaded(ev);
}

void Debugger::handle_dll_unloaded(DEBUG_EVENT& dbgEvent)
{
	auto ev = DllUnloaded();
	onDllUnloaded(ev);
}

void Debugger::handle_debug_output_string_received(DEBUG_EVENT& dbgEvent)
{
	DWORD bytes_read;
	BYTE buffer[2048];
	 
	if (!ReadProcessMemory(_processes[dbgEvent.dwProcessId], dbgEvent.u.DebugString.lpDebugStringData,
		buffer, dbgEvent.u.DebugString.nDebugStringLength, &bytes_read))
	{
		auto ev = DebuggerErrorOccurred();
		ev.debuggerErrorCode = error_codes::cannot_read_process_memory;
		onError(ev);

		return;
	}

	auto ev = OutputDebugStringReveived();
	ev.value = std::string(reinterpret_cast<char*>(buffer)); //refactor not safe

	onOutputStringReceived(ev);
}

void Debugger::handle_exception_thrown(DEBUG_EVENT& dbgEvent)
{
	if (!dbgEvent.u.Exception.dwFirstChance) {
		TerminateProcess(_processes[dbgEvent.dwProcessId], 1);
		auto ev = ProcessTerminated();
		onProcessTerminated(ev);

		return;
	}

	DWORD debug_continue_status = DBG_EXCEPTION_NOT_HANDLED;

	switch (dbgEvent.u.Exception.ExceptionRecord.ExceptionCode)
	{
	case EXCEPTION_BREAKPOINT:
	{
		debug_continue_status = DBG_CONTINUE;

		if (_bpManager->exist(dbgEvent.u.Exception.ExceptionRecord.ExceptionAddress)) {
			if (!_bpManager->restoreOriginalByte(dbgEvent.u.Exception.ExceptionRecord.ExceptionAddress, _processes[dbgEvent.dwProcessId])) {
				auto ev = DebuggerErrorOccurred();
				ev.debuggerErrorCode = error_codes::cannot_restore_original_byte;
				ev.systemErrorCode = GetLastError();
				onError(ev);
				
				break;
			}
			this->setSingleStep(dbgEvent.dwThreadId, false);

			CONTEXT ctx;
			memset(&ctx, 0, sizeof(ctx));
			ctx.ContextFlags = CONTEXT_CONTROL;

			if (!GetThreadContext(_threads[dbgEvent.dwThreadId], &ctx)) {
				auto ev = DebuggerErrorOccurred();
				ev.debuggerErrorCode = error_codes::cannot_get_context;
				onError(ev);

				return;
			}

			ctx.Eip--;

			if (!SetThreadContext(_threads[dbgEvent.dwThreadId], &ctx)) {
				auto ev = DebuggerErrorOccurred();
				ev.debuggerErrorCode = error_codes::cannot_set_context;
				onError(ev);

				return;
			}

			auto ev = BreakpointExceptionOccured();
			ev.firstChance = static_cast<unsigned long>(dbgEvent.u.Exception.dwFirstChance);
			onBreakpointExceptionOccured(ev);
		}
	}
	break;
		case EXCEPTION_SINGLE_STEP:
		{
			debug_continue_status = DBG_CONTINUE;

			if (_bpManager->existAnyPending(dbgEvent.dwThreadId)) {
				if (!_bpManager->restorePending(dbgEvent.dwThreadId, _processes[dbgEvent.dwProcessId])) {
					auto ev = DebuggerErrorOccurred();
					ev.debuggerErrorCode = error_codes::cannot_restore_pending_breakpoint;
					ev.systemErrorCode = GetLastError();
					onError(ev);
					break;
				}
				this->setSingleStep(dbgEvent.dwThreadId, false);
				this->_notifySingleStep = false;
				break;
			}

			if (this->_notifySingleStep) {
				auto ev = SingleStepExceptionOccured();
				ev.firstChance = static_cast<unsigned long>(dbgEvent.u.Exception.dwFirstChance);
				onSingleStepExceptionOccured(ev);
			}
		}
			break;

		default:
			auto ev = UsualExceptionOccured();
			ev.firstChance = static_cast<unsigned long>(dbgEvent.u.Exception.dwFirstChance);
			onUsualExceptionOccured(ev);
		break;
	}

	ContinueDebugEvent(dbgEvent.dwProcessId, dbgEvent.dwThreadId, debug_continue_status);
}


/**
	Do zrobienia :
		- lista watkow,
		- dodawanie braekpointa
		- usuwanie breakpointa 
		- lista breakpointow
		- ustawianie trap flagi
		- deasemblaccja kodu 


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