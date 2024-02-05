#pragma once
#include "LayerManager.h"
using namespace FocalEngine;

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Boolean_set_operations_2.h>
#include <CGAL/Polygon_set_2.h>
#include <CGAL/convexity_check_2.h>

namespace FocalEngine
{
#include "SphereVectors.h"

	class RugosityLayerProducer
	{
	public:
		SINGLETON_PUBLIC_PART(RugosityLayerProducer)

		bool bWeightedNormals = true;
		bool bNormalizedNormals = true;
		bool bDeleteOutliers = true;
		bool bOverlapAware = false;
		bool bCalculateStandardDeviation = false;
		std::vector<std::string> OrientationSetNamesForMinRugosityList;

		void CalculateWithJitterAsync();
		void CalculateOnWholeModel();

		std::vector<std::string> dimentionsList;
		std::vector<std::string> colorSchemesList;

		std::string colorSchemeIndexToString(int index);
		int colorSchemeIndexFromString(std::string name);

		bool GetUseFindSmallestRugosity();
		void SetUseFindSmallestRugosity(bool NewValue);

		bool GetUseCGALVariant();
		void SetUseCGALVariant(bool NewValue);

		float GetLastTimeTookForCalculation();

		void SetOnRugosityCalculationsStartCallback(void(*Func)(void));
		void SetOnRugosityCalculationsEndCallback(void(*Func)(MeshLayer));

		std::string GetUsedRugosityAlgorithmName();
		void SetUsedRugosityAlgorithmName(std::string name);
		std::vector<std::string> RugosityAlgorithmList;

		void RenderDebugInfoForSelectedNode(SDF* Grid);

		std::string GetOrientationSetForMinRugosityName();
		void SetOrientationSetForMinRugosityName(std::string name);
	private:
		SINGLETON_PRIVATE_PART(RugosityLayerProducer)

		int RugosityLayerIndex = 0;
		bool bUseFindSmallestRugosity = false;
		bool bUseCGALVariant = false;
		bool bWaitForJitterResult = false;
		std::string OrientationSetForMinRugosity = "91";

		static float LastTimeTookForCalculation;

		static void OnRugosityCalculationsStart();
		static void(*OnRugosityCalculationsStartCallbackImpl)(void);

		static void(*OnRugosityCalculationsEndCallbackImpl)(MeshLayer);

		static void CalculateOneNodeRugosity(SDFNode* CurrentNode);
		static void OnJitterCalculationsEnd(MeshLayer NewLayer);

		uint64_t StartTime;
	};

	#define RUGOSITY_LAYER_PRODUCER RugosityLayerProducer::getInstance()
}