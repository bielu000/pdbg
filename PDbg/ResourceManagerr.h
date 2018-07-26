#pragma once
#include <Windows.h>
#include <map>
#include <vector>

class IResourceManger {
public:
	virtual bool anyProcessExist() = 0;
	virtual void addProcess(DWORD processId, HANDLE hProcess) = 0;
	virtual HANDLE getProcess(DWORD processid) = 0;
	virtual bool processExist(DWORD processid) = 0;
	virtual std::vector<DWORD> getAllProcPids() = 0;
	virtual void removeProcess(DWORD processId) = 0;
	
	virtual bool anyThreadExist() = 0;
	virtual void addThread(DWORD threadid, HANDLE hThread) = 0;
	virtual HANDLE getThread(DWORD threadid) = 0;
	virtual bool threadExist(DWORD processid) = 0;
	virtual std::vector<DWORD> getAllThreadPids() = 0;
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
	virtual bool processExist(DWORD processid) override
	{
		if (this->_processes.find(processid) == this->_processes.end()) {
			return false;
		}

		return true;
	}

	std::vector<DWORD> getAllProcPids() override 
	{
		std::vector<DWORD> v;

		for (auto &x : this->_processes) {
			v.push_back(x.first);
		}

		return v;
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
	virtual bool threadExist(DWORD threadid) override
	{
		auto x = this->_threads;
		if (this->_threads.find(threadid) == this->_threads.end()) {
			return false;
		}

		return true;
	}
	std::vector<DWORD> getAllThreadPids() override 
	{
		std::vector<DWORD> v;

		for (auto &x : this->_threads) {
			v.push_back(x.first);
		}

		return v;
	}

	void removeThread(DWORD threadid) override
	{
		this->_processes.erase(threadid);
	}

private:
	std::map<DWORD, HANDLE> _processes;
	std::map<DWORD, HANDLE> _threads;
};