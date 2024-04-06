#pragma once
#include "LayerManager.h"
using namespace FocalEngine;

#include "../../CGALDeclarations.h"
//#include <CGAL/Polygon_2.h>
//#include <CGAL/Boolean_set_operations_2.h>
//#include <CGAL/Polygon_set_2.h>
//#include <CGAL/convexity_check_2.h>
//
//#include "CGAL/Surface_mesh.h"
//#include "CGAL/Polygon_mesh_processing/polygon_soup_to_polygon_mesh.h"
//#include "CGAL/Polygon_mesh_processing/polygon_mesh_to_polygon_soup.h"
//#include "CGAL/Polygon_mesh_processing/repair_polygon_soup.h"
//
//#include "CGAL/Shape_detection/Region_growing/Region_growing_on_polygon_mesh/Least_squares_plane_fit_region.h"
//
//typedef CGAL::Simple_cartesian<double>					   Kernel;
//typedef Kernel::Point_3									   Point_3;
//typedef CGAL::Surface_mesh<Point_3>						   Surface_mesh;
//
//typedef std::vector<std::size_t>						   Polygon_3;
//namespace PMP = CGAL::Polygon_mesh_processing;
//
//typedef CGAL::Exact_predicates_exact_constructions_kernel  Kernel2;
//typedef Kernel2::Point_2                                   Point_2;
//typedef CGAL::Polygon_2<Kernel2>                           Polygon_2;
//typedef std::vector<Polygon_2>                             Polygon_vector;
//typedef CGAL::Polygon_set_2<Kernel2>                       Polygon_set_2;
//
//#define CGAL_FOR_PROJECTION
//
//#ifdef CGAL_FOR_PROJECTION
//#include <CGAL/Simple_cartesian.h>
//#include <CGAL/Plane_3.h>
//
//typedef Kernel::Point_3 Point_3;
//typedef Kernel::Vector_3 Vector_3;
//typedef Kernel::Plane_3 Plane_3;
//#endif

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

		bool ShouldDeletedOutlier();
		void SetDeleteOutliers(bool NewValue);

		bool ShouldCalculateStandardDeviation();
		void SetCalculateStandardDeviation(bool NewValue);

		// Rugosity calculation control methods
		bool GetUseFindSmallestRugosity();
		void SetUseFindSmallestRugosity(bool NewValue);
		bool GetUseCGALVariant();
		void SetUseCGALVariant(bool NewValue);
		bool GetIsUsingUniqueProjectedArea();
		void SetIsUsingUniqueProjectedArea(bool NewValue);

		// Asynchronous calculation methods
		void CalculateWithJitterAsync();
		void CalculateOnWholeModel();

		// Callback setters
		void SetOnRugosityCalculationsStartCallback(void(*Func)(void));
		void SetOnRugosityCalculationsEndCallback(void(*Func)(MeshLayer));

		std::string GetUsedRugosityAlgorithmName();
		void SetUsedRugosityAlgorithmName(std::string name);

		std::string GetOrientationSetForMinRugosityName();
		void SetOrientationSetForMinRugosityName(std::string name);
		std::vector<std::string> GetOrientationSetNamesForMinRugosityList();

		std::vector<std::string> GetRugosityAlgorithmList();

		void RenderDebugInfoForSelectedNode(MeasurementGrid* Grid);
		float GetLastTimeTookForCalculation();
	private:
		SINGLETON_PRIVATE_PART(RugosityLayerProducer)

		// Configuration flags
		bool bWeightedNormals = true;
		bool bNormalizedNormals = true;
		bool bDeleteOutliers = true;
		bool bCalculateStandardDeviation = false;
		bool bUseFindSmallestRugosity = false;
		bool bUseCGALVariant = false;
		bool bWaitForJitterResult = false;
		bool bUniqueProjectedArea = false;

		// Rugosity and orientation set configuration
		std::string OrientationSetForMinRugosity = "91";
		std::vector<std::string> RugosityAlgorithmList;
		std::vector<std::string> OrientationSetNamesForMinRugosityList;

		// Time and callback handling
		static float LastTimeTookForCalculation;
		static void OnRugosityCalculationsStart();
		static void(*OnRugosityCalculationsStartCallbackImpl)(void);
		static void(*OnRugosityCalculationsEndCallbackImpl)(MeshLayer);

		// Internal calculation methods
		static void CalculateOneNodeRugosity(GridNode* CurrentNode);
		static void OnJitterCalculationsEnd(MeshLayer NewLayer);
		double CGALCalculateArea(const Polygon_set_2& polygon_set);
		void CreateLocalCoordinateSystem(const glm::dvec3& Normal, glm::dvec3& U, glm::dvec3& V);
		Point_2 ProjectToLocalCoordinates(const glm::dvec3& Point, const glm::dvec3& U, const glm::dvec3& V);

		uint64_t StartTime;
	};

	#define RUGOSITY_LAYER_PRODUCER RugosityLayerProducer::getInstance()
}