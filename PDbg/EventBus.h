#pragma once
#include "DebuggEvents.h"
#include "Signal.h"

class EventBus
{
public:
	EventBus() = default;
	~EventBus() = default;
	//ERRORS
	void error(DWORD error_code, DWORD system_code = 0) {
		auto ev = DebuggerErrorOccurred();
		ev.debuggerErrorCode = error_code;
		ev.systemErrorCode = system_code;

		onError(ev);
	}
	
	//DEBUGGER EVENTS
	void event(DebuggerStarted& ev) { onStarted(ev); }
	void event(DebuggerErrorOccurred& ev) { onError(ev); }
	void event(SingleStepSet& ev) { onSingleStepSet(ev); }
	void event(BreakpointAdded& ev) { onBreakpointAdded(ev); }
	void event(BreakpointRemoved& ev) { onBreakpointRemoved(ev); }
	void event(ProcessTerminated& ev) { onProcessTerminated(ev); }

	//DEBUGGED PROCESS EVENTS
	void event(ProcessCreated& ev) { onProcessCreated(ev); }
	void event(ProcessExited& ev) { onProcessExited(ev); }
	void event(ThreadCreated& ev) { onThreadCreated(ev); }
	void event(ThreadExited& ev) { onThreadExited(ev); }
	void event(DllLoaded& ev) { onDllLoaded(ev); }
	void event(DllUnloaded& ev) { onDllUnloaded(ev); }
	void event(OutputDebugStringReveived& ev) { onOutputStringReceived(ev); }
	void event(BreakpointExceptionOccured& ev) { onBreakpointExceptionOccured(ev); }
	void event(SingleStepExceptionOccured& ev) { onSingleStepExceptionOccured(ev); }
	void event(UsualExceptionOccured& ev) { onUsualExceptionOccured(ev); }

	//SIGNALS
	//DEBUGGER COMMAND SIGNALS
	signals::signal<void(const DebuggerStarted&)> onStarted;
	signals::signal<void(const DebuggerErrorOccurred&)> onError;
	signals::signal<void(const SingleStepSet&)> onSingleStepSet;
	signals::signal<void(const BreakpointAdded&)> onBreakpointAdded;
	signals::signal<void(const BreakpointRemoved&)> onBreakpointRemoved;
	signals::signal<void(const ProcessTerminated&)> onProcessTerminated;

	//DEBUGGED PROCESS SIGNALS
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
};
