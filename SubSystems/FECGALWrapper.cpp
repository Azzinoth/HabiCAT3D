#include "FECGALWrapper.h"
using namespace FocalEngine;

FECGALWrapper* FECGALWrapper::_instance = nullptr;

FECGALWrapper::FECGALWrapper()
{

}

FECGALWrapper::~FECGALWrapper()
{

}

FEMesh* FECGALWrapper::rawDataToMesh(float* positions, int posSize,
									 float* colors, int colorSize,
									 float* UV, int UVSize,
									 float* normals, int normSize,
									 float* tangents, int tanSize,
									 int* indices, int indexSize,
									 float* matIndexs, int matIndexsSize, int matCount,
									 std::string Name)
{
	int vertexType = FE_POSITION | FE_INDEX;

	GLuint vaoID;
	FE_GL_ERROR(glGenVertexArrays(1, &vaoID));
	FE_GL_ERROR(glBindVertexArray(vaoID));

	GLuint indicesBufferID;
	// index
	FE_GL_ERROR(glGenBuffers(1, &indicesBufferID));
	FE_GL_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesBufferID));
	FE_GL_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * indexSize, indices, GL_STATIC_DRAW));

	GLuint positionsBufferID;
	// verCoords
	FE_GL_ERROR(glGenBuffers(1, &positionsBufferID));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, positionsBufferID));
	FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * posSize, positions, GL_STATIC_DRAW));
	FE_GL_ERROR(glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

	GLuint colorsBufferID = 0;
	if (colors != nullptr)
	{
		vertexType |= FE_COLOR;
		// colors
		FE_GL_ERROR(glGenBuffers(1, &colorsBufferID));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, colorsBufferID));
		FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * colorSize, colors, GL_STATIC_DRAW));
		FE_GL_ERROR(glVertexAttribPointer(1/*FE_COLOR*/, 3, GL_FLOAT, false, 0, 0));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}

	GLuint normalsBufferID = 0;
	if (normals != nullptr)
	{
		vertexType |= FE_NORMAL;
		// normals
		FE_GL_ERROR(glGenBuffers(1, &normalsBufferID));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, normalsBufferID));
		FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * normSize, normals, GL_STATIC_DRAW));
		FE_GL_ERROR(glVertexAttribPointer(2/*FE_NORMAL*/, 3, GL_FLOAT, false, 0, 0));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}

	GLuint tangentsBufferID = 0;
	if (tangents != nullptr)
	{
		vertexType |= FE_TANGENTS;
		// tangents
		FE_GL_ERROR(glGenBuffers(1, &tangentsBufferID));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, tangentsBufferID));
		FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * tanSize, tangents, GL_STATIC_DRAW));
		FE_GL_ERROR(glVertexAttribPointer(3/*FE_TANGENTS*/, 3, GL_FLOAT, false, 0, 0));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}

	GLuint UVBufferID = 0;
	if (UV != nullptr)
	{
		// UV
		FE_GL_ERROR(glGenBuffers(1, &UVBufferID));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, UVBufferID));
		FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * UVSize, UV, GL_STATIC_DRAW));
		FE_GL_ERROR(glVertexAttribPointer(4/*FE_UV*/, 2, GL_FLOAT, false, 0, 0));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}

	FEMesh* newMesh = new FEMesh(vaoID, indexSize, vertexType, Name);
	newMesh->indicesCount = indexSize;
	newMesh->indicesBufferID = indicesBufferID;

	newMesh->positionsCount = posSize;
	newMesh->positionsBufferID = positionsBufferID;

	newMesh->normalsCount = normSize;
	newMesh->normalsBufferID = normalsBufferID;

	newMesh->tangentsCount = tanSize;
	newMesh->tangentsBufferID = tangentsBufferID;

	newMesh->UVCount = UVSize;
	newMesh->UVBufferID = UVBufferID;

	newMesh->AABB = FEAABB(positions, posSize);

	return newMesh;
}

FEMesh* FECGALWrapper::importOBJ(const char* fileName, bool forceOneMesh)
{
	FEMesh* result = nullptr;
	FEObjLoader& objLoader = FEObjLoader::getInstance();
	objLoader.forceOneMesh = forceOneMesh;
	objLoader.readFile(fileName);

	if (objLoader.loadedObjects.size() > 0)
	{
		result = rawDataToMesh(objLoader.loadedObjects[0]->fVerC.data(), int(objLoader.loadedObjects[0]->fVerC.size()),
			nullptr, 0,
			objLoader.loadedObjects[0]->fTexC.data(), int(objLoader.loadedObjects[0]->fTexC.size()),
			objLoader.loadedObjects[0]->fNorC.data(), int(objLoader.loadedObjects[0]->fNorC.size()),
			objLoader.loadedObjects[0]->fTanC.data(), int(objLoader.loadedObjects[0]->fTanC.size()),
			objLoader.loadedObjects[0]->fInd.data(), int(objLoader.loadedObjects[0]->fInd.size()),
			objLoader.loadedObjects[0]->matIDs.data(), int(objLoader.loadedObjects[0]->matIDs.size()), int(objLoader.loadedObjects[0]->materialRecords.size()), "");
	}

	//createMaterialsFromOBJData(result);

	return result;
}

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

	result = rawDataToMesh(FEVertices.data(), int(FEVertices.size()),
		nullptr, 0, nullptr, 0, nullptr, 0, nullptr, 0,
		FEIndices.data(), int(FEIndices.size()),
		nullptr, 0, 0, "");

	return result;
}

Surface_mesh FECGALWrapper::FEMeshToSurfaceMesh(FEMesh* mesh)
{
	// Extracting data from FEMesh.
	std::vector<float> FEVertices;
	FEVertices.resize(mesh->getPositionsCount());
	FE_GL_ERROR(glGetNamedBufferSubData(mesh->getPositionsBufferID(), 0, sizeof(float) * FEVertices.size(), FEVertices.data()));

	std::vector<int> FEIndices;
	FEIndices.resize(mesh->getIndicesCount());
	FE_GL_ERROR(glGetNamedBufferSubData(mesh->getIndicesBufferID(), 0, sizeof(int) * FEIndices.size(), FEIndices.data()));

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

	// convert from soup to surface mesh
	PMP::orient_polygon_soup(anchors, triangles);

	std::vector<int> originalMeshToSegments;
	auto iterator = fpxmap.begin();
	while (iterator != fpxmap.end())
	{
		originalMeshToSegments.push_back(iterator._Ptr[0]);
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


	

	/*Surface_mesh tempMesh;
	PMP::polygon_soup_to_polygon_mesh(anchors, triangles, tempMesh);
	if (CGAL::is_closed(tempMesh) && (!PMP::is_outward_oriented(tempMesh)))
		PMP::reverse_face_orientations(tempMesh);

	
	std::vector<Point_3> extractedPoints;
	std::vector<Polygon_3> extractedFaces;
	PMP::polygon_mesh_to_polygon_soup(tempMesh, extractedPoints, extractedFaces);
	PMP::repair_polygon_soup(extractedPoints, extractedFaces);
	PMP::orient_polygon_soup(extractedPoints, extractedFaces);

	bool t = PMP::is_polygon_soup_a_polygon_mesh(extractedFaces);

	Surface_mesh output;
	PMP::polygon_soup_to_polygon_mesh(extractedPoints, extractedFaces, output);*/


	Surface_mesh::Property_map<face_descriptor, Kernel::Vector_3 > face_normals =
		output.add_property_map<face_descriptor, Kernel::Vector_3 >("f:normal").first;


	CGAL::Polygon_mesh_processing::compute_face_normals(output, face_normals);

	//std::vector<Point_3> extractedPoints;
	//std::vector<Polygon_3> extractedFaces;
	//PMP::polygon_mesh_to_polygon_soup(output, extractedPoints, extractedFaces);

	glm::vec3 point_3 = glm::vec3(anchors[3][0], anchors[3][1], anchors[3][2]);
	glm::vec3 point_2 = glm::vec3(anchors[2][0], anchors[2][1], anchors[2][2]);
	glm::vec3 point_1 = glm::vec3(anchors[1][0], anchors[1][1], anchors[1][2]);

	glm::vec3 edge_0 = point_3 - point_2;
	glm::vec3 edge_1 = point_3 - point_1;

	glm::vec3 normal = glm::normalize(glm::cross(edge_0, edge_1));



	std::string normalsList;

	for (size_t i = 0; i < polygons.size(); i++)
	{
		normalsList += "vn " + std::to_string(face_normals.data()[i].x());
		normalsList += " " + std::to_string(face_normals.data()[i].y());
		normalsList += " " + std::to_string(face_normals.data()[i].z());
		normalsList += "\n";
	}

	std::vector<float> calculatedNormals;
	calculatedNormals.resize(anchors.size() * 3);
	for (size_t i = 0; i < polygons.size(); i++)
	{
		int vertexIndex = i/*polygons[i][0]*/ * 3;
		calculatedNormals[vertexIndex + 0] = face_normals.data()[i].x();
		calculatedNormals[vertexIndex + 1] = face_normals.data()[i].y();
		calculatedNormals[vertexIndex + 2] = face_normals.data()[i].z();

		//vertexIndex = polygons[i][1] * 3;
		calculatedNormals[vertexIndex + 0] = face_normals.data()[i].x();
		calculatedNormals[vertexIndex + 1] = face_normals.data()[i].y();
		calculatedNormals[vertexIndex + 2] = face_normals.data()[i].z();

		//vertexIndex = polygons[i][2] * 3;
		calculatedNormals[vertexIndex + 0] = face_normals.data()[i].x();
		calculatedNormals[vertexIndex + 1] = face_normals.data()[i].y();
		calculatedNormals[vertexIndex + 2] = face_normals.data()[i].z();
	}



	saveSurfaceMeshToOBJFile("C:/Users/Kindr/Downloads/OBJ_Models/simplified.obj", output);

	//return surfaceMeshToFEMesh(output);


	int posSize = originalMesh->getPositionsCount();
	float* positions = new float[posSize];
	FE_GL_ERROR(glGetNamedBufferSubData(originalMesh->getPositionsBufferID(), 0, sizeof(float) * posSize, positions));

	int UVSize = originalMesh->getUVCount();
	float* UV = new float[UVSize];
	FE_GL_ERROR(glGetNamedBufferSubData(originalMesh->getUVBufferID(), 0, sizeof(float) * UVSize, UV));

	int normSize = originalMesh->getNormalsCount();
	float* normalsFloat = new float[normSize];
	FE_GL_ERROR(glGetNamedBufferSubData(originalMesh->getNormalsBufferID(), 0, sizeof(float) * normSize, normalsFloat));

	int tanSize = originalMesh->getTangentsCount();
	float* tangents = new float[tanSize];
	FE_GL_ERROR(glGetNamedBufferSubData(originalMesh->getTangentsBufferID(), 0, sizeof(float) * tanSize, tangents));

	int indexSize = originalMesh->getIndicesCount();
	int* indices = new int[indexSize];
	FE_GL_ERROR(glGetNamedBufferSubData(originalMesh->getIndicesBufferID(), 0, sizeof(int) * indexSize, indices));

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

	std::string outputString = "";
	for (size_t i = 0; i < originalMeshToSegments.size(); i++)
	{
		outputString += "facet " + std::to_string(i) + ": " + std::to_string(originalMeshToSegments[i]) + "\n";
	}

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
	for (size_t i = 0; i < originalMeshToSegments.size(); i++)
	{
		if (segIDColors.find(originalMeshToSegments[i]) == segIDColors.end())
		{
			glm::vec3 newColor = getRandColor(usedColors);
			segIDColors[originalMeshToSegments[i]] = newColor;
			usedColors.push_back(newColor);
		}

		setColorOfFace(i, segIDColors[originalMeshToSegments[i]]);
	}

	int segmentsColorsSize = posSize;
	float* segmentsColors = new float[posSize];

	for (size_t i = 0; i < posSize; i++)
	{
		segmentsColors[i] = colors[i];
	}

	// COLORS
	std::unordered_map<int, double> originalTrianglesRugosity;
	std::unordered_map<int, double> sectorRugositySum;
	std::unordered_map<int, int> sectorsCount;
	for (size_t i = 0; i < originalMeshToSegments.size(); i++)
	{
		glm::vec3 segmentNormal = proxiesVector[originalMeshToSegments[i]];
		std::vector<int> points = getVertexOfFace(i);

		Point_3 a = Point_3(positionsVector[points[0] * 3], positionsVector[points[0] * 3 + 1], positionsVector[points[0] * 3 + 2]);
		Point_3 b = Point_3(positionsVector[points[1] * 3], positionsVector[points[1] * 3 + 1], positionsVector[points[1] * 3 + 2]);
		Point_3 c = Point_3(positionsVector[points[2] * 3], positionsVector[points[2] * 3 + 1], positionsVector[points[2] * 3 + 2]);
		double originalArea = sqrt(CGAL::squared_area(a, b, c));

		Plane_3 segmentPlane = Plane_3(Point_3(0.0, 0.0, 0.0), Direction_3(segmentNormal.x, segmentNormal.y, segmentNormal.z));
		Point_3 aProjection = segmentPlane.projection(a);
		Point_3 bProjection = segmentPlane.projection(b);
		Point_3 cProjection = segmentPlane.projection(c);

		double projectionArea = sqrt(CGAL::squared_area(aProjection, bProjection, cProjection));
		double rugosity = originalArea / projectionArea;

		sectorRugositySum[originalMeshToSegments[i]] += rugosity;
		sectorsCount[originalMeshToSegments[i]]++;

		originalTrianglesRugosity[i] += rugosity;
	}

	double minRugorsity = DBL_MAX;
	double maxRugorsity = -DBL_MAX;
	std::vector<double> sectorsRugorsity;
	sectorsRugorsity.resize(proxiesVector.size());
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

	for (size_t i = 0; i < originalMeshToSegments.size(); i++)
	{
		//double normalizedRugorsity = (sectorsRugorsity[originalMeshToSegments[i]] - minRugorsity) / (maxRugorsity - minRugorsity);
		double normalizedRugorsity = sectorsRugorsity[originalMeshToSegments[i]];

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

	// COLORS END

	originalMesh->addColorToVertices(colors, colorSize);
	originalMesh->addSegmentsColorToVertices(segmentsColors, segmentsColorsSize);

	originalMesh->minRugorsity = minRugorsity;
	originalMesh->maxRugorsity = maxRugorsity;

	return surfaceMeshToFEMesh(output, calculatedNormals.data(), calculatedNormals.size());
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

	result = rawDataToMesh(FEVertices.data(), int(FEVertices.size()),
		nullptr, 0, nullptr, 0, normals, normSize, nullptr, 0,
		FEIndices.data(), int(FEIndices.size()),
		nullptr, 0, 0, "");

	return result;
}