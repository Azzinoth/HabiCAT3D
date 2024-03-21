#pragma once

#include "../UI/UIManager.h"
#include "../ScreenshotManager.h"

using namespace FocalEngine;

class ConsoleJobManager;
class ConsoleJob
{
	friend ConsoleJobManager;
	std::string ID;
protected:
	std::string Type;

	ConsoleJob();
	//virtual void Execute(void* InputData) = 0;
public:
	std::string GetID();
};