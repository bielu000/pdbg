#pragma once
#include <string>
#include <sstream>
#include "Debugger.h"

class ICommand {
public:
	virtual std::string optcode() = 0;
	virtual std::string helper() = 0;
	virtual void parse(std::string cmdline) = 0;
	virtual bool validate() = 0;
	virtual void handle(std::shared_ptr<IDebugger> dbg) = 0;
};

class IConsoleCommand : public ICommand {
public:
	virtual bool needBreak() = 0;
};

class ShowBreakpoints : public IConsoleCommand {
public:
	bool needBreak() override { return false;  }
	std::string optcode() override { return this->_optcode; }
	std::string helper() override { return this->_helper; }
	void parse(std::string cmdline) override {}
	bool validate() override { return true; }
	void handle(std::shared_ptr<IDebugger> dbg) override
	{
		//auto bps = dbg->getBreakpoints();

		std::cout << "Breakpoints" << std::endl;

		//for (auto &x : bps) {
		//	std::cout << "-> " << x->address() << std::endl;
	//	}
	}
private:
	std::string _optcode = "bps";
	std::string _helper = "Shows all added breakpoints";
};

class AddBreakpoint : public IConsoleCommand {
public:
	bool needBreak() override { return false; }
	std::string optcode() override { return this->_optcode; }
	std::string helper() override { return this->_helper; }
	void parse(std::string cmdline)
	{
		std::stringstream ss(cmdline);

		ss >> this->_address;
	}
	bool validate()
	{
		if (this->_address.length() == 0) {
			return false;
		}
	}
	void handle(std::shared_ptr<IDebugger> dbg) override
	{
		std::cout << this->_address << std::endl;
	}

private:
	std::string _address;
	std::string _optcode = "bps-add";
	std::string _helper = "Error! Add breakpoint. Usage bps-add address. Example: bps-add 0x432e4f";
};

class ConsoleCommandHandler
{
public:
	ConsoleCommandHandler();
	~ConsoleCommandHandler();
};

