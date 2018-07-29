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
		_bus->error(error_codes::cannot_create_process, GetLastError());

		return false;
	}

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	auto ev = DebuggerStarted();
	_bus->event(ev);
	
	this->listenEvents();

	return true;
}

void Debugger::listenEvents()
{
	do {
		DEBUG_EVENT dbgEvent;
		WaitForDebugEvent(&dbgEvent, INFINITE);

		_currentListenThread = dbgEvent.dwThreadId;
		
		switch (dbgEvent.dwDebugEventCode)
		{
			case CREATE_PROCESS_DEBUG_EVENT: 
				_dbgEventHandler->handle(dbgEvent.u.CreateProcessInfo, dbgEvent.dwProcessId, dbgEvent.dwThreadId); break;
			case EXIT_PROCESS_DEBUG_EVENT:  
				_dbgEventHandler->handle(dbgEvent.u.ExitProcess, dbgEvent.dwProcessId, dbgEvent.dwThreadId); break;
			case CREATE_THREAD_DEBUG_EVENT:  
				_dbgEventHandler->handle(dbgEvent.u.CreateThread, dbgEvent.dwProcessId, dbgEvent.dwThreadId); break;
			case EXIT_THREAD_DEBUG_EVENT:   
				_dbgEventHandler->handle(dbgEvent.u.ExitThread, dbgEvent.dwProcessId, dbgEvent.dwThreadId); break;
			case LOAD_DLL_DEBUG_EVENT:      
				_dbgEventHandler->handle(dbgEvent.u.LoadDll, dbgEvent.dwProcessId, dbgEvent.dwThreadId); break;
			case UNLOAD_DLL_DEBUG_EVENT:     
				_dbgEventHandler->handle(dbgEvent.u.UnloadDll, dbgEvent.dwProcessId, dbgEvent.dwThreadId); break;
			case OUTPUT_DEBUG_STRING_EVENT:  
				_dbgEventHandler->handle(dbgEvent.u.DebugString, dbgEvent.dwProcessId, dbgEvent.dwThreadId); break;
			case EXCEPTION_DEBUG_EVENT:      
				_dbgEventHandler->handle(dbgEvent.u.Exception, dbgEvent.dwProcessId, dbgEvent.dwThreadId); break;
		}
		ContinueDebugEvent(dbgEvent.dwProcessId, dbgEvent.dwThreadId, DBG_CONTINUE);
	} while (_rmManager->anyProcessExist());
}

bool Debugger::setSingleStep(DWORD threadId, bool raiseEvent)
{
	if (!_stepper->setSignleStep(threadId)) {

		_bus->error(error_codes::cannot_set_single_step);

		return false;
	}

	auto ev = SingleStepSet();
	ev.threadId = static_cast<unsigned long>(threadId);
	_bus->event(ev);

	return true;
}

bool Debugger::jumpNextInstruction()
{
	this->setSingleStep(_currentListenThread);

	CONTEXT ctx;
	memset(&ctx, 0, sizeof(ctx));

	ctx.ContextFlags = CONTEXT_CONTROL;

	GetThreadContext(_rmManager->getThread(_currentListenThread), &ctx);

	return true;
}


bool Debugger::addBreakpoint(LPVOID address, HANDLE hProcess, DWORD threadId)
{
	if (_bpManager->add(address, hProcess, threadId)) {
		auto ev = BreakpointAdded();
		_bus->event(ev);

		return true;
	}

	_bus->error(error_codes::cannot_add_breakpoint, GetLastError());
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
	return _rmManager->getAllThreadPids();
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