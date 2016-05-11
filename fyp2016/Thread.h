#pragma once
#include <memory>
#include <cassert>
#include <windows.h>
#include <process.h>

#include "Runnable.h"

/*
Thread helper library from http://www.bogotobogo.com/cplusplus/multithreading_win32.php
*/

class Thread {
public:
	Thread(std::auto_ptr<Runnable> run);
	Thread();
	virtual ~Thread();
	void start();
	void* join();
private:
	HANDLE hThread;
	unsigned wThreadID;
	// runnable object will be deleted automatically
	std::auto_ptr<Runnable> runnable;
	Thread(const Thread&);
	const Thread& operator=(const Thread&);
	// called when run() completes
	void setCompleted();
	// stores return value from run()
	void* result;
	virtual void* run() { return 0; }
	static unsigned WINAPI startThreadRunnable(LPVOID pVoid);
	static unsigned WINAPI startThread(LPVOID pVoid);
	void printError(LPTSTR lpszFunction, LPSTR fileName, int lineNumber);
};