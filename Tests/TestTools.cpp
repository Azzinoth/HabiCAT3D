#include "TestTools.h"

TestTools::TestTools() {}
TestTools::~TestTools() {}

DataLayer* TestTools::CreateSyntheticDataLayer(std::vector<float> Values, std::string Caption)
{
	DataLayer* NewLayer = new DataLayer(std::vector<FEUUID>{}, Values);
	NewLayer->SetCaption(Caption);
	return NewLayer;
}
