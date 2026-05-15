#include "AnalysisObjectTests.h"

TEST(AnalysisObject, AddLayer_IncreasesCount_AndLayerIsRetrievable)
{
	AnalysisObject* TestObject = new AnalysisObject();
	ASSERT_EQ(TestObject->GetLayerCount(), 0);

	DataLayer* NewLayer = TEST_TOOLS.CreateSyntheticDataLayer({ 1.0f, 2.0f, 3.0f }, "LayerA");
	std::string LayerID = NewLayer->GetID();

	ASSERT_TRUE(TestObject->AddLayer(NewLayer));
	ASSERT_EQ(TestObject->GetLayerCount(), 1);
	ASSERT_EQ(TestObject->GetLayer(LayerID), NewLayer);

	delete TestObject;
}

TEST(AnalysisObject, SetActiveLayer_SwitchesActiveLayer_AndRejectsInvalidID)
{
	AnalysisObject* TestObject = new AnalysisObject();

	DataLayer* LayerA = TEST_TOOLS.CreateSyntheticDataLayer({ 1.0f }, "LayerA");
	DataLayer* LayerB = TEST_TOOLS.CreateSyntheticDataLayer({ 2.0f }, "LayerB");
	ASSERT_TRUE(TestObject->AddLayer(LayerA));
	ASSERT_TRUE(TestObject->AddLayer(LayerB));

	ASSERT_TRUE(TestObject->SetActiveLayer(LayerB->GetID()));
	ASSERT_EQ(TestObject->GetActiveLayer(), LayerB);

	ASSERT_TRUE(TestObject->SetActiveLayer(LayerA->GetID()));
	ASSERT_EQ(TestObject->GetActiveLayer(), LayerA);

	// Invalid IDs must not change the active layer.
	ASSERT_FALSE(TestObject->SetActiveLayer("not-a-real-id"));
	ASSERT_EQ(TestObject->GetActiveLayer(), LayerA);

	delete TestObject;
}

TEST(AnalysisObject, SetActiveLayer_EmptyStringClearsActiveLayer)
{
	AnalysisObject* TestObject = new AnalysisObject();

	DataLayer* LayerA = TEST_TOOLS.CreateSyntheticDataLayer({ 1.0f }, "LayerA");
	ASSERT_TRUE(TestObject->AddLayer(LayerA));
	ASSERT_TRUE(TestObject->SetActiveLayer(LayerA->GetID()));
	ASSERT_EQ(TestObject->GetActiveLayer(), LayerA);

	TestObject->SetActiveLayer("");
	ASSERT_EQ(TestObject->GetActiveLayer(), nullptr);

	delete TestObject;
}

TEST(AnalysisObject, AddLayer_DoesNotImplicitlyPromoteToActive)
{
	AnalysisObject* TestObject = new AnalysisObject();
	ASSERT_EQ(TestObject->GetActiveLayer(), nullptr);

	DataLayer* LayerA = TEST_TOOLS.CreateSyntheticDataLayer({ 1.0f }, "LayerA");
	ASSERT_TRUE(TestObject->AddLayer(LayerA));
	EXPECT_EQ(TestObject->GetActiveLayer(), nullptr);

	delete TestObject;
}

TEST(AnalysisObject, RemoveLayer_RemovesAndCannotBeRetrieved)
{
	AnalysisObject* TestObject = new AnalysisObject();

	DataLayer* LayerA = TEST_TOOLS.CreateSyntheticDataLayer({ 1.0f }, "LayerA");
	std::string LayerID = LayerA->GetID();
	ASSERT_TRUE(TestObject->AddLayer(LayerA));
	ASSERT_EQ(TestObject->GetLayerCount(), 1);

	ASSERT_TRUE(TestObject->RemoveLayer(LayerID));
	ASSERT_EQ(TestObject->GetLayerCount(), 0);
	ASSERT_EQ(TestObject->GetLayer(LayerID), nullptr);
	ASSERT_FALSE(TestObject->RemoveLayer(LayerID));

	delete TestObject;
}

TEST(AnalysisObject, Layers_AreOwnedPerObject_NotShared)
{
	AnalysisObject* FirstObject = new AnalysisObject();
	AnalysisObject* SecondObject = new AnalysisObject();

	DataLayer* LayerOnFirst = TEST_TOOLS.CreateSyntheticDataLayer({ 1.0f }, "OnlyOnFirst");
	ASSERT_TRUE(FirstObject->AddLayer(LayerOnFirst));

	ASSERT_EQ(FirstObject->GetLayerCount(), 1);
	ASSERT_EQ(SecondObject->GetLayerCount(), 0);
	ASSERT_EQ(SecondObject->GetLayer(LayerOnFirst->GetID()), nullptr);

	delete FirstObject;
	delete SecondObject;
}