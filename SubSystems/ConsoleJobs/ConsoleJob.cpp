#include "ConsoleJob.h"
using namespace FocalEngine;

ConsoleJob::ConsoleJob()
{
	ID = UNIQUE_ID.GenerateID();
}

FEUUID ConsoleJob::GetID()
{
	return ID;
}

void ConsoleJob::OutputConsoleTextWithColor(std::string Text, int R, int G, int B)
{
	APPLICATION.GetConsoleWindow()->SetNearestConsoleTextColor(R, G, B);
	std::cout << Text << std::endl;
	if (LOG.IsFileOutputActive())
		LOG.Add(Text, "CONSOLE_LOG");
	APPLICATION.GetConsoleWindow()->SetNearestConsoleTextColor(255, 255, 255);
}