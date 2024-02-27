#include "FECGALWrapper.h"
using namespace FocalEngine;

FECGALWrapper* FECGALWrapper::Instance = nullptr;

FECGALWrapper::FECGALWrapper(){}
FECGALWrapper::~FECGALWrapper(){}

FEMesh* FECGALWrapper::surfaceMeshToFEMesh(Surface_mesh mesh)
{
	FEMesh* result = nullptr;

	// Extracting data from Surface_mesh.
	std::vector<Point_3> extractedPoints;
	std::vector<Polygon_3> extractedFaces;
	PMP::polygon_mesh_to_polygon_soup(mesh, extractedPoints, extractedFaces);
	PMP::repair_polygon_soup(extractedPoints, extractedFaces);

	// Formating data to FE format.
	std::vector<int> FEIndices;
	for (size_t i = 0; i < extractedFaces.size(); i++)
	{
		FEIndices.push_back(int(extractedFaces[i][0]));
		FEIndices.push_back(int(extractedFaces[i][1]));
		FEIndices.push_back(int(extractedFaces[i][2]));
	}

	std::vector<float> FEVertices;
	for (size_t i = 0; i < extractedPoints.size(); i++)
	{
		FEVertices.push_back(float(extractedPoints[i][0]));
		FEVertices.push_back(float(extractedPoints[i][1]));
		FEVertices.push_back(float(extractedPoints[i][2]));
	}

	result = RESOURCE_MANAGER.RawDataToMesh(FEVertices.data(), int(FEVertices.size()),
											nullptr, 0,
											nullptr, 0,
											nullptr, 0,
											FEIndices.data(), int(FEIndices.size()),
											nullptr, 0,
											nullptr, 0, 0, "");

	return result;
}

Surface_mesh FECGALWrapper::FEMeshToSurfaceMesh(FEMesh* mesh)
{
	// Extracting data from FEMesh.
	std::vector<float> FEVertices;
	FEVertices.resize(mesh->GetPositionsCount());
	FE_GL_ERROR(glGetNamedBufferSubData(mesh->GetPositionsBufferID(), 0, sizeof(float) * FEVertices.size(), FEVertices.data()));

	std::vector<int> FEIndices;
	FEIndices.resize(mesh->GetIndicesCount());
	FE_GL_ERROR(glGetNamedBufferSubData(mesh->GetIndicesBufferID(), 0, sizeof(int) * FEIndices.size(), FEIndices.data()));

	// Formating data to CGAL format.
	std::vector<Polygon_3> CGALFaces;
	CGALFaces.resize(FEIndices.size() / 3);
	int count = 0;
	for (size_t i = 0; i < FEIndices.size(); i += 3)
	{
		CGALFaces[count].push_back(FEIndices[i]);
		CGALFaces[count].push_back(FEIndices[i + 1]);
		CGALFaces[count].push_back(FEIndices[i + 2]);
		count++;
	}

	std::vector<Point_3> CGALPoints;
	for (size_t i = 0; i < FEVertices.size(); i += 3)
	{
		CGALPoints.push_back(Point_3(FEVertices[i], FEVertices[i + 1], FEVertices[i + 2]));
	}

	Surface_mesh result;

	if (!PMP::is_polygon_soup_a_polygon_mesh(CGALFaces))
	{
		PMP::repair_polygon_soup(CGALPoints, CGALFaces);
		PMP::orient_polygon_soup(CGALPoints, CGALFaces);
	}

	PMP::polygon_soup_to_polygon_mesh(CGALPoints, CGALFaces, result);
	return result;
}

void FECGALWrapper::saveSurfaceMeshToOBJFile(std::string fileName, Surface_mesh mesh)
{
	std::vector<Point_3> extractedPoints;
	std::vector<Polygon_3> extractedFaces;
	PMP::polygon_mesh_to_polygon_soup(mesh, extractedPoints, extractedFaces);
	PMP::repair_polygon_soup(extractedPoints, extractedFaces);

	CGAL::IO::write_OBJ(fileName, extractedPoints, extractedFaces);
}

FEMesh* FECGALWrapper::SurfaceMeshSimplification(FEMesh* originalMesh, double verticesLeftInPersent)
{
	if (verticesLeftInPersent == 0)
		return nullptr;

	Surface_mesh surface_mesh = FEMeshToSurfaceMesh(originalMesh);

	// In this example, the simplification stops when the number of undirected edges
	// drops below 10% of the initial count
	double stop_ratio = verticesLeftInPersent;
	SMS::Count_ratio_stop_predicate<Surface_mesh> stop(stop_ratio);

	int r = SMS::edge_collapse(surface_mesh, stop);

	//saveSurfaceMeshToOBJFile("C:/Users/kandr/Downloads/OBJ_Models/simplified.obj", surface_mesh);

	return surfaceMeshToFEMesh(surface_mesh);
}

void FECGALWrapper::addRugosityInfo(FEMesh* mesh, std::vector<int> originalTrianglesToSegments, std::vector<glm::vec3> segmentsNormals)
{
	int posSize = mesh->GetPositionsCount();
	float* positions = new float[posSize];
	FE_GL_ERROR(glGetNamedBufferSubData(mesh->GetPositionsBufferID(), 0, sizeof(float) * posSize, positions));

	int UVSize = mesh->GetUVCount();
	float* UV = new float[UVSize];
	FE_GL_ERROR(glGetNamedBufferSubData(mesh->GetUVBufferID(), 0, sizeof(float) * UVSize, UV));

	int normSize = mesh->GetNormalsCount();
	float* normalsFloat = new float[normSize];
	FE_GL_ERROR(glGetNamedBufferSubData(mesh->GetNormalsBufferID(), 0, sizeof(float) * normSize, normalsFloat));

	int tanSize = mesh->GetTangentsCount();
	float* tangents = new float[tanSize];
	FE_GL_ERROR(glGetNamedBufferSubData(mesh->GetTangentsBufferID(), 0, sizeof(float) * tanSize, tangents));

	int indexSize = mesh->GetIndicesCount();
	int* indices = new int[indexSize];
	FE_GL_ERROR(glGetNamedBufferSubData(mesh->GetIndicesBufferID(), 0, sizeof(int) * indexSize, indices));

	std::vector<float> positionsVector;
	for (size_t i = 0; i < posSize; i++)
	{
		positionsVector.push_back(positions[i]);
	}

	std::vector<int> indexVector;
	for (size_t i = 0; i < indexSize; i++)
	{
		indexVector.push_back(indices[i]);
	}

	int colorSize = posSize;
	float* colors = new float[posSize];
	int vertexIndex = 0;

	/*std::string outputString = "";
	for (size_t i = 0; i < originalTrianglesToSegments.size(); i++)
	{
		outputString += "facet " + std::to_string(i) + ": " + std::to_string(originalTrianglesToSegments[i]) + "\n";
	}*/

	auto setColorOfVertex = [&](int index, glm::vec3 color) {
		colors[index * 3] = color.x;
		colors[index * 3 + 1] = color.y;
		colors[index * 3 + 2] = color.z;
	};

	auto getVertexOfFace = [&](int faceIndex) {
		std::vector<int> result;
		result.push_back(indexVector[faceIndex * 3]);
		result.push_back(indexVector[faceIndex * 3 + 1]);
		result.push_back(indexVector[faceIndex * 3 + 2]);

		return result;
	};

	auto setColorOfFace = [&](int faceIndex, glm::vec3 color) {
		std::vector<int> faceVertex = getVertexOfFace(faceIndex);

		for (size_t i = 0; i < faceVertex.size(); i++)
		{
			setColorOfVertex(faceVertex[i], color);
		}
	};

	auto getMinDistanceToColorsInUse = [&](glm::vec3 colorToTest, std::vector<glm::vec3> colorsInUse) {
		float minDistance = FLT_MAX;
		for (size_t i = 0; i < colorsInUse.size(); i++)
		{
			float currentDistance = glm::distance(colorToTest, colorsInUse[i]);
			if (currentDistance < minDistance)
				minDistance = currentDistance;
		}

		return minDistance;
	};

	auto randColor = []() {
		glm::vec3 result;

		float randR = float(rand() % 100) / 100.0f;
		float randG = float(rand() % 100) / 100.0f;
		float randB = float(rand() % 100) / 100.0f;

		result = glm::vec3(randR, randG, randB);
		return result;
	};

	auto getRandColor = [&](std::vector<glm::vec3> colorsInUse) {

		glm::vec3 currentColor = randColor();

		for (size_t i = 0; i < 1000; i++)
		{
			if (getMinDistanceToColorsInUse(currentColor, colorsInUse) > 0.2)
				return currentColor;

			currentColor = randColor();
		}

		return currentColor;
	};

	std::vector<glm::vec3> usedColors;
	std::unordered_map<int, glm::vec3> segIDColors;
	for (int i = 0; i < originalTrianglesToSegments.size(); i++)
	{
		if (segIDColors.find(originalTrianglesToSegments[i]) == segIDColors.end())
		{
			glm::vec3 newColor = getRandColor(usedColors);
			segIDColors[originalTrianglesToSegments[i]] = newColor;
			usedColors.push_back(newColor);
		}

		setColorOfFace(i, segIDColors[originalTrianglesToSegments[i]]);
	}

	int segmentsColorsSize = posSize;
	float* segmentsColors = new float[posSize];

	for (size_t i = 0; i < posSize; i++)
	{
		segmentsColors[i] = colors[i];
	}

	std::unordered_map<int, double> originalTrianglesRugosity;
	std::unordered_map<int, double> sectorRugositySum;
	std::unordered_map<int, int> sectorsCount;
	double totalArea = 0.0f;

	double totalAreaTEST = 0.0f;
	double totalAreaTEST1 = 0.0f;

	for (int i = 0; i < originalTrianglesToSegments.size(); i++)
	{
		glm::vec3 segmentNormal = segmentsNormals[originalTrianglesToSegments[i]];
		std::vector<int> points = getVertexOfFace(i);

		glm::vec3 a = glm::vec3(positionsVector[points[0] * 3], positionsVector[points[0] * 3 + 1], positionsVector[points[0] * 3 + 2]);
		glm::vec3 b = glm::vec3(positionsVector[points[1] * 3], positionsVector[points[1] * 3 + 1], positionsVector[points[1] * 3 + 2]);
		glm::vec3 c = glm::vec3(positionsVector[points[2] * 3], positionsVector[points[2] * 3 + 1], positionsVector[points[2] * 3 + 2]);
		double originalArea = SDF::TriangleArea(a, b, c);
		totalArea += originalArea;

		FEPlane currentPlane = FEPlane(a, segmentNormal);

		glm::vec3 aProjection = currentPlane.ProjectPoint(a);
		glm::vec3 bProjection = currentPlane.ProjectPoint(b);
		glm::vec3 cProjection = currentPlane.ProjectPoint(c);

		double projectionArea = SDF::TriangleArea(aProjection, bProjection, cProjection);

		double rugosity = originalArea / projectionArea;

		sectorRugositySum[originalTrianglesToSegments[i]] += rugosity;
		sectorsCount[originalTrianglesToSegments[i]]++;

		originalTrianglesRugosity[i] += rugosity;
	}

	double minRugorsity = DBL_MAX;
	double maxRugorsity = -DBL_MAX;
	std::vector<double> sectorsRugorsity;
	sectorsRugorsity.resize(segmentsNormals.size());
	auto it = sectorRugositySum.begin();
	while (it != sectorRugositySum.end())
	{
		sectorsRugorsity[it->first] = it->second / sectorsCount[it->first];

		if (sectorsRugorsity[it->first] > maxRugorsity)
			maxRugorsity = sectorsRugorsity[it->first];

		if (sectorsRugorsity[it->first] < minRugorsity)
			minRugorsity = sectorsRugorsity[it->first];

		it++;
	}

	glm::vec3 darkBlue = glm::vec3(0.0f, 0.0f, 0.4f);
	glm::vec3 lightCyan = glm::vec3(27.0f / 255.0f, 213.0f / 255.0f, 200.0f / 255.0f);
	glm::vec3 green = glm::vec3(0.0f / 255.0f, 255.0f / 255.0f, 64.0f / 255.0f);
	glm::vec3 yellow = glm::vec3(225.0f / 255.0f, 225.0f / 255.0f, 0.0f / 255.0f);
	glm::vec3 red = glm::vec3(225.0f / 255.0f, 0 / 255.0f, 0.0f / 255.0f);

	for (int i = 0; i < originalTrianglesToSegments.size(); i++)
	{
		//double normalizedRugorsity = (sectorsRugorsity[originalTrianglesToSegments[i]] - minRugorsity) / (maxRugorsity - minRugorsity);
		double normalizedRugorsity = sectorsRugorsity[originalTrianglesToSegments[i]];

		if (normalizedRugorsity <= 1.2 && normalizedRugorsity > 1.0)
		{
			setColorOfFace(i, darkBlue);
		}
		else if (normalizedRugorsity <= 2.5 && normalizedRugorsity > 1.2)
		{
			setColorOfFace(i, lightCyan);
		}
		else if (normalizedRugorsity <= 3.5 && normalizedRugorsity > 2.5)
		{
			setColorOfFace(i, green);
		}
		else if (normalizedRugorsity <= 4.5 && normalizedRugorsity > 3.5)
		{
			setColorOfFace(i, yellow);
		}
		else if (normalizedRugorsity <= 10.0 && normalizedRugorsity > 4.5)
		{
			setColorOfFace(i, red);
		}
	}

	RESOURCE_MANAGER.AddColorToFEMeshVertices(mesh, colors, colorSize);
	// FIX ME
	//mesh->addSegmentsColorToVertices(segmentsColors, segmentsColorsSize);

	COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->originalTrianglesToSegments = originalTrianglesToSegments;
	COMPLEXITY_METRIC_MANAGER.ActiveComplexityMetricInfo->segmentsNormals = segmentsNormals;
}

std::vector<float> FECGALWrapper::calculateNormals(Surface_mesh mesh)
{
	Surface_mesh::Property_map<face_descriptor, Kernel::Vector_3 > face_normals =
		mesh.add_property_map<face_descriptor, Kernel::Vector_3 >("f:normal").first;


	CGAL::Polygon_mesh_processing::compute_face_normals(mesh, face_normals);

	std::vector<Point_3> extractedPoints;
	std::vector<Polygon_3> extractedFaces;
	PMP::polygon_mesh_to_polygon_soup(mesh, extractedPoints, extractedFaces);

	//glm::vec3 point_3 = glm::vec3(anchors[3][0], anchors[3][1], anchors[3][2]);
	//glm::vec3 point_2 = glm::vec3(anchors[2][0], anchors[2][1], anchors[2][2]);
	//glm::vec3 point_1 = glm::vec3(anchors[1][0], anchors[1][1], anchors[1][2]);

	//glm::vec3 edge_0 = point_3 - point_2;
	//glm::vec3 edge_1 = point_3 - point_1;

	//glm::vec3 normal = glm::normalize(glm::cross(edge_0, edge_1));



	//std::string normalsList;

	//for (size_t i = 0; i < extractedFaces/*polygons*/.size(); i++)
	//{
	//	normalsList += "vn " + std::to_string(face_normals.data()[i].x());
	//	normalsList += " " + std::to_string(face_normals.data()[i].y());
	//	normalsList += " " + std::to_string(face_normals.data()[i].z());
	//	normalsList += "\n";
	//}

	std::vector<float> calculatedNormals;
	calculatedNormals.resize(extractedPoints.size() * 3);
	for (int i = 0; i < extractedFaces.size(); i++)
	{
		int vertexIndex = int(extractedFaces[i][0] * 3);
		calculatedNormals[vertexIndex + 0] = float(face_normals.data()[i].x());
		calculatedNormals[vertexIndex + 1] = float(face_normals.data()[i].y());
		calculatedNormals[vertexIndex + 2] = float(face_normals.data()[i].z());

		vertexIndex = int(extractedFaces[i][1] * 3);
		calculatedNormals[vertexIndex + 0] = float(face_normals.data()[i].x());
		calculatedNormals[vertexIndex + 1] = float(face_normals.data()[i].y());
		calculatedNormals[vertexIndex + 2] = float(face_normals.data()[i].z());

		vertexIndex = int(extractedFaces[i][2] * 3);
		calculatedNormals[vertexIndex + 0] = float(face_normals.data()[i].x());
		calculatedNormals[vertexIndex + 1] = float(face_normals.data()[i].y());
		calculatedNormals[vertexIndex + 2] = float(face_normals.data()[i].z());
	}

	return calculatedNormals;
}

FEMesh* FECGALWrapper::SurfaceMeshApproximation(FEMesh* originalMesh, int segmentsMax)
{
	if (segmentsMax <= 0)
		return nullptr;

	Surface_mesh mesh = FEMeshToSurfaceMesh(originalMesh);

	// output face proxy index property map
	Face_proxy_pmap fpxmap = mesh.add_property_map<face_descriptor, std::size_t>("f:proxy_id", 0).first;
	Face_proxy_pmap _map = mesh.add_property_map<face_descriptor, std::size_t>("f:partition_id", 0).first;

	// The face <--> partition_id property map
	typedef CGAL::Simple_cartesian<double>                           K;
	typedef CGAL::Surface_mesh<K::Point_3>                           SM;
	typedef SM::Property_map<SM::Face_index, std::size_t>            Face_id_map;
	Face_id_map face_pid_map = mesh.add_property_map<SM::Face_index, std::size_t>("f:pid").first;

	// output planar proxies
	std::vector<Kernel::Vector_3> proxies;

	// The output will be an indexed triangle mesh
	std::vector<Kernel::Point_3> anchors;
	std::vector<std::array<std::size_t, 3> > triangles;
	// free function interface with named parameters
	VSA::approximate_triangle_mesh(mesh,
		//CGAL::parameters::seeding_method(VSA::HIERARCHICAL).
		CGAL::parameters::verbose_level(VSA::MAIN_STEPS).
		//CGAL::parameters::min_error_drop(0.95).
		max_number_of_proxies(segmentsMax).
		face_proxy_map(fpxmap).proxies(std::back_inserter(proxies)).	//face_partition_id_map(_map).
		anchors(std::back_inserter(anchors)). // anchor points
		triangles(std::back_inserter(triangles))); // indexed triangles

	PMP::orient_polygon_soup(anchors, triangles);

	std::vector<int> originalMeshToSegments;
	auto iterator = fpxmap.begin();
	while (iterator != fpxmap.end())
	{
		originalMeshToSegments.push_back(int(iterator._Ptr[0]));
		iterator++;
	}

	

	Surface_mesh output;

	std::vector<Polygon_3> polygons;
	polygons.resize(triangles.size());
	for (size_t i = 0; i < triangles.size(); i++)
	{	
		polygons[i].push_back(triangles[i][0]);
		polygons[i].push_back(triangles[i][1]);
		polygons[i].push_back(triangles[i][2]);
	}
	PMP::repair_polygon_soup(anchors, polygons);
	PMP::orient_polygon_soup(anchors, polygons);


	std::vector<glm::vec3> anchorsVector;
	for (size_t i = 0; i < anchors.size(); i++)
	{
		anchorsVector.push_back(glm::vec3(anchors[i].x(), anchors[i].y(), anchors[i].z()));
	}

	std::vector<glm::vec3> proxiesVector;
	for (size_t i = 0; i < proxies.size(); i++)
	{
		proxiesVector.push_back(glm::vec3(proxies[i].x(), proxies[i].y(), proxies[i].z()));
	}

	//PMP::polygon_soup_to_polygon_mesh(anchors, triangles, output);
	PMP::polygon_soup_to_polygon_mesh(anchors, polygons, output);
	if (CGAL::is_closed(output) && (!PMP::is_outward_oriented(output)))
		PMP::reverse_face_orientations(output);


	std::vector<std::vector<glm::vec3>> trianglesNew;
	std::vector<glm::vec3> normalsNew;
	std::vector<int> newTrianglesToSegments;

	for (size_t i = 0; i < polygons.size(); i++)
	{
		std::vector<glm::vec3> triangle;
		triangle.push_back(anchorsVector[polygons[i][0]]);
		triangle.push_back(anchorsVector[polygons[i][1]]);
		triangle.push_back(anchorsVector[polygons[i][2]]);

		trianglesNew.push_back(triangle);

		glm::vec3 edge_0 = trianglesNew.back()[2] - trianglesNew.back()[1];
		glm::vec3 edge_1 = trianglesNew.back()[2] - trianglesNew.back()[0];

		glm::vec3 normal = glm::normalize(glm::cross(edge_0, edge_1));
		normalsNew.push_back(normal);

		double minError = 99999.0;
		int minErrorIndex = -1;
		for (int j = 0; j < proxies.size(); j++)
		{
			double currentError = glm::length(abs(proxiesVector[j] - normalsNew.back()));
			if (currentError < minError)
			{
				minError = currentError;
				minErrorIndex = j;
			}
		}

		newTrianglesToSegments.push_back(minErrorIndex);
	}

	saveSurfaceMeshToOBJFile("C:/Users/Kindr/Downloads/OBJ_Models/simplified.obj", output);

	addRugosityInfo(originalMesh, originalMeshToSegments, proxiesVector);
	std::vector<float> calculatedNormals = calculateNormals(output);

	std::vector<glm::vec3> normalsNewNEW;
	for (size_t i = 0; i < calculatedNormals.size(); i+=3)
	{
		normalsNewNEW.push_back(glm::vec3(calculatedNormals[i], calculatedNormals[i + 1], calculatedNormals[i + 2]));
	}

	return surfaceMeshToFEMesh(output, calculatedNormals.data(), int(calculatedNormals.size()));
}

FEMesh* FECGALWrapper::surfaceMeshToFEMesh(Surface_mesh mesh, float* normals, int normSize)
{
	FEMesh* result = nullptr;

	// Extracting data from Surface_mesh.
	std::vector<Point_3> extractedPoints;
	std::vector<Polygon_3> extractedFaces;
	PMP::polygon_mesh_to_polygon_soup(mesh, extractedPoints, extractedFaces);
	PMP::repair_polygon_soup(extractedPoints, extractedFaces);

	// Formating data to FE format.
	std::vector<int> FEIndices;
	for (size_t i = 0; i < extractedFaces.size(); i++)
	{
		FEIndices.push_back(int(extractedFaces[i][0]));
		FEIndices.push_back(int(extractedFaces[i][1]));
		FEIndices.push_back(int(extractedFaces[i][2]));
	}

	std::vector<float> FEVertices;
	for (size_t i = 0; i < extractedPoints.size(); i++)
	{
		FEVertices.push_back(float(extractedPoints[i][0]));
		FEVertices.push_back(float(extractedPoints[i][1]));
		FEVertices.push_back(float(extractedPoints[i][2]));
	}

	result = RESOURCE_MANAGER.RawDataToMesh(FEVertices.data(), int(FEVertices.size()),
											nullptr, 0,
											normals, normSize,
											nullptr, 0,
											FEIndices.data(), int(FEIndices.size()),
											nullptr, 0,
											nullptr, 0, 0, "");

	return result;
}