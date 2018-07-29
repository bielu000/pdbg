#include "stdafx.h"
#include <iostream>
#include "ConsoleController.h"


void ConsoleController::run()
{
	std::string application = std::string("C:\\Users\\pb\\source\\repos\\PDbg\\Debug\\SimpleApp.exe");

	_debugger->run(application);
}

void ConsoleController::registerCommand(std::shared_ptr<ICommand> cmd)
{
	if (this->_commands.find(cmd->optcode()) != this->_commands.end()) {
		std::cout << "ConsoleController: Warrning! Command with optcode: " << cmd->optcode() << " already exist, and will be overwritten!" << std::endl;
	}

	this->_commands[cmd->optcode()] = cmd;

	std::cout << "ConsoleController: registered command with optcode : " << cmd->optcode() << std::endl;
}

void ConsoleController::waitForCommand()
{
	while (true)
	{
		std::string line = "";
		std::string command;
		std::string args;

		std::cout << "(pdbg): ";
		std::getline(std::cin, line);

		std::stringstream ss(line);

		ss >> command >> args;

		if (this->_commands.find(command) == this->_commands.end()) {
			std::cout << "Commands:" << std::endl;
			for (auto &x : this->_commands) {
				std::cout << " -> " << x.first << std::endl;
			}

			continue;
		}

		auto x = this->_commands[command];
		x->parse(args);

		if (!x->validate()) {
			std::cout << x->helper() << std::endl;
			continue;
		}

		x->handle(_debugger);
		x->reset();

		if (x->needBreak()) {
			break;
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
