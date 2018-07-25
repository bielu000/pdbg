#pragma once
#include <Windows.h>
#include <memory>
#include "EventBus.h"
#include "ResourceManagerr.h"

class ISingleStepper {
public:
	ISingleStepper() = default;
	virtual ~ISingleStepper() = default;
	virtual bool setSignleStep(DWORD threadId) = 0;
};

class SingleStepper : public ISingleStepper
{
public:
	SingleStepper(std::shared_ptr<EventBus> bus, std::shared_ptr<ResourceManager> rmManger)
		: _bus(bus),
		  _rmManager(rmManger)
	{}

	~SingleStepper() = default;

	bool setSignleStep(DWORD threadId) override;

private: 
	std::shared_ptr<EventBus> _bus;
	std::shared_ptr<ResourceManager> _rmManager;
};

