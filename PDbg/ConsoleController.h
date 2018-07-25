#pragma once
#include <memory>
#include "DebuggEvents.h"
#include "Debugger.h"
#include "IAppController.h"
#include "EventBus.h"
class ConsoleController : IAppController
{
public:
	ConsoleController(std::shared_ptr<Debugger> dbg, std::shared_ptr<EventBus> bus)
		: _debugger(dbg),
		 _bus(bus)
	{
		_bus->onStarted.connect(std::bind(&ConsoleController::handleDebuggerEvent, this, std::placeholders::_1));
		_bus->onError.connect(std::bind(&ConsoleController::handleDebuggerErrorEvent, this, std::placeholders::_1));
		_bus->onBreakpointAdded.connect(std::bind(&ConsoleController::handleBreakpointAdded, this, std::placeholders::_1));
		_bus->onBreakpointRemoved.connect(std::bind(&ConsoleController::handleBreakpointRemoved, this, std::placeholders::_1));

		_bus->onProcessCreated.connect(std::bind(&ConsoleController::handleProcessCreated, this, std::placeholders::_1)); //new
		_bus->onProcessExited.connect(std::bind(&ConsoleController::handleProcessExited, this, std::placeholders::_1)); //new
		_bus->onThreadCreated.connect(std::bind(&ConsoleController::handleThreadCreated, this, std::placeholders::_1)); //new
		_bus->onThreadExited.connect(std::bind(&ConsoleController::handleThreadExited, this, std::placeholders::_1)); //new
		_bus->onDllLoaded.connect(std::bind(&ConsoleController::handleDllLoaded, this, std::placeholders::_1)); //new
		_bus->onDllUnloaded.connect(std::bind(&ConsoleController::handleDllUnloaded, this, std::placeholders::_1)); //new
		_bus->onOutputStringReceived.connect(std::bind(&ConsoleController::handleOutputDebugStringReceived, this, std::placeholders::_1)); //new
		_bus->onSingleStepExceptionOccured.connect(std::bind(&ConsoleController::handleSingleStepExceptionOccurred, this, std::placeholders::_1)); //new
		_bus->onBreakpointExceptionOccured.connect(std::bind(&ConsoleController::handleBreakpointExceptionOccurred, this, std::placeholders::_1)); //new
		_bus->onUsualExceptionOccured.connect(std::bind(&ConsoleController::handleUsualExceptionOccurred, this, std::placeholders::_1)); //new
	}

	~ConsoleController() = default;

	void run() override;
	void handleDebuggerEvent(const DebuggerStarted&);
	void handleDebuggerErrorEvent(const DebuggerErrorOccurred&);
	void handleSignleStepSet(const SingleStepSet&);
	void handleBreakpointAdded(const BreakpointAdded&);
	void handleBreakpointRemoved(const BreakpointRemoved&);

	//Debug events
	void handleProcessCreated(const ProcessCreated & ev); // new
	void handleProcessExited(const ProcessExited & ev);

	void handleThreadCreated(const ThreadCreated & ev);
	void handleThreadExited(const ThreadExited & ev);
	void handleDllLoaded(const DllLoaded & ev);
	void handleDllUnloaded(const DllUnloaded & ev);
	void handleOutputDebugStringReceived(const OutputDebugStringReveived & ev);
	void handleSingleStepExceptionOccurred(const SingleStepExceptionOccured & ev);
	void handleBreakpointExceptionOccurred(const BreakpointExceptionOccured & ev);
	void handleUsualExceptionOccurred(const UsualExceptionOccured & ev);

private: 
	std::shared_ptr<Debugger> _debugger;
	std::shared_ptr<EventBus> _bus;

	void waitForCommand();
};

