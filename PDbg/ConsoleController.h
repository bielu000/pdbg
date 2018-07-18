#pragma once
#include <memory>
#include "DebuggEvents.h"
#include "Debugger.h"
#include "IAppController.h"
class ConsoleController : IAppController
{
public:
	ConsoleController(std::shared_ptr<Debugger> dbg)
		: _debugger(dbg)
	{
		_debugger->onStarted.connect(std::bind(&ConsoleController::handleDebuggerEvent, this, std::placeholders::_1));
		_debugger->onError.connect(std::bind(&ConsoleController::handleDebuggerErrorEvent, this, std::placeholders::_1));

		_debugger->onProcessCreated.connect(std::bind(&ConsoleController::handleProcessCreated, this, std::placeholders::_1)); //new
		_debugger->onProcessExited.connect(std::bind(&ConsoleController::handleProcessExited, this, std::placeholders::_1)); //new
		_debugger->onThreadCreated.connect(std::bind(&ConsoleController::handleThreadCreated, this, std::placeholders::_1)); //new
		_debugger->onThreadExited.connect(std::bind(&ConsoleController::handleThreadExited, this, std::placeholders::_1)); //new
		_debugger->onDllLoaded.connect(std::bind(&ConsoleController::handleDllLoaded, this, std::placeholders::_1)); //new
		_debugger->onDllUnloaded.connect(std::bind(&ConsoleController::handleDllUnloaded, this, std::placeholders::_1)); //new
		_debugger->onOutputStringReceived.connect(std::bind(&ConsoleController::handleOutputDebugStringReceived, this, std::placeholders::_1)); //new
	}

	~ConsoleController() = default;

	void run() override;
	void handleDebuggerEvent(const DebuggerStarted&);
	void handleDebuggerErrorEvent(const DebuggerErrorOccurred&);

	//Debug events
	void handleProcessCreated(const ProcessCreated & ev); // new
	void handleProcessExited(const ProcessExited & ev);

	void handleThreadCreated(const ThreadCreated & ev);
	void handleThreadExited(const ThreadExited & ev);
	void handleDllLoaded(const DllLoaded & ev);
	void handleDllUnloaded(const DllUnloaded & ev);
	void handleOutputDebugStringReceived(const OutputDebugStringReveived & ev);

private: 
	std::shared_ptr<Debugger> _debugger;
};

