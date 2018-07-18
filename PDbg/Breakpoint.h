#pragma once
#include <Windows.h>
#include <map>
#include <iostream>

class Breakpoint
{
public:
	Breakpoint(LPVOID address, BYTE instruction = NULL)
		: _address(address), _instruction(instruction)
	{}
	~Breakpoint() = default;

	LPVOID address() { return _address; }
	BYTE instruction() { return _instruction; }

private:
	LPVOID _address;
	BYTE _instruction;
};



class Breakpoints {
public:
	bool add(LPVOID address, BYTE opcode = NULL) { //maybe create another struct for pending breakpoints
		//check
		_breakpoints[address] = std::make_shared<Breakpoint>(address, opcode);

		return true;
	}

	bool remove(LPVOID address) {
		if (!exist(address)) {
			return false;
		}

		_breakpoints.erase(address);

		return true;
	}

	bool exist(LPVOID address) {
		if (_breakpoints.find(address) == _breakpoints.end()) {
			return false;
		}

		return true;
	}

	std::shared_ptr<Breakpoint> get(LPVOID address) {
		if (!exist(address)) {
			std::cout << "Breakpointa nie ma Breakpoint.h";
		}

		return std::move(_breakpoints[address]);
	}

private:
	std::map<LPVOID, std::shared_ptr<Breakpoint>> _breakpoints;
};

