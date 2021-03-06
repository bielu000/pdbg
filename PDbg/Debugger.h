#pragma once
#include <Windows.h>
#include <map>
#include <memory>
#include "Signal.h"
#include "DebuggEvents.h"
#include "Breakpoint.h"
#include "BreakpointManager.h"
#include "DebugEventHandler.h"
#include "ResourceManagerr.h"
#include "SingleStepper.h"

class IDebugger {
public:
	virtual bool run(std::string application) = 0;
	virtual bool setSingleStep(DWORD threadId, bool raiseEvent = true) = 0;
	virtual bool jumpNextInstruction() = 0;
	virtual bool addBreakpoint(LPVOID address, HANDLE hProcess, DWORD threadId) = 0;
	virtual bool removeBreakpoint(LPVOID address, HANDLE hProcess) = 0;
	virtual std::vector<std::shared_ptr<nBreakpoint>> getBreakpoints() = 0;
	virtual std::vector<DWORD> getThreads() = 0;

	virtual ~IDebugger() = default;
};

class Debugger : public IDebugger
{
public:
	Debugger(
		std::shared_ptr<EventBus>& bus, 
		std::shared_ptr<ResourceManager>& rmManager, 
		std::shared_ptr<ISingleStepper>& stepper
	)
		: _bus(bus),
		 _rmManager(rmManager), 
		 _stepper(stepper)
	{
		_bpManager = std::make_shared<BreakpointManager>(_rmManager);
		_dbgEventHandler = std::make_shared<DebugEventHandler>(bus, _rmManager, _bpManager, _stepper);
	}

	//Debugger funcionalit
	bool run(std::string application);
	bool setSingleStep(DWORD threadId, bool raiseEvent = true);
	bool jumpNextInstruction();
	bool addBreakpoint(LPVOID address, HANDLE hProcess, DWORD threadId);
	bool removeBreakpoint(LPVOID address, HANDLE hProcess);
	std::vector<std::shared_ptr<nBreakpoint>> getBreakpoints();
	std::vector<DWORD> getThreads();

	
	~Debugger() = default;

private:
	void listenEvents();

	DWORD _currentListenThread;

	std::shared_ptr<EventBus> _bus;
	std::shared_ptr<ResourceManager> _rmManager;
	std::shared_ptr<BreakpointManager> _bpManager;
	std::shared_ptr<IDebugEventHandler> _dbgEventHandler;
	std::shared_ptr<ISingleStepper> _stepper;
};


