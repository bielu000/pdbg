#pragma once
#include <Windows.h>
#include <map>
#include <memory>
#include "Signal.h"
#include "DebuggEvents.h"
#include "Breakpoint.h"
#include "BreakpointManager.h"

class Debugger
{
public:
	Debugger() = default;
	bool run(std::string application);
	bool setSingleStep(DWORD threadId, bool raiseEvent = true);
	bool addBreakpoint(LPVOID address, HANDLE hProcess, DWORD threadId);
	bool removeBreakpoint(LPVOID address, HANDLE hProcess);

	//Does this is not going outside scope? Think through
	//DEBUGGER FUNCTIONALITY EVENTS
	signals::signal<void(const DebuggerStarted&)> onStarted;
	signals::signal<void(const DebuggerErrorOccurred&)> onError;
	signals::signal<void(const SingleStepSet&)> onSingleStepSet;
	signals::signal<void(const BreakpointAdded&)> onBreakpointAdded;
	signals::signal<void(const BreakpointRemoved&)> onBreakpointRemoved;
	signals::signal<void(const ProcessTerminated&)> onProcessTerminated;
	
	
	//DEBUG EVENTS
	signals::signal<void(const ProcessCreated&)> onProcessCreated;
	signals::signal<void(const ProcessExited&)> onProcessExited;
	signals::signal<void(const ThreadCreated&)> onThreadCreated;
	signals::signal<void(const ThreadExited&)> onThreadExited;
	signals::signal<void(const DllLoaded&)> onDllLoaded;
	signals::signal<void(const DllUnloaded&)> onDllUnloaded;
	signals::signal<void(const OutputDebugStringReveived&)> onOutputStringReceived;
	signals::signal<void(const SingleStepExceptionOccured&)> onSingleStepExceptionOccured;
	signals::signal<void(const BreakpointExceptionOccured&)> onBreakpointExceptionOccured;
	signals::signal<void(const UsualExceptionOccured&)> onUsualExceptionOccured;
	
	~Debugger() = default;

private:
	void listenEvents();

	std::unique_ptr<BreakpointManager> _bpManager = std::make_unique<BreakpointManager>();
	std::map<DWORD, HANDLE> _processes; //posiibly to replace with struct
	std::map<DWORD, HANDLE> _threads; // the same as above
	bool _notifySingleStep = FALSE;
	DWORD _debugContinueStatus = DBG_CONTINUE;

	//handes
	void handle_process_created(DEBUG_EVENT& dbgEvent);
	void handle_process_exited(DEBUG_EVENT& dbgEvent);
	void handle_thread_created(DEBUG_EVENT& dbgEvent);
	void handle_thread_exited(DEBUG_EVENT& dbgEvent);
	void handle_dll_loaded(DEBUG_EVENT& dbgEvent);
	void handle_dll_unloaded(DEBUG_EVENT& dbgEvent);
	void handle_debug_output_string_received(DEBUG_EVENT& dbgEvent);
	void handle_exception_thrown(DEBUG_EVENT& dbgEvent);

};


