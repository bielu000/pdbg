#pragma once
#include <Windows.h>
#include <string>
#include <iostream>
#include <vector>
#include <memory>

class IInstruction {
public:
	virtual ~IInstruction() = default;
	virtual DWORD address() = 0;
	virtual std::string code() = 0;
};

class Instruction : public IInstruction {
public:
	Instruction(DWORD address, std::string code)
		: _address(address),
		  _code(code)
	{}

	DWORD address() override { return this->_address; }
	std::string code() override { return this->_code; }

private:
	DWORD _address;
	std::string _code;
};


class IDisassembler
{
public :
	virtual ~IDisassembler() = default;
	virtual bool disassembly(HANDLE hProcess, HANDLE hThread, DWORD address, unsigned int instructionsAmount) = 0;
	virtual std::vector<std::shared_ptr<IInstruction>> getInstructions() = 0;
};

class Disassembler : public IDisassembler
{
public:
	Disassembler() = default;
	~Disassembler() = default;
	bool disassembly(HANDLE hProcess, HANDLE hThraed, DWORD address, unsigned int instructionsAmount) override;
	virtual std::vector<std::shared_ptr<IInstruction>> getInstructions() { return _disassembledInstructions; }

private:
	std::vector<std::shared_ptr<IInstruction>> _disassembledInstructions;
};

