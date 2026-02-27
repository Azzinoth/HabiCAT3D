#include "StructuralRoughnessLayerProducer.h"
using namespace FocalEngine;
#include "../../../UI/UIManager.h"

#include <CGAL/Eigen_matrix.h>
#include <CGAL/Eigen_svd.h>
typedef CGAL::Eigen_svd SVD;
#include <CGAL/Simple_cartesian.h>
typedef CGAL::Simple_cartesian<double> Kernel;
typedef Kernel::Point_3 Point_3;

StructuralRoughnessLayerProducer::StructuralRoughnessLayerProducer()
{
	JITTER_MANAGER.SetOnCalculationsEndCallback(OnJitterCalculationsEnd);

	if (!APPLICATION.HasConsoleWindow())
		DEVELOPER_MODE.AddOnDebugGridSelectedCellChangedCallback(OnDebugGridSelectedCellChanged);

	DebugPlaneEntity = MAIN_SCENE_MANAGER.GetMainScene()->CreateEntity("Structural Roughness Debug Plane");
	FEMesh* PlaneMesh = RESOURCE_MANAGER.GetMesh("1Y251E6E6T78013635793156"/*"plane"*/);
	FEMaterial* BlueMaterial = RESOURCE_MANAGER.CreateMaterial();
	BlueMaterial->Shader = RESOURCE_MANAGER.GetShader("6917497A5E0C05454876186F"/*"FESolidColorShader"*/);
	BlueMaterial->SetBaseColor(glm::vec3(0.0f, 0.0f, 0.6f));
	BlueMaterial->Shader->UpdateUniformData("BrightnessFactor", 1.0f);
	FEGameModel* BluePlaneGameModel = RESOURCE_MANAGER.CreateGameModel(PlaneMesh, BlueMaterial);
	DebugPlaneEntity->AddComponent<FEGameModelComponent>(BluePlaneGameModel);
	DebugPlaneEntity->SetComponentVisible(ComponentVisibilityType::ALL, false);
}

StructuralRoughnessLayerProducer::~StructuralRoughnessLayerProducer() {}

std::pair<glm::vec3, std::vector<glm::vec3>> StructuralRoughnessLayerProducer::GetPointsInCell(GridNode* Node)
{
	std::pair<glm::vec3, std::vector<glm::vec3>> Result;
	if (Node == nullptr)
		return Result;

	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr)
		return Result;

	PointCloudAnalysisData* CurrentPointCloudAnalysisData = ActiveObject->GetPointCloudAnalysisData();
	if (CurrentPointCloudAnalysisData == nullptr)
		return Result;

	if (Node->PointsInCell.empty())
		return Result;

	glm::vec3 Centroid(0.0f);

	for (size_t i = 0; i < Node->PointsInCell.size(); i++)
	{
		Centroid += glm::vec3(CurrentPointCloudAnalysisData->RawPointCloudData[Node->PointsInCell[i]].X,
							  CurrentPointCloudAnalysisData->RawPointCloudData[Node->PointsInCell[i]].Y,
							  CurrentPointCloudAnalysisData->RawPointCloudData[Node->PointsInCell[i]].Z) / static_cast<float>(Node->PointsInCell.size());

		Result.second.push_back(glm::vec3(CurrentPointCloudAnalysisData->RawPointCloudData[Node->PointsInCell[i]].X,
										  CurrentPointCloudAnalysisData->RawPointCloudData[Node->PointsInCell[i]].Y,
										  CurrentPointCloudAnalysisData->RawPointCloudData[Node->PointsInCell[i]].Z));
	}

	Result.first = Centroid;
	return Result;
}

glm::vec3 StructuralRoughnessLayerProducer::GetNormalOfBestFitPlane(const std::vector<glm::vec3>& Points)
{
	if (Points.size() < 3)
		return glm::vec3(0.0f);

	std::vector<Point_3> CGALPoints;
	Point_3 Centroid(0, 0, 0);
	size_t PointCount = Points.size();
	for (size_t i = 0; i < PointCount; i++)
	{
		glm::dvec3 CurrentPoint = glm::vec3(0.0);
		Point_3 CGALPoint(Points[i].x, Points[i].y, Points[i].z);
		CGALPoints.push_back(CGALPoint);

		Centroid = Point_3(Centroid.x() + CGALPoint.x() / PointCount,
						   Centroid.y() + CGALPoint.y() / PointCount,
						   Centroid.z() + CGALPoint.z() / PointCount);
	}

	SVD::Matrix CenteredPointsMatrix(PointCount, 3);
	for (std::size_t i = 0; i < PointCount; i++)
	{
		CenteredPointsMatrix.set(i, 0, CGALPoints[i].x() - Centroid.x());
		CenteredPointsMatrix.set(i, 1, CGALPoints[i].y() - Centroid.y());
		CenteredPointsMatrix.set(i, 2, CGALPoints[i].z() - Centroid.z());
	}

	// Perform SVD, computing thin U and V, for non thin computations we will not have enough memory.
	Eigen::BDCSVD<Eigen::MatrixXd> SVDDecomposition(CenteredPointsMatrix, Eigen::ComputeThinU | Eigen::ComputeThinV);

	Eigen::VectorXd SingularValues = SVDDecomposition.singularValues();
	Eigen::MatrixXd RightSingularVectors = SVDDecomposition.matrixV();

	// Find index of smallest singular value.
	Eigen::Index SmallestSingularValueIndex;
	SingularValues.minCoeff(&SmallestSingularValueIndex);

	Eigen::Vector3d PlaneNormal = RightSingularVectors.col(SmallestSingularValueIndex);
	PlaneNormal.normalize();

	return glm::vec3(PlaneNormal(0), PlaneNormal(1), PlaneNormal(2));
}

void StructuralRoughnessLayerProducer::WorkOnNode(GridNode* CurrentNode)
{
	// Structural Roughness is calculated as the RMS distance of points to their 
	// best-fit plane within each grid cell, following the methodology in:
	// Gu et al. (2024), Methods in Ecology and Evolution, 15(4), 639-646.

	if (CurrentNode->PointsInCell.empty() || CurrentNode->PointsInCell.size() < 3)
		return;

	std::pair<glm::vec3, std::vector<glm::vec3>> CellCentroidAndPoints = STRUCTURAL_ROUGHNESS_LAYER_PRODUCER.GetPointsInCell(CurrentNode);
	glm::vec3 Centroid = CellCentroidAndPoints.first;
	glm::vec3 BestFitPlaneNormal = STRUCTURAL_ROUGHNESS_LAYER_PRODUCER.GetNormalOfBestFitPlane(CellCentroidAndPoints.second);

	// Calculate structural roughness as RMS DistanceToPlane to plane.
	double SumSquaredDistances = 0.0;
	for (const auto& Point : CellCentroidAndPoints.second)
	{
		glm::vec3 VectorToPoint(Point.x - Centroid.x,
			Point.y - Centroid.y,
			Point.z - Centroid.z);

		double DistanceToPlane = std::abs(BestFitPlaneNormal.x * VectorToPoint.x +
			BestFitPlaneNormal.y * VectorToPoint.y +
			BestFitPlaneNormal.z * VectorToPoint.z);
		SumSquaredDistances += DistanceToPlane * DistanceToPlane;
	}

	CurrentNode->UserData = std::sqrt(SumSquaredDistances / CurrentNode->PointsInCell.size());
}

void StructuralRoughnessLayerProducer::CalculateWithJitterAsync(bool bSmootherResult)
{
	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr)
		return;

	PointCloudAnalysisData* CurrentPointCloudAnalysisData = ActiveObject->GetPointCloudAnalysisData();
	if (CurrentPointCloudAnalysisData == nullptr)
		return;

	bWaitForJitterResult = true;
	uint64_t StartTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);

	JITTER_MANAGER.CalculateWithGridJitterAsync(WorkOnNode, bSmootherResult);
}

void StructuralRoughnessLayerProducer::OnJitterCalculationsEnd(DataLayer* NewLayer)
{
	if (!STRUCTURAL_ROUGHNESS_LAYER_PRODUCER.bWaitForJitterResult)
		return;

	STRUCTURAL_ROUGHNESS_LAYER_PRODUCER.bWaitForJitterResult = false;

	AnalysisObject* CurrentObject = NewLayer->GetMainParentObject();
	if (CurrentObject == nullptr)
		return;

	NewLayer->SetType(LAYER_TYPE::STRUCTURAL_ROUGHNESS);
	NewLayer->DebugInfo->Type = "StructuralRoughnessDataLayerDebugInfo";
	NewLayer->SetCaption(LAYER_MANAGER.SuitableNewLayerCaption("Structural Roughness"));
	CurrentObject->AddLayer(NewLayer);
	CurrentObject->SetActiveLayer(NewLayer->GetID());
}

void StructuralRoughnessLayerProducer::RenderDebugInfoForSelectedNode(MeasurementGrid* Grid)
{
	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr)
		return;

	PointCloudAnalysisData* CurrentPointCloudAnalysisData = ActiveObject->GetPointCloudAnalysisData();
	if (CurrentPointCloudAnalysisData == nullptr)
		return;

	FEEntity* ActiveEntity = ANALYSIS_OBJECT_MANAGER.GetActiveEntity();
	if (ActiveEntity == nullptr)
		return;

	if (Grid == nullptr || Grid->SelectedCell == glm::vec3(-1.0))
		return;

	static glm::vec3 NormalDirection = glm::vec3(0.0f);

	if (ImGui::Begin("Structural Roughness Debug Info"))
	{
		ImGui::Text("Selected cell: X: %d Y: %d Z: %d", int(Grid->SelectedCell.x), int(Grid->SelectedCell.y), int(Grid->SelectedCell.z));

		FEAABB SelectedCellAABB = Grid->Data[int(Grid->SelectedCell.x)][int(Grid->SelectedCell.y)][int(Grid->SelectedCell.z)].AABB;

		static float PlaneScale = 1.0f;
		ImGui::SliderFloat("Debug Plane Scale", &PlaneScale, SelectedCellAABB.GetLongestAxisLength() / 5.0f, SelectedCellAABB.GetLongestAxisLength() * 2.0f);
		
		DebugPlaneEntity->GetComponent<FETransformComponent>().SetScale(glm::vec3(PlaneScale));

		std::pair<glm::vec3, std::vector<glm::vec3>> PointsInCell = GetPointsInCell(&Grid->Data[int(Grid->SelectedCell.x)][int(Grid->SelectedCell.y)][int(Grid->SelectedCell.z)]);
		glm::vec3 BestFitPlaneNormal = GetNormalOfBestFitPlane(PointsInCell.second);

		glm::vec3 PointsCentroid = PointsInCell.first;
		DebugPlaneEntity->GetComponent<FETransformComponent>().SetPosition(PointsCentroid);

		// Tweak current normal direction
		ImGui::SliderFloat3("Normal Direction", &NormalDirection.x, -1.0f, 1.0f);
		if (glm::length(NormalDirection) > 0.001f)
			NormalDirection = glm::normalize(NormalDirection);

		FEMesh* PlaneMesh = RESOURCE_MANAGER.GetMesh("1Y251E6E6T78013635793156"/*"plane"*/);
		FEAABB PlaneAABB = PlaneMesh->GetAABB();
		glm::vec3 PlaneNormal = glm::normalize(PlaneAABB.GetAproximateForwardDirection());
		glm::quat RotationQuaternion = glm::rotation(PlaneNormal, BestFitPlaneNormal);
		DebugPlaneEntity->GetComponent<FETransformComponent>().SetQuaternion(RotationQuaternion, FE_WORLD_SPACE);
	}

	ImGui::End();
}

void StructuralRoughnessLayerProducer::OnDebugGridSelectedCellChanged(glm::vec3 NewSelectedCell)
{
	MeasurementGrid* DebugGrid = DEVELOPER_MODE.GetDebugGrid();
	if (DebugGrid == nullptr)
	{
		STRUCTURAL_ROUGHNESS_LAYER_PRODUCER.DebugPlaneEntity->SetComponentVisible(ComponentVisibilityType::ALL, false);
		return;
	}

	if (NewSelectedCell == glm::vec3(-1.0))
	{
		STRUCTURAL_ROUGHNESS_LAYER_PRODUCER.DebugPlaneEntity->SetComponentVisible(ComponentVisibilityType::ALL, false);
		return;
	}

	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr)
	{
		STRUCTURAL_ROUGHNESS_LAYER_PRODUCER.DebugPlaneEntity->SetComponentVisible(ComponentVisibilityType::ALL, false);
		return;
	}

	DataLayer* ActiveLayer = ActiveObject->GetActiveLayer();
	if (ActiveLayer == nullptr || ActiveLayer->GetType() != LAYER_TYPE::STRUCTURAL_ROUGHNESS)
	{
		STRUCTURAL_ROUGHNESS_LAYER_PRODUCER.DebugPlaneEntity->SetComponentVisible(ComponentVisibilityType::ALL, false);
		return;
	}

	STRUCTURAL_ROUGHNESS_LAYER_PRODUCER.DebugPlaneEntity->SetComponentVisible(ComponentVisibilityType::ALL, true);
}

void StructuralRoughnessLayerProducer::CalculateOnEntireObject()
{
	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr)
		return;

	PointCloudAnalysisData* CurrentPointCloudAnalysisData = ActiveObject->GetPointCloudAnalysisData();
	if (CurrentPointCloudAnalysisData == nullptr)
		return;

	bWaitForJitterResult = true;
	uint64_t StartTime = TIME.GetTimeStamp(FE_TIME_RESOLUTION_NANOSECONDS);

	JITTER_MANAGER.SetFallbackValue(0.0f);
	JITTER_MANAGER.CalculateOnEntireObject(WorkOnNode);
}