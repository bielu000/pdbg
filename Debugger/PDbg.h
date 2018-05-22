#pragma once
#include <map>
#include <Windows.h>

class PDbg
{
public:
	PDbg() = default;
	~PDbg() = default;
	bool StartDebugActiveProcess(DWORD processId);

private:
	std::map<DWORD, HANDLE> _processes;
	std::map<DWORD, HANDLE> _threads;

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

