#pragma once
#include <map>
#include <Windows.h>

typedef struct BreakpointTag Breakpoint, *PBreakpoint;
typedef VOID(*PBreakpointHandler)(PBreakpoint breakpoint, DEBUG_EVENT *dbgEvent, PDWORD continue_status);

struct BreakpointTag
{
	LPVOID address;
	BYTE byte;
	PBreakpointHandler handler;
};

class PDbg
{
public:
	PDbg() = default;
	~PDbg();
	bool StartDebugActiveProcess(DWORD processId);
	bool StartDebugNewProcess(LPTSTR processName);
	bool AddBreakpoint(LPVOID address, HANDLE hProecss, PBreakpointHandler pbreakpoint_handler = NULL);
	bool RemoveBreakpoint(LPVOID address, HANDLE hProcess);
	bool SetThreadTrapFlag(HANDLE hThread);
	bool Shutdown();

private:
	DWORD _startupProcessId;
	LPVOID _image_base; 
	DWORD _image_size;
	std::map<DWORD, HANDLE> _processes; //handle all processes
	std::map<DWORD, HANDLE> _threads; // handle all threads
	std::map<LPVOID, PBreakpoint> _breakpoints; // handle all breakpoints
	std::map<DWORD, LPVOID> _pending_breakpoints; // handle breakpoints to recreate

	void run();
	void handle_create_process_debug_event(DEBUG_EVENT* dbgEvent);
	void handle_create_thread_debug_event(DEBUG_EVENT* dbgEvent);
	void handle_exception_debug_event(DEBUG_EVENT* dbgEvent);
	void handle_load_dll_debug_event(DEBUG_EVENT* dbgEvent);
	void handle_unload_dll_debug_event(DEBUG_EVENT* dbgEvent);
	void handle_output_debug_string(DEBUG_EVENT* dbgEvent);
	void handle_exit_thread_debug_event(DEBUG_EVENT* dbgEvent);
	void handle_exit_process_debug_event(DEBUG_EVENT* dbgEvent);
};

