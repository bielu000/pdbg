#pragma once
#include <Windows.h>
#include "ResourceManagerr.h"
#include "EventBus.h"
#include <memory>
#include "BreakpointManager.h"

class IDebugEventHandler {
public:
	virtual void handle(CREATE_PROCESS_DEBUG_INFO& dbgEvent, DWORD processId, DWORD threadId) = 0;
	virtual void handle(EXIT_PROCESS_DEBUG_INFO& dbgEvent, DWORD processId, DWORD threadId) = 0;
	virtual void handle(CREATE_THREAD_DEBUG_INFO& dbgEvent, DWORD processId, DWORD threadId) = 0;
	virtual void handle(EXIT_THREAD_DEBUG_INFO& dbgEvent, DWORD processId, DWORD threadId) = 0;
	virtual void handle(LOAD_DLL_DEBUG_INFO& dbgEvent, DWORD processId, DWORD threadId) = 0;
	virtual void handle(UNLOAD_DLL_DEBUG_INFO& dbgEvent, DWORD processId, DWORD threadId) = 0;
	virtual void handle(OUTPUT_DEBUG_STRING_INFO& dbgEvent, DWORD processId, DWORD threadId) = 0;
	virtual void handle(EXCEPTION_DEBUG_INFO& dbgEvent, DWORD processId, DWORD threadId) = 0;
};

class DebugEventHandler : public IDebugEventHandler
{
public:
	DebugEventHandler(std::shared_ptr<EventBus> bus, std::shared_ptr<ResourceManager> &rm, std::shared_ptr<BreakpointManager> bp)
		: _bus(bus),
		  _rmManager(rm),
		 _bpManager(bp)
	{}
	virtual ~DebugEventHandler() = default;

	void handle(CREATE_PROCESS_DEBUG_INFO& dbgEvent, DWORD processId, DWORD threadId) override;
	void handle(EXIT_PROCESS_DEBUG_INFO& dbgEvent, DWORD processId, DWORD threadId) override;
	void handle(CREATE_THREAD_DEBUG_INFO& dbgEvent, DWORD processId, DWORD threadId) override;
	void handle(EXIT_THREAD_DEBUG_INFO& dbgEvent, DWORD processId, DWORD threadId) override;
	void handle(LOAD_DLL_DEBUG_INFO& dbgEvent, DWORD processId, DWORD threadId) override;
	void handle(UNLOAD_DLL_DEBUG_INFO& dbgEvent, DWORD processId, DWORD threadId) override;
	void handle(OUTPUT_DEBUG_STRING_INFO& dbgEvent, DWORD processId, DWORD threadId) override;
	void handle(EXCEPTION_DEBUG_INFO& dbgEvent, DWORD processId, DWORD threadId) override;

private:
	std::shared_ptr<ResourceManager> _rmManager;
	std::shared_ptr<BreakpointManager> _bpManager;
	std::shared_ptr<EventBus> _bus;
};

