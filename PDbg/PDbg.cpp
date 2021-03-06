// PDbg.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <iostream>
#include <memory>

#include "Signal.h"
#include "Debugger.h"
#include "ConsoleController.h"
#include "EventBus.h"
#include "SingleStepper.h"
#include "ConsoleCommandHandler.h"
#include <sstream>


int main()
{
	std::shared_ptr<EventBus> bus = std::make_shared<EventBus>();
	std::shared_ptr<ResourceManager> rm = std::make_shared<ResourceManager>();
	std::shared_ptr<ISingleStepper> dbgStepper = std::make_shared<SingleStepper>(bus, rm);
	std::shared_ptr<Debugger> dbg = std::make_shared<Debugger>(bus, rm, dbgStepper);

	auto showBreakpointsCmd = std::make_shared<GetBreakpoints>();
	auto showThreadsCmd = std::make_shared<GetThraeds>();
	auto addBreakpointCmd = std::make_shared<AddBreakpoint>();
	auto continueCmd = std::make_shared<ContinueDebugging>();
	auto nextInstructionCmd = std::make_shared<NextInstruction>();
	auto clearConsoleCmd = std::make_shared<ClearConsole>();

	auto controller = std::make_unique<ConsoleController>(dbg, bus);
	controller->registerCommand(showBreakpointsCmd);
	controller->registerCommand(showThreadsCmd);
	controller->registerCommand(addBreakpointCmd);
	controller->registerCommand(continueCmd);
	controller->registerCommand(nextInstructionCmd);
	controller->registerCommand(clearConsoleCmd);
	controller->run();

	getchar();
	
	return 0;
}

