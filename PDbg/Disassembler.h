#pragma once
#include <Windows.h>
#include <string>
#include <iostream>
#include <vector>
#include <memory>

class Instruction {
public:
	Instruction(DWORD address, std::string code)
		: _address(address),
		  _code(code)
	{}

	DWORD address() { return this->_address; }
	std::string code() { return this->_code; }

private:
	DWORD _address;
	std::string _code;
};


class IDisassembler
{
public :
	virtual ~IDisassembler() = default;
	virtual std::vector<std::shared_ptr<Instruction>> disassembly(HANDLE hProcess, HANDLE hThread, DWORD address, unsigned int instructionsAmount) = 0;
};

class Disassembler : public IDisassembler
{
public:
	Disassembler() = default;
	~Disassembler() = default;

	std::vector<std::shared_ptr<Instruction>> disassembly(
		HANDLE hProcess, 
		HANDLE hThraed, 
		DWORD address, 
		unsigned int instructionsAmount
	) override;
};

