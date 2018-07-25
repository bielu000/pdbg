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

class Debugger
{
public:
	Debugger(
		std::shared_ptr<EventBus>& bus, 
		std::shared_ptr<ResourceManager>& rmManager, 
		std::shared_ptr<ISingleStepper> stepper
	)
		: _bus(bus),
		 _rmManager(rmManager), 
		 _stepper(stepper)
	{
		_dbgEventHandler = std::make_shared<DebugEventHandler>(bus, _rmManager);
		_bpManager = std::make_shared<BreakpointManager>(_rmManager);
	}

	//Debugger funcionalit
	bool run(std::string application);
	bool setSingleStep(DWORD threadId, bool raiseEvent = true);
	bool addBreakpoint(LPVOID address, HANDLE hProcess, DWORD threadId);
	bool removeBreakpoint(LPVOID address, HANDLE hProcess);
	std::vector<std::shared_ptr<nBreakpoint>> getBreakpoints();
	std::vector<DWORD> getThreads();

	
	~Debugger() = default;

private:
	void listenEvents();

	std::shared_ptr<EventBus> _bus;
	std::shared_ptr<ResourceManager> _rmManager;
	std::shared_ptr<BreakpointManager> _bpManager;
	std::shared_ptr<IDebugEventHandler> _dbgEventHandler;
	std::shared_ptr<ISingleStepper> _stepper;
	std::map<DWORD, HANDLE> _processes; //posiibly to replace with struct
	std::map<DWORD, HANDLE> _threads; // the same as above
	bool _notifySingleStep = FALSE;
	DWORD _debugContinueStatus = DBG_CONTINUE;
};


