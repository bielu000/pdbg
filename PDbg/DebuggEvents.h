#pragma once
#include <string>
#include <Windows.h> //remove later
#include "EventCodes.h"

class BaseEvent {
public:
	unsigned int eventCode;
};

class DebuggerStarted : public BaseEvent {
public:
	unsigned int eventCode = event_codes::debugger_started;
	int processId;
	std::string appName;
};

class DebuggerErrorOccurred : public BaseEvent {
public:
	unsigned int eventCode = event_codes::debugger_error;
	unsigned long systemErrorCode;
	unsigned int  debuggerErrorCode;
};

class SingleStepSet : public BaseEvent {
public:
	unsigned int eventCode = event_codes::single_step_set;
	unsigned long threadId;
};

class BreakpointAdded : public BaseEvent {
public:
	unsigned int eventCode = event_codes::breakpoint_added;
};

class BreakpointRemoved : public BaseEvent {
public:
	unsigned int eventCode = event_codes::breakpoint_removed;
};


//Debug events
class ProcessCreated : public BaseEvent {
public:
	unsigned int eventCode = event_codes::proces_created;
	unsigned long processId;
	unsigned long* baseAddress; //type
};

class ProcessExited : public BaseEvent {
public:
	unsigned int eventCode = event_codes::process_exited;
	unsigned long processId;
	unsigned long exitCode;
};

class ThreadCreated : public BaseEvent {
public:
	unsigned int eventCode = event_codes::thread_created;
	unsigned long threadId;
} ;

class ThreadExited : public BaseEvent {
public:
	unsigned int eventCode = event_codes::thread_exited;
	unsigned long threadId;
	unsigned long exitCode;
};

class DllLoaded : public BaseEvent {
public:
	unsigned int eventCode = event_codes::dll_loaded;
};

class DllUnloaded : public BaseEvent {
public:
	unsigned int eventCode = event_codes::dll_unloaded;
} ;

class OutputDebugStringReveived : public BaseEvent {
public:
	unsigned int eventCode = event_codes::debug_string_received;
	std::string value;
} ;

