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