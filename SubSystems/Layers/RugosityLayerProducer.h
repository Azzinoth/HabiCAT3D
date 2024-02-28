#pragma once
#include "LayerManager.h"
using namespace FocalEngine;

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Boolean_set_operations_2.h>
#include <CGAL/Polygon_set_2.h>
#include <CGAL/convexity_check_2.h>

#include "CGAL/Surface_mesh.h"
#include "CGAL/Surface_mesh_simplification/edge_collapse.h"
#include "CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Count_ratio_stop_predicate.h"

#include "CGAL/Polygon_mesh_processing/polygon_soup_to_polygon_mesh.h"
#include "CGAL/Polygon_mesh_processing/polygon_mesh_to_polygon_soup.h"
#include "CGAL/Polygon_mesh_processing/repair_polygon_soup.h"

#include "CGAL/Shape_detection/Region_growing/Region_growing_on_polygon_mesh/Least_squares_plane_fit_region.h"

typedef CGAL::Simple_cartesian<double>  Kernel;
typedef Kernel::Point_3                 Point_3;
typedef CGAL::Surface_mesh<Point_3>		Surface_mesh;

typedef std::vector<std::size_t>		Polygon_3;
namespace PMP = CGAL::Polygon_mesh_processing;

namespace FocalEngine
{
#include "SphereVectors.h"

	struct FEPlane
	{
		glm::dvec3 PointOnPlane;
		glm::dvec3 Normal;
		float Distance = 0.0f;

		FEPlane(const glm::dvec3 PointOnPlane, const glm::dvec3 Normal)
		{
			this->PointOnPlane = PointOnPlane;
			this->Normal = Normal;

			// Distance is the length of the perpendicular line from the origin to the plane.
			double PlaneD = glm::length(glm::dot(PointOnPlane, Normal));
		}

		glm::vec3 ProjectPoint(const glm::dvec3& Point) const
		{
			// 3D Math Primer, page 719
			// Plane equation :
			// dot(p, n) = d
			// Formula to project point q on plane:
			// q' = q + (d - dot(q, n)) * n

			return Point + (Distance - glm::dot(Point, Normal)) * Normal;
		}
	};

	class RugosityLayerProducer
	{
	public:
		SINGLETON_PUBLIC_PART(RugosityLayerProducer)

		bool bWeightedNormals = true;
		bool bNormalizedNormals = true;
		bool bDeleteOutliers = true;
		
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

		bool GetIsUsingUniqueProjectedArea();
		void SetIsUsingUniqueProjectedArea(bool NewValue);
	private:
		SINGLETON_PRIVATE_PART(RugosityLayerProducer)

		int RugosityLayerIndex = 0;
		bool bUseFindSmallestRugosity = false;
		bool bUseCGALVariant = false;
		bool bWaitForJitterResult = false;
		bool bUniqueProjectedArea = false;
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