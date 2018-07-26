#include "stdafx.h"
#include <iostream>
#include "ConsoleController.h"


void ConsoleController::run()
{
	std::string application = std::string("C:\\Users\\pb\\source\\repos\\PDbg\\Debug\\SimpleApp.exe");

	_debugger->run(application);
}

void ConsoleController::waitForCommand()
{
	while (true)
	{
		std::string command;
		std::cout << "(pdbg): ";

		command = "ni";

		if (command == "threads") {
			auto threads = _debugger->getThreads();
			std::cout << "Threds:" << std::endl;
			for (auto &x : threads) {
				std::cout << " -> " << x << std::endl;
			}
		}
		else if (command == "bps") {
			auto bps = _debugger->getBreakpoints();
			std::cout << "Breakpoints:" << std::endl;
			for (auto &x : bps) {
				std::cout << " -> 0x" << std::hex << x->address() << std::endl;
			}
		}
		else if (command == "ni") {
			_debugger->jumpNextInstruction();
			break;
		}
		else if (command == "c") {
			break;
		}
		else if (command == "cls") {
			system("cls");
		}
	}
}

void ConsoleController::handleDebuggerEvent(const DebuggerStarted& ev)
{
	std::cout << "Action - Event: Debugger Started" << std::endl;
}

void ConsoleController::handleDebuggerErrorEvent(const DebuggerErrorOccurred &ev)
{
	std::cout << "Error occured! Code: " << ev.debuggerErrorCode  << " System code: " << ev.systemErrorCode << std::endl;
}

void ConsoleController::handleSignleStepSet(const SingleStepSet &)
{
	std::cout << "Action done - single step set" << std::endl;
}

void ConsoleController::handleBreakpointAdded(const BreakpointAdded &)
{
	std::cout << "Action done - Breakpoint added" << std::endl;
}

void ConsoleController::handleBreakpointRemoved(const BreakpointRemoved &)
{
	std::cout << "Action done - Breakpoint removed" << std::endl;
}

void ConsoleController::handleProcessCreated(const ProcessCreated& ev)
{
	std::cout 
		<< "Event: Process created "
		<< " Process id: " << ev.processId 
		<< " Base address: " << ev.baseAddress
	<< std::endl;
}

void ConsoleController::handleProcessExited(const ProcessExited & ev)
{
	std::cout
		<< "Event: Process exited "
		<< " PID: " << ev.processId
		<< " Exit code: " << ev.exitCode
	<< std::endl;
}

void ConsoleController::handleThreadCreated(const ThreadCreated& ev)
{
	std::cout
		<< "Event: Thread created" 
		<< " Thread id: " << ev.threadId
		<< std::endl;
}

void ConsoleController::handleThreadExited(const ThreadExited & ev)
{
	std::cout
		<< "Event: Thread exited" 
		<< " Thread id: " << ev.threadId
		<< std::endl;
}

void ConsoleController::handleDllLoaded(const DllLoaded & ev)
{
	std::cout << "Event: Dll loaded " << std::endl;
}

void ConsoleController::handleDllUnloaded(const DllUnloaded & ev)
{
	std::cout << "Event: Dll unloaded" << std::endl;
}

void ConsoleController::handleOutputDebugStringReceived(const OutputDebugStringReveived & ev)
{
	std::cout 
		<< "Event: message received "
		<< "Value: " << ev.value 
		<< std::endl;
}

void ConsoleController::handleSingleStepExceptionOccurred(const SingleStepExceptionOccured & ev)
{
	std::cout
		<< "Event: Single-Step Exception "
		<< " First chance: " << ev.firstChance
		<< std::endl;
	
	this->waitForCommand();
}

void ConsoleController::handleBreakpointExceptionOccurred(const BreakpointExceptionOccured & ev)
{
	std::cout
		<< "Event: Breakpoint exception "
		<< " ThreadId:" << ev.threadId
		<< " First chance: " << ev.firstChance
		<< std::endl;

	this->waitForCommand();
}

void ConsoleController::handleUsualExceptionOccurred(const UsualExceptionOccured & ev)
{
	std::cout
		<< "Event: Usual exception "
		<< " First chance: " << ev.firstChance
		<< std::endl;
}
