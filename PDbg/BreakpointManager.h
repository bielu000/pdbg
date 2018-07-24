#pragma once
#include <Windows.h>
#include <vector>
#include <memory>

class nBreakpoint {
public:
	nBreakpoint(LPVOID address, BYTE originalByte, DWORD issuedByThread)
		: _address(address), 
		  _orgInstruction(originalByte), 
		  _issuedByThread(issuedByThread)
	{}

	LPVOID address() { return _address; }
	DWORD thread() { return _issuedByThread; }
	bool pending() { return _pending; }
	BYTE instruction() { return _orgInstruction; }
	void setPending(bool val) { _pending = val; }

private:
	LPVOID _address;
	BYTE _orgInstruction;
	DWORD _issuedByThread;
	bool _pending = false;
};

class BreakpointManager
{
public:
	BreakpointManager() = default;
	~BreakpointManager() = default;

	bool add(LPVOID address, HANDLE hrProcess, DWORD threadid);
	bool remove(LPVOID address, HANDLE hProcess);
	bool exist(LPVOID address);
	bool existAnyPending(DWORD threadId);
	std::shared_ptr<nBreakpoint> get(LPVOID address);
	std::vector<std::shared_ptr<nBreakpoint>> getAll();
	bool restoreOriginalByte(LPVOID address, HANDLE hProcess);
	bool restorePending(DWORD threaid, HANDLE hProcess);


private: 
	std::vector<std::shared_ptr<nBreakpoint>> _bps;
	std::shared_ptr<nBreakpoint> add_to_memory(LPVOID address, HANDLE hProcess, DWORD threadid);
	bool remove_from_memory(LPVOID address, HANDLE hProcess);
};

