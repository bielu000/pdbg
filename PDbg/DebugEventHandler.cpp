#include "stdafx.h"
#include "DebugEventHandler.h"
#include <iostream>
#include "DebuggEvents.h"
#include "ErrorCodes.h"

void DebugEventHandler::handle(CREATE_PROCESS_DEBUG_INFO& dbgEvent, DWORD processId, DWORD threadId)
{
	_rmManager->addProcess(processId, dbgEvent.hProcess);
	_rmManager->addThread(threadId, dbgEvent.hThread);

	if (dbgEvent.hFile != NULL) {
		CloseHandle(dbgEvent.hFile);
	}

	auto ev = ProcessCreated();
	ev.processId = static_cast<unsigned long>(processId);
	ev.baseAddress = reinterpret_cast<unsigned long*>(dbgEvent.lpBaseOfImage);

	std::cout << "Start address" << dbgEvent.lpStartAddress << std::endl;

	_bpManager->add(dbgEvent.lpStartAddress, dbgEvent.hProcess, threadId);

	_bus->event(ev);
}

void DebugEventHandler::handle(EXIT_PROCESS_DEBUG_INFO& dbgEvent, DWORD processId, DWORD threadId)
{
	//CloseHandle(_processes[dbgEvent.dwProcessId]); //here is something wrong

	_rmManager->removeProcess(processId);
	auto ev = ProcessExited();
	ev.processId = static_cast<unsigned long>(processId);
	ev.exitCode = static_cast<unsigned long>(dbgEvent.dwExitCode);
	
	_bus->event(ev);
}


void DebugEventHandler::handle(CREATE_THREAD_DEBUG_INFO& dbgEvent, DWORD processId, DWORD threadId)
{
	_rmManager->addThread(threadId, dbgEvent.hThread);

	auto ev = ThreadCreated();
	ev.threadId = static_cast<unsigned long>(threadId);
	
	_bus->event(ev);
}

void DebugEventHandler::handle(EXIT_THREAD_DEBUG_INFO& dbgEvent, DWORD processId, DWORD threadId)
{
	auto handle = _rmManager->getThread(threadId);

	if (handle != NULL)
	{
		//CloseHandle(_threads[dbgEvent.dwThreadId]); somehting is wrong
	}

	_rmManager->removeThread(threadId);

	auto ev = ThreadExited();
	ev.threadId = static_cast<unsigned long>(threadId);
	ev.exitCode = static_cast<unsigned long>(dbgEvent.dwExitCode);

	_bus->event(ev);
}

void DebugEventHandler::handle(LOAD_DLL_DEBUG_INFO& dbgEvent, DWORD processId, DWORD threadId)
{
	if (dbgEvent.hFile != NULL)
	{
		CloseHandle(dbgEvent.hFile);
	}

	auto ev = DllLoaded();
	_bus->event(ev);
}

void DebugEventHandler::handle(UNLOAD_DLL_DEBUG_INFO& dbgEvent, DWORD processId, DWORD threadId)
{
	auto ev = DllUnloaded();
	
	_bus->event(ev);
}

void DebugEventHandler::handle(OUTPUT_DEBUG_STRING_INFO& dbgEvent, DWORD processId, DWORD threadId)
{
	DWORD bytes_read;
	BYTE buffer[2048];

	if (!ReadProcessMemory(_rmManager->getProcess(processId), dbgEvent.lpDebugStringData,
		buffer, dbgEvent.nDebugStringLength, &bytes_read))
	{
		_bus->error(error_codes::cannot_read_process_memory, GetLastError());

		return;
	}

	auto ev = OutputDebugStringReveived();
	ev.value = std::string(reinterpret_cast<char*>(buffer)); //refactor not safe

	_bus->event(ev);
}

void DebugEventHandler::handle(EXCEPTION_DEBUG_INFO& dbgEvent, DWORD processId, DWORD threadId)
{
	if (!dbgEvent.dwFirstChance) {
		TerminateProcess(_rmManager->getProcess(processId), 1);
		auto ev = ProcessTerminated();
		_bus->event(ev);

		return;
	}

	DWORD debug_continue_status = DBG_EXCEPTION_NOT_HANDLED;

	switch (dbgEvent.ExceptionRecord.ExceptionCode)
	{
	case EXCEPTION_BREAKPOINT:
	{
		debug_continue_status = DBG_CONTINUE;

		if (_bpManager->exist(dbgEvent.ExceptionRecord.ExceptionAddress)) {
			if (!_bpManager->restoreOriginalByte(dbgEvent.ExceptionRecord.ExceptionAddress, _rmManager->getProcess(processId))) {
				_bus->error(error_codes::cannot_restore_original_byte, GetLastError());

				break;
			}

			_stepper->setSignleStep(threadId);

			CONTEXT ctx;
			memset(&ctx, 0, sizeof(ctx));
			ctx.ContextFlags = CONTEXT_CONTROL;

			if (!GetThreadContext(_rmManager->getThread(threadId), &ctx)) {
				_bus->error(error_codes::cannot_get_context, GetLastError());

				return;
			}

			ctx.Eip--;

			if (!SetThreadContext(_rmManager->getThread(threadId), &ctx)) {
				_bus->error(error_codes::cannot_set_context, GetLastError());

				return;
			}

			//auto ev = BreakpointExceptionOccured();
			//ev.threadId = threadId;
			//ev.firstChance = static_cast<unsigned long>(dbgEvent.dwFirstChance);
			//_bus->event(ev);
		}

	}
	break;
	case EXCEPTION_SINGLE_STEP:
	{
		debug_continue_status = DBG_CONTINUE;

		if (_bpManager->existAnyPending(threadId)) {
			if (!_bpManager->restorePending(threadId, _rmManager->getProcess(processId))) {
				_bus->error(error_codes::cannot_restore_pending_breakpoint, GetLastError());
				break;
			}

			auto ev = BreakpointExceptionOccured();
			ev.threadId = threadId;
			ev.firstChance = static_cast<unsigned long>(dbgEvent.dwFirstChance);
			_bus->event(ev);

			//_stepper->setSignleStep(threadId);
			break;
		}

		auto ev = SingleStepExceptionOccured();
		ev.firstChance = static_cast<unsigned long>(dbgEvent.dwFirstChance);
		_bus->event(ev);

	}
	break;

	default:
		auto ev = UsualExceptionOccured();
		ev.firstChance = static_cast<unsigned long>(dbgEvent.dwFirstChance);	
		_bus->event(ev);

		break;
	}

	ContinueDebugEvent(processId, threadId, debug_continue_status);
}