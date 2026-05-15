#pragma once

#include "../SubSystems/AnalysisObjectManager.h"

class TestTools
{
	SINGLETON_PRIVATE_PART(TestTools)

public:
	SINGLETON_PUBLIC_PART(TestTools)

	// Creates a free standing DataLayer for unit tests. Not attached to any AnalysisObject.
	// Caller takes ownership and must delete (or hand off via AnalysisObject::AddLayer).
	DataLayer* CreateSyntheticDataLayer(std::vector<float> Values, std::string Caption = "TestLayer");
};

#define TEST_TOOLS TestTools::GetInstance()
