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
	virtual void handle(const std::shared_ptr<IDebugger> &dbg) = 0;
	virtual void reset() = 0;
	virtual bool needBreak() = 0;

	virtual ~ICommand() = default;
};

class GetBreakpoints : public ICommand {
public:
	bool needBreak() override { return false;  }
	std::string optcode() override { return this->_optcode; }
	std::string helper() override { return this->_helper; }
	void parse(std::string cmdline) override {}
	bool validate() override { return true; }
	void handle(const std::shared_ptr<IDebugger> & dbg) override
	{
		auto bps = dbg->getBreakpoints();

		std::cout << "Breakpoints" << std::endl;

		for (auto &x : bps) {
			std::cout << "-> " << "0x" << x->address() << std::endl;
		}
	}
	void reset() {}

private:
	std::string _optcode = "bps";
	std::string _helper = "Shows all added breakpoints";
};

class GetThraeds : public ICommand {
public:
	bool needBreak() override { return false; }
	std::string optcode() override { return this->_optcode; }
	std::string helper() override { return this->_helper; }
	void parse(std::string cmdline) override {}
	bool validate() override { return true; }
	void handle(const std::shared_ptr<IDebugger> & dbg) override
	{
		auto bps = dbg->getThreads();

		std::cout << "Threads" << std::endl;

		for (auto &x : bps) {
			std::cout << "-> " << x << std::endl;
		}
	}
	void reset() {}
private:
	std::string _optcode = "threads";
	std::string _helper = "Shows all threads";
};

class AddBreakpoint : public ICommand {
public:
	bool needBreak() override { return false; }
	std::string optcode() override { return this->_optcode; }
	std::string helper() override { return this->_helper; }
	void parse(std::string cmdline)
	{
		std::stringstream ss(cmdline);
		ss >> std::hex >> this->_address;

		std::cout << "Address" << std::hex << this->_address << std::endl;
	}
	bool validate()
	{
		if (this->_address == 0) {
			return false;
		}

		return true;
	}
	void handle(const std::shared_ptr<IDebugger> & dbg) override
	{
		std::cout << "Add breakpoint" << std::endl;
		std::cout << this->_address << std::endl;

		dbg->addBreakpoint(this->_address);
	}

	void reset() override
	{
		_address = 0;
	}

private:
	DWORD _address;
	std::string _optcode = "bps-add";
	std::string _helper = "Error! Add breakpoint. Usage bps-add address. Example: bps-add 0x432e4f";
};

class ContinueDebugging : public ICommand {
public:
	bool needBreak() override { return true; }
	std::string optcode() override { return this->_optcode; }
	std::string helper() override { return this->_helper; }
	void parse(std::string cmdline) override {}
	bool validate() override { return true; }
	void handle(const std::shared_ptr<IDebugger> & dbg) override
	{
	}

	void reset() {}
private:
	std::string _optcode = "c";
	std::string _helper = "Continue debugging.";
};

class NextInstruction : public ICommand {
public:
	bool needBreak() override { return true; }
	std::string optcode() override { return this->_optcode; }
	std::string helper() override { return this->_helper; }
	void parse(std::string cmdline) override {}
	bool validate() override { return true; }
	void handle(const std::shared_ptr<IDebugger> & dbg) override
	{
		dbg->jumpNextInstruction();
	}
	void reset() {}
private:
	std::string _optcode = "ni";
	std::string _helper = "Next instruction.";
};

class ClearConsole : public ICommand {
public:
	bool needBreak() override { return false; }
	std::string optcode() override { return this->_optcode; }
	std::string helper() override { return this->_helper; }
	void parse(std::string cmdline) override {}
	bool validate() override { return true; }
	void handle(const std::shared_ptr<IDebugger> & dbg) override
	{
		std::cout << "Clear console" << std::endl;

		system("cls");
	}
	void reset() {}
private:
	std::string _optcode = "cls";
	std::string _helper = "CLear console.";
};


class Disassembly : public ICommand
{
public:
	bool needBreak() override { return false; }
	std::string optcode() override { return this->_optcode; }
	std::string helper() override { return this->_helper; }
	void parse(std::string cmdline)
	{
		std::stringstream ss(cmdline);
		
		std::string tmp_address;

		ss >> tmp_address;
		ss >> this->_linesParam;
		ss >> this->_linesAmount;

		std::stringstream addressStream;
		addressStream << tmp_address;
		addressStream >> std::hex >> this->_address;
	}
	bool validate()
	{
		if (this->_address == 0) {
			return false;
		}

		if (this->_linesParam.length() == 0) {
			return false;
		}

		if (this->_linesAmount <= 0) {
			return false;
		}
	}
	void handle(const std::shared_ptr<IDebugger> & dbg) override
	{
		std::cout << "Disassembly" << std::endl;	

		dbg->disassembly(this->_address, this->_linesAmount);
	}

	void reset() override
	{
		this->_address = 0;
		this->_linesAmount = 0;
		this->_linesParam.clear();
	}

private:
	DWORD _address;
	std::string _linesParam;
	unsigned int _linesAmount;
	std::string _optcode = "diss";
	std::string _helper = "Error! Disassembly. Usage diss address -l x. Example: diss 0x432e4f -l 10";
};

class ConsoleCommandHandler
{
public:
	ConsoleCommandHandler();
	~ConsoleCommandHandler();
};

