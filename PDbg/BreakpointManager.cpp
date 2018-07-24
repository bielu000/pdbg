#include "stdafx.h"
#include "BreakpointManager.h"
#include <functional>
#include <algorithm>
#include <iostream>

bool BreakpointManager::add(LPVOID address, HANDLE hProcess, DWORD threadid)
{
	auto new_breakpoint = this->add_to_memory(address, hProcess, threadid);

	if (new_breakpoint != nullptr) {
		this->_bps.push_back(new_breakpoint);

		return true;
	}

	return false;
}

std::shared_ptr<nBreakpoint> BreakpointManager::add_to_memory(LPVOID address, HANDLE hProcess, DWORD threadid)
{
	BYTE originalInstruction;
	SIZE_T bytesRead;

	if (!ReadProcessMemory(hProcess, address, &originalInstruction, sizeof(BYTE), &bytesRead)) {
		std::cout << "Breakpoint Manager -> add -> cannot read process memory" << std::endl;
		return nullptr;

	}

	BYTE int3optcode = 0xCC;
	SIZE_T bytesWritten;

	if (!WriteProcessMemory(hProcess, address, &int3optcode, sizeof(BYTE), &bytesWritten)) {
		std::cout << "Breakpoint Manager -> add -> cannot write process memory" << std::endl;
		return nullptr;
	}

	return std::make_shared<nBreakpoint>(address, originalInstruction, threadid);
}

bool BreakpointManager::remove(LPVOID address, HANDLE hProcess)
{
	if (this->remove_from_memory(address, hProcess)) {
		std::remove_if(this->_bps.begin(), this->_bps.end(), [address](std::shared_ptr<nBreakpoint>& b)-> bool
		{
			return b->address() == address;
		});

		return true;
	}


	return false;
}
bool BreakpointManager::remove_from_memory(LPVOID address, HANDLE hProcess)
{
	//add error messages
	if (!this->exist(address)) {
		std::cout << "BreakpointManager -> remove -> not exist" << std::endl;
		return false;
	}

	auto bp = this->get(address);
	SIZE_T bytes_written;
	BYTE optcode = bp->instruction();

	if (!WriteProcessMemory(hProcess, bp->address(), &optcode, sizeof(BYTE), &bytes_written)) {
		std::cout << "BrekpointManager -> remove -> cannot write process" << std::endl;
		return false;
	}

	return true;
}


bool BreakpointManager::restoreOriginalByte(LPVOID address, HANDLE hProcess)
{
	if (!this->exist(address)) {
		return false;
	}

	auto iterator = std::find_if(this->_bps.begin(), this->_bps.end(), [address](std::shared_ptr<nBreakpoint> &b) ->bool
	{
		return b->address() == address;
	});

	if (!this->remove_from_memory(address, hProcess)) {

		return false;
	}

	auto bp = static_cast<std::shared_ptr<nBreakpoint>>(*iterator);
	bp->setPending(true);

	return true;
}
bool BreakpointManager::restorePending(DWORD threadId, HANDLE hProcess)
{
	return true;
}
bool BreakpointManager::exist(LPVOID address)
{
	auto iterator = std::find_if(this->_bps.begin(), this->_bps.end(), [address](std::shared_ptr<nBreakpoint>& b) ->bool
	{
		return b->address() == address;
	});

	if (iterator != this->_bps.end()) {
		return true;
	}

	return false;
}
bool BreakpointManager::existAnyPending(DWORD threadId)
{
	auto iterator = std::find_if(this->_bps.begin(), this->_bps.end(), [threadId](std::shared_ptr<nBreakpoint>& b) ->bool
	{
		return b->thread() == threadId && b->pending() == true;
	});

	if (iterator != this->_bps.end()) {
		static_cast<std::shared_ptr<nBreakpoint>>(*iterator)->setPending(false);
		return true;
	}

	return false;
}
std::shared_ptr<nBreakpoint> BreakpointManager::get(LPVOID address)
{
	auto iterator = std::find_if(this->_bps.begin(), this->_bps.end(), [address](std::shared_ptr<nBreakpoint> b) ->bool
	{
		return b->address() == address;
	});

	return *iterator;
}

std::vector<std::shared_ptr<nBreakpoint>> BreakpointManager::getAll()
{
	return this->_bps;
}

