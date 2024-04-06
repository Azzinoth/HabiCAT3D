#include "ConsoleJob.h"
using namespace FocalEngine;

ConsoleJob::ConsoleJob()
{
	ID = APPLICATION.GetUniqueHexID();
}

std::string ConsoleJob::GetID()
{
	return ID;
}

void ConsoleJob::OutputConsoleTextWithColor(std::string Text, int R, int G, int B)
{
	APPLICATION.GetConsoleWindow()->SetNearestConsoleTextColor(R, G, B);
	std::cout << Text << std::endl;
	APPLICATION.GetConsoleWindow()->SetNearestConsoleTextColor(255, 255, 255);
}