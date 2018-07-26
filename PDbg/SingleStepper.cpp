#include "stdafx.h"
#include "SingleStepper.h"
#include "ErrorCodes.h"

bool SingleStepper::setSignleStep(DWORD threadId)
{
	if (!_rmManager->threadExist(threadId)) {
		_bus->error(error_codes::thread_not_exits);

		return false;
	}

	CONTEXT ctx;
	unsigned int cTrapFlag = (1 << 8); // EFLAGS registry
	memset(&ctx, 0, sizeof(ctx));
	ctx.ContextFlags = CONTEXT_CONTROL;

	if (!GetThreadContext(_rmManager->getThread(threadId), &ctx)) {
		_bus->error(error_codes::cannot_set_context, GetLastError());

		return false;
	}

	ctx.EFlags |= cTrapFlag;

	if (!SetThreadContext(_rmManager->getThread(threadId), &ctx)) {
		_bus->error(error_codes::cannot_get_context, GetLastError());

		return false;
	}

	return true;
}