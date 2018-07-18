// PDbg.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <iostream>
#include <memory>

#include "Signal.h"
#include "Debugger.h"
#include "ConsoleController.h"

int main()
{
	auto debugger = std::make_shared<Debugger>();
	auto controller = std::make_unique<ConsoleController>(debugger);
	controller->run();

	getchar();
	
	return 0;
}

