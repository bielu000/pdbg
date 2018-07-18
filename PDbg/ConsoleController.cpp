#include "stdafx.h"
#include <iostream>
#include "ConsoleController.h"


void ConsoleController::run()
{
	std::string application = std::string("C:\\Users\\pb\\source\\repos\\PDbg\\Debug\\SimpleApp.exe");

	_debugger->run(application);
}

void ConsoleController::handleDebuggerEvent(const DebuggerStarted& ev)
{
	std::cout << "Ev: Debugger Started" << std::endl;
}

void ConsoleController::handleDebuggerErrorEvent(const DebuggerErrorOccurred &ev)
{
	std::cout << "Error occured! Message: " << " System code: " << ev.systemErrorCode << std::endl;
}

void ConsoleController::handleProcessCreated(const ProcessCreated& ev)
{
	std::cout 
		<< "Ev: Process created "
		<< " Process id: " << ev.processId 
		<< " Base address: " << ev.baseAddress
	<< std::endl;
}

void ConsoleController::handleProcessExited(const ProcessExited & ev)
{
	std::cout
		<< "Ev: Process exited "
		<< " PID: " << ev.processId
		<< " Exit code: " << ev.exitCode
	<< std::endl;
}

void ConsoleController::handleThreadCreated(const ThreadCreated& ev)
{
	std::cout
		<< "Ev: Thread created" 
		<< " Thread id: " << ev.threadId
		<< std::endl;
}

void ConsoleController::handleThreadExited(const ThreadExited & ev)
{
	std::cout
		<< "Ev: Thread exited" 
		<< " Thread id: " << ev.threadId
		<< std::endl;
}

void ConsoleController::handleDllLoaded(const DllLoaded & ev)
{
	std::cout << "Ev. Dll loaded " << std::endl;
}

void ConsoleController::handleDllUnloaded(const DllUnloaded & ev)
{
	std::cout << "Ev. Dll unloaded" << std::endl;
}

void ConsoleController::handleOutputDebugStringReceived(const OutputDebugStringReveived & ev)
{
	std::cout 
		<< "Ev. message received "
		<< "Value: " << ev.value 
		<< std::endl;
}
