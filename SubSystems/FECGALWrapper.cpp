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

	GLuint normalsBufferID = 0;
	if (normals != nullptr)
	{
		vertexType |= FE_UV;
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
		nullptr, 0, nullptr, 0, nullptr, 0,
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

	PMP::repair_polygon_soup(CGALPoints, CGALFaces);
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

	//saveSurfaceMeshToOBJFile("C:/Users/kandr/Downloads/simplified.obj", surface_mesh);

	return surfaceMeshToFEMesh(surface_mesh);
}

FEMesh* FECGALWrapper::SurfaceMeshApproximation(FEMesh* originalMesh, double verticesLeftInPersent)
{
	Surface_mesh mesh = FEMeshToSurfaceMesh(originalMesh);

	// The output will be an indexed triangle mesh
	std::vector<Kernel::Point_3> anchors;
	std::vector<std::array<std::size_t, 3> > triangles;
	// free function interface with named parameters
	VSA::approximate_triangle_mesh(mesh,
		CGAL::parameters::verbose_level(VSA::MAIN_STEPS).
		max_number_of_proxies(1000).
		anchors(std::back_inserter(anchors)). // anchor points
		triangles(std::back_inserter(triangles))); // indexed triangles

	// convert from soup to surface mesh
	PMP::orient_polygon_soup(anchors, triangles);
	Surface_mesh output;

	PMP::polygon_soup_to_polygon_mesh(anchors, triangles, output);
	if (CGAL::is_closed(output) && (!PMP::is_outward_oriented(output)))
		PMP::reverse_face_orientations(output);

	//saveSurfaceMeshToOBJFile("C:/Users/kandr/Downloads/simplified.obj", output);

	return surfaceMeshToFEMesh(output);
}