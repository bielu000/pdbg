#pragma once

#include "dbgnum.h"


namespace event_codes {
	enum event_codes {
		//system
		debugger_started = DEBUGGER_STARTED,
		debugger_error = DEBUGGER_ERROR_OCCURED,
		single_step_set = SINGLE_STEP_SET,
		breakpoint_added = BREAKPOINT_ADDED,
		breakpoint_removed = BREAKPOINT_REMOVED,

		//debug events
		proces_created = PROCESS_CREATED,
		process_exited = PROCESS_EXITED,
		thread_created = THREAD_CREATED,
		thread_exited = THREAD_EXITED,
		dll_loaded = DLL_LOADED,
		dll_unloaded = DLL_UNLOADED,
		debug_string_received = DEBUG_STRING_RECEIVED
	};
}
