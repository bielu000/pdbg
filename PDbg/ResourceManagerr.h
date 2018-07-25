#pragma once
#include <Windows.h>
#include <map>

class IResourceManger {
public:
	virtual bool anyProcessExist() = 0;
	virtual void addProcess(DWORD processId, HANDLE hProcess) = 0;
	virtual HANDLE getProcess(DWORD processid) = 0;
	virtual void removeProcess(DWORD processId) = 0;
	
	virtual bool anyThreadExist() = 0;
	virtual void addThread(DWORD threadid, HANDLE hThread) = 0;
	virtual HANDLE getThread(DWORD threadid) = 0;
	virtual void removeThread(DWORD threadid) = 0;
};

class ResourceManager : public IResourceManger {
public:
	bool anyProcessExist() override
	{ 
		return this->_processes.size() > 0; 
	}
	void addProcess(DWORD processId, HANDLE hProcess) override 
	{ 
		this->_processes[processId] = hProcess; 
	}
	
	HANDLE getProcess(DWORD processId) override  
	{ 
		return this->_processes[processId]; 
	}

	void removeProcess(DWORD processId) override
	{
		this->_processes.erase(processId);
	}

	bool anyThreadExist() override
	{
		return this->_processes.size() > 0;
	}
	void addThread(DWORD threadid, HANDLE hThread) override
	{
		this->_threads[threadid] = hThread;
	}
	HANDLE getThread(DWORD threadid) override
	{
		return this->_threads[threadid];
	}

	void removeThread(DWORD threadid) override
	{
		this->_processes.erase(threadid);
	}

private:
	std::map<DWORD, HANDLE> _processes;
	std::map<DWORD, HANDLE> _threads;
};