#pragma once
#include <string>
#include <Windows.h> //remove later

typedef struct {
	std::string message;
	int processId;
	std::string appName;
} DebuggerStarted;

typedef struct {
	std::string message;
	unsigned long systemErrorCode;
} DebuggerErrorOccurred;

typedef struct {
	std::string message;
	unsigned long processId;
	unsigned long* baseAddress; //type
} ProcessCreated;

typedef struct {
	std::string message;
	unsigned long processId;
	unsigned long exitCode;
} ProcessExited;

typedef struct {
	std::string message;
	unsigned long threadId;
} ThreadCreated;

typedef struct {
	std::string message;
	unsigned long threadId;
	unsigned long exitCode;
} ThreadExited;

typedef struct {
	std::string message;
} DllLoaded;

typedef struct {
	std::string message;
} DllUnloaded;

typedef struct {
	std::string message;
	std::string value;
} OutputDebugStringReveived;

