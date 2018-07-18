#pragma once

class IAppController {
public:
	virtual ~IAppController() {} 
	virtual void run() = 0;
};