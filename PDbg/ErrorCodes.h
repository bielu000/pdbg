#pragma once
#include "dbgnum.h"

namespace error_codes {
	enum error_code {
		cannot_add_breakpoint = CANNOT_ADD_BREAKPOINT,
		cannot_remove_breakpoint = CANNOT_REMOVE_BREAKPOINT,
		cannot_restore_original_byte = CANNOT_RESTORE_ORGINAL_BYTE,
		cannot_restore_pending_breakpoint = CANNOT_RESTORE_PENDING_BREAKPOINT,
		cannot_create_process = CANNOT_CREATE_PROCESS,
		thread_not_exits = THREAD_NOT_EXIST,
		cannot_set_context = CANNOT_SET_CONTEXT,
		cannot_get_context = CANNOT_GET_CONTEXT,
		cannot_read_process_memory = CANNOT_READ_PROCESS_MEMORY,
		cannot_write_process_memory = CANNOT_WRITE_PROCESS_MEMORY,
		breakpoint_not_exist = BREAKPOINT_NOT_EXIST
	};
}