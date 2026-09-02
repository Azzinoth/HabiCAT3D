#include "AnalysisObjectManager.h"
using namespace FocalEngine;
#include "ComplexityCore/Layers/LayerManager.h"
#include "Shaders/CustomMeshShader/VS.glsl"
#include "Shaders/CustomMeshShader/FS.glsl"
#include "Shaders/PointCloudColorShader/CS.glsl"

#include "../VersionInfo/HabiCAT3D_Version.h"
#include "../VersionInfo/FEVersionInfo.h"
FE_DEFINE_VERSION_INFO(HabiCAT3D_)
#define APPLICATION_VERSION_FLOAT (GetHabiCAT3D_VersionInfo().Major + GetHabiCAT3D_VersionInfo().Minor / 10.0f + GetHabiCAT3D_VersionInfo().Patch / 100.0f)

AnalysisObjectManager::AnalysisObjectManager()
{
	if (!APPLICATION.HasConsoleWindow())
	{
		CustomMeshShader = RESOURCE_MANAGER.CreateShader("MainMeshShader", CustomMesh_VS, CustomMesh_FS);
		CustomMeshShader->UpdateUniformData("lightDirection", glm::vec3(0.0, 1.0, 0.2));

		CustomMaterial = RESOURCE_MANAGER.CreateMaterial("MainMeshMaterial");
		CustomMaterial->Shader = CustomMeshShader;

		PointCloudRecoloringShader = RESOURCE_MANAGER.CreateShader("PointCloudRecoloringShader", nullptr, nullptr, nullptr, nullptr, nullptr, PointCloudRecoloringShader_CS);

		static float turbo_srgb_floats[256][3] = { {0.18995f,0.07176f,0.23217f},{0.19483f,0.08339f,0.26149f},{0.19956f,0.09498f,0.29024f},{0.20415f,0.10652f,0.31844f},{0.20860f,0.11802f,0.34607f},{0.21291f,0.12947f,0.37314f},{0.21708f,0.14087f,0.39964f},{0.22111f,0.15223f,0.42558f},{0.22500f,0.16354f,0.45096f},{0.22875f,0.17481f,0.47578f},{0.23236f,0.18603f,0.50004f},{0.23582f,0.19720f,0.52373f},{0.23915f,0.20833f,0.54686f},{0.24234f,0.21941f,0.56942f},{0.24539f,0.23044f,0.59142f},{0.24830f,0.24143f,0.61286f},{0.25107f,0.25237f,0.63374f},{0.25369f,0.26327f,0.65406f},{0.25618f,0.27412f,0.67381f},{0.25853f,0.28492f,0.69300f},{0.26074f,0.29568f,0.71162f},{0.26280f,0.30639f,0.72968f},{0.26473f,0.31706f,0.74718f},{0.26652f,0.32768f,0.76412f},{0.26816f,0.33825f,0.78050f},{0.26967f,0.34878f,0.79631f},{0.27103f,0.35926f,0.81156f},{0.27226f,0.36970f,0.82624f},{0.27334f,0.38008f,0.84037f},{0.27429f,0.39043f,0.85393f},{0.27509f,0.40072f,0.86692f},{0.27576f,0.41097f,0.87936f},{0.27628f,0.42118f,0.89123f},{0.27667f,0.43134f,0.90254f},{0.27691f,0.44145f,0.91328f},{0.27701f,0.45152f,0.92347f},{0.27698f,0.46153f,0.93309f},{0.27680f,0.47151f,0.94214f},{0.27648f,0.48144f,0.95064f},{0.27603f,0.49132f,0.95857f},{0.27543f,0.50115f,0.96594f},{0.27469f,0.51094f,0.97275f},{0.27381f,0.52069f,0.97899f},{0.27273f,0.53040f,0.98461f},{0.27106f,0.54015f,0.98930f},{0.26878f,0.54995f,0.99303f},{0.26592f,0.55979f,0.99583f},{0.26252f,0.56967f,0.99773f},{0.25862f,0.57958f,0.99876f},{0.25425f,0.58950f,0.99896f},{0.24946f,0.59943f,0.99835f},{0.24427f,0.60937f,0.99697f},{0.23874f,0.61931f,0.99485f},{0.23288f,0.62923f,0.99202f},{0.22676f,0.63913f,0.98851f},{0.22039f,0.64901f,0.98436f},{0.21382f,0.65886f,0.97959f},{0.20708f,0.66866f,0.97423f},{0.20021f,0.67842f,0.96833f},{0.19326f,0.68812f,0.96190f},{0.18625f,0.69775f,0.95498f},{0.17923f,0.70732f,0.94761f},{0.17223f,0.71680f,0.93981f},{0.16529f,0.72620f,0.93161f},{0.15844f,0.73551f,0.92305f},{0.15173f,0.74472f,0.91416f},{0.14519f,0.75381f,0.90496f},{0.13886f,0.76279f,0.89550f},{0.13278f,0.77165f,0.88580f},{0.12698f,0.78037f,0.87590f},{0.12151f,0.78896f,0.86581f},{0.11639f,0.79740f,0.85559f},{0.11167f,0.80569f,0.84525f},{0.10738f,0.81381f,0.83484f},{0.10357f,0.82177f,0.82437f},{0.10026f,0.82955f,0.81389f},{0.09750f,0.83714f,0.80342f},{0.09532f,0.84455f,0.79299f},{0.09377f,0.85175f,0.78264f},{0.09287f,0.85875f,0.77240f},{0.09267f,0.86554f,0.76230f},{0.09320f,0.87211f,0.75237f},{0.09451f,0.87844f,0.74265f},{0.09662f,0.88454f,0.73316f},{0.09958f,0.89040f,0.72393f},{0.10342f,0.89600f,0.71500f},{0.10815f,0.90142f,0.70599f},{0.11374f,0.90673f,0.69651f},{0.12014f,0.91193f,0.68660f},{0.12733f,0.91701f,0.67627f},{0.13526f,0.92197f,0.66556f},{0.14391f,0.92680f,0.65448f},{0.15323f,0.93151f,0.64308f},{0.16319f,0.93609f,0.63137f},{0.17377f,0.94053f,0.61938f},{0.18491f,0.94484f,0.60713f},{0.19659f,0.94901f,0.59466f},{0.20877f,0.95304f,0.58199f},{0.22142f,0.95692f,0.56914f},{0.23449f,0.96065f,0.55614f},{0.24797f,0.96423f,0.54303f},{0.26180f,0.96765f,0.52981f},{0.27597f,0.97092f,0.51653f},{0.29042f,0.97403f,0.50321f},{0.30513f,0.97697f,0.48987f},{0.32006f,0.97974f,0.47654f},{0.33517f,0.98234f,0.46325f},{0.35043f,0.98477f,0.45002f},{0.36581f,0.98702f,0.43688f},{0.38127f,0.98909f,0.42386f},{0.39678f,0.99098f,0.41098f},{0.41229f,0.99268f,0.39826f},{0.42778f,0.99419f,0.38575f},{0.44321f,0.99551f,0.37345f},{0.45854f,0.99663f,0.36140f},{0.47375f,0.99755f,0.34963f},{0.48879f,0.99828f,0.33816f},{0.50362f,0.99879f,0.32701f},{0.51822f,0.99910f,0.31622f},{0.53255f,0.99919f,0.30581f},{0.54658f,0.99907f,0.29581f},{0.56026f,0.99873f,0.28623f},{0.57357f,0.99817f,0.27712f},{0.58646f,0.99739f,0.26849f},{0.59891f,0.99638f,0.26038f},{0.61088f,0.99514f,0.25280f},{0.62233f,0.99366f,0.24579f},{0.63323f,0.99195f,0.23937f},{0.64362f,0.98999f,0.23356f},{0.65394f,0.98775f,0.22835f},{0.66428f,0.98524f,0.22370f},{0.67462f,0.98246f,0.21960f},{0.68494f,0.97941f,0.21602f},{0.69525f,0.97610f,0.21294f},{0.70553f,0.97255f,0.21032f},{0.71577f,0.96875f,0.20815f},{0.72596f,0.96470f,0.20640f},{0.73610f,0.96043f,0.20504f},{0.74617f,0.95593f,0.20406f},{0.75617f,0.95121f,0.20343f},{0.76608f,0.94627f,0.20311f},{0.77591f,0.94113f,0.20310f},{0.78563f,0.93579f,0.20336f},{0.79524f,0.93025f,0.20386f},{0.80473f,0.92452f,0.20459f},{0.81410f,0.91861f,0.20552f},{0.82333f,0.91253f,0.20663f},{0.83241f,0.90627f,0.20788f},{0.84133f,0.89986f,0.20926f},{0.85010f,0.89328f,0.21074f},{0.85868f,0.88655f,0.21230f},{0.86709f,0.87968f,0.21391f},{0.87530f,0.87267f,0.21555f},{0.88331f,0.86553f,0.21719f},{0.89112f,0.85826f,0.21880f},{0.89870f,0.85087f,0.22038f},{0.90605f,0.84337f,0.22188f},{0.91317f,0.83576f,0.22328f},{0.92004f,0.82806f,0.22456f},{0.92666f,0.82025f,0.22570f},{0.93301f,0.81236f,0.22667f},{0.93909f,0.80439f,0.22744f},{0.94489f,0.79634f,0.22800f},{0.95039f,0.78823f,0.22831f},{0.95560f,0.78005f,0.22836f},{0.96049f,0.77181f,0.22811f},{0.96507f,0.76352f,0.22754f},{0.96931f,0.75519f,0.22663f},{0.97323f,0.74682f,0.22536f},{0.97679f,0.73842f,0.22369f},{0.98000f,0.73000f,0.22161f},{0.98289f,0.72140f,0.21918f},{0.98549f,0.71250f,0.21650f},{0.98781f,0.70330f,0.21358f},{0.98986f,0.69382f,0.21043f},{0.99163f,0.68408f,0.20706f},{0.99314f,0.67408f,0.20348f},{0.99438f,0.66386f,0.19971f},{0.99535f,0.65341f,0.19577f},{0.99607f,0.64277f,0.19165f},{0.99654f,0.63193f,0.18738f},{0.99675f,0.62093f,0.18297f},{0.99672f,0.60977f,0.17842f},{0.99644f,0.59846f,0.17376f},{0.99593f,0.58703f,0.16899f},{0.99517f,0.57549f,0.16412f},{0.99419f,0.56386f,0.15918f},{0.99297f,0.55214f,0.15417f},{0.99153f,0.54036f,0.14910f},{0.98987f,0.52854f,0.14398f},{0.98799f,0.51667f,0.13883f},{0.98590f,0.50479f,0.13367f},{0.98360f,0.49291f,0.12849f},{0.98108f,0.48104f,0.12332f},{0.97837f,0.46920f,0.11817f},{0.97545f,0.45740f,0.11305f},{0.97234f,0.44565f,0.10797f},{0.96904f,0.43399f,0.10294f},{0.96555f,0.42241f,0.09798f},{0.96187f,0.41093f,0.09310f},{0.95801f,0.39958f,0.08831f},{0.95398f,0.38836f,0.08362f},{0.94977f,0.37729f,0.07905f},{0.94538f,0.36638f,0.07461f},{0.94084f,0.35566f,0.07031f},{0.93612f,0.34513f,0.06616f},{0.93125f,0.33482f,0.06218f},{0.92623f,0.32473f,0.05837f},{0.92105f,0.31489f,0.05475f},{0.91572f,0.30530f,0.05134f},{0.91024f,0.29599f,0.04814f},{0.90463f,0.28696f,0.04516f},{0.89888f,0.27824f,0.04243f},{0.89298f,0.26981f,0.03993f},{0.88691f,0.26152f,0.03753f},{0.88066f,0.25334f,0.03521f},{0.87422f,0.24526f,0.03297f},{0.86760f,0.23730f,0.03082f},{0.86079f,0.22945f,0.02875f},{0.85380f,0.22170f,0.02677f},{0.84662f,0.21407f,0.02487f},{0.83926f,0.20654f,0.02305f},{0.83172f,0.19912f,0.02131f},{0.82399f,0.19182f,0.01966f},{0.81608f,0.18462f,0.01809f},{0.80799f,0.17753f,0.01660f},{0.79971f,0.17055f,0.01520f},{0.79125f,0.16368f,0.01387f},{0.78260f,0.15693f,0.01264f},{0.77377f,0.15028f,0.01148f},{0.76476f,0.14374f,0.01041f},{0.75556f,0.13731f,0.00942f},{0.74617f,0.13098f,0.00851f},{0.73661f,0.12477f,0.00769f},{0.72686f,0.11867f,0.00695f},{0.71692f,0.11268f,0.00629f},{0.70680f,0.10680f,0.00571f},{0.69650f,0.10102f,0.00522f},{0.68602f,0.09536f,0.00481f},{0.67535f,0.08980f,0.00449f},{0.66449f,0.08436f,0.00424f},{0.65345f,0.07902f,0.00408f},{0.64223f,0.07380f,0.00401f},{0.63082f,0.06868f,0.00401f},{0.61923f,0.06367f,0.00410f},{0.60746f,0.05878f,0.00427f},{0.59550f,0.05399f,0.00453f},{0.58336f,0.04931f,0.00486f},{0.57103f,0.04474f,0.00529f},{0.55852f,0.04028f,0.00579f},{0.54583f,0.03593f,0.00638f},{0.53295f,0.03169f,0.00705f},{0.51989f,0.02756f,0.00780f},{0.50664f,0.02354f,0.00863f},{0.49321f,0.01963f,0.00955f},{0.47960f,0.01583f,0.01055f} };
		FE_GL_ERROR(glGenBuffers(1, &TurboColorBuffer));
		FE_GL_ERROR(glBindBuffer(GL_SHADER_STORAGE_BUFFER, TurboColorBuffer));
		FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, TurboColorBuffer));
		FE_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * 256 * 3, turbo_srgb_floats, GL_STATIC_READ));
	}
}

AnalysisObjectManager::~AnalysisObjectManager() {}

MeshAnalysisData* AnalysisObjectManager::ExtractAdditionalGeometryData(std::vector<double>& Vertices, std::vector<float>& Colors, std::vector<float>& UVs, std::vector<float>& Tangents, std::vector<int>& Indices, std::vector<float>& Normals)
{
	MeshAnalysisData* Result = new MeshAnalysisData();
	Result->Vertices = Vertices;
	Result->Colors = Colors;
	Result->UVs = UVs;
	Result->Tangents = Tangents;
	Result->Indices = Indices;
	Result->Normals = Normals;

	std::vector<glm::dvec3> Triangle;
	Triangle.resize(3);
	std::vector<glm::vec3> TriangleNormal;
	TriangleNormal.resize(3);

	for (size_t i = 0; i < Indices.size(); i += 3)
	{
		int VertexPosition = Indices[i] * 3;
		Triangle[0] = glm::dvec3(Vertices[VertexPosition], Vertices[VertexPosition + 1], Vertices[VertexPosition + 2]);

		VertexPosition = Indices[i + 1] * 3;
		Triangle[1] = glm::dvec3(Vertices[VertexPosition], Vertices[VertexPosition + 1], Vertices[VertexPosition + 2]);

		VertexPosition = Indices[i + 2] * 3;
		Triangle[2] = glm::dvec3(Vertices[VertexPosition], Vertices[VertexPosition + 1], Vertices[VertexPosition + 2]);

		Result->Triangles.push_back(Triangle);
		Result->TrianglesArea.push_back(GEOMETRY.CalculateTriangleArea(Triangle[0], Triangle[1], Triangle[2]));
		Result->TotalArea += Result->TrianglesArea.back();

		Result->TrianglesCentroids.push_back((Triangle[0] + Triangle[1] + Triangle[2]) / 3.0);

		if (!Normals.empty())
		{
			VertexPosition = Indices[i] * 3;
			TriangleNormal[0] = glm::vec3(Normals[VertexPosition], Normals[VertexPosition + 1], Normals[VertexPosition + 2]);

			VertexPosition = Indices[i + 1] * 3;
			TriangleNormal[1] = glm::vec3(Normals[VertexPosition], Normals[VertexPosition + 1], Normals[VertexPosition + 2]);

			VertexPosition = Indices[i + 2] * 3;
			TriangleNormal[2] = glm::vec3(Normals[VertexPosition], Normals[VertexPosition + 1], Normals[VertexPosition + 2]);

			Result->TrianglesNormals.push_back(TriangleNormal);
		}
	}
	Result->UpdateAverageNormal();

	Result->AABB = FEAABB(Vertices.data(), static_cast<int>(Vertices.size()));
	return Result;
}

float AnalysisObjectManager::CheckRUGFileVersion(std::string FilePath)
{
	float Result = -1.0f;
	if (!FILE_SYSTEM.DoesFileExist(FilePath))
	{
		LOG.Add(std::string("Can't find file: ") + FilePath + " in function CheckRUGFileVersion.");
		return Result;
	}

	std::fstream File;
	File.open(FilePath, std::ios::in | std::ios::binary);
	if (!File.is_open())
	{
		LOG.Add(std::string("Can't open file: ") + FilePath + " in function CheckRUGFileVersion.");
		return Result;
	}

	File.seekg(0, std::ios::end);
	const std::streamsize FileSize = File.tellg();
	File.seekg(0, std::ios::beg);
	if (FileSize <= 0)
	{
		LOG.Add(std::string("Can't get file size: ") + FilePath + " in function CheckRUGFileVersion.");
		return Result;
	}

	char* Buffer = new char[4];
	File.read(Buffer, 4);
	Result = *(float*)Buffer;
	delete[] Buffer;

	return Result;
}

AnalysisObject* AnalysisObjectManager::ImportOBJ(const char* FilePath, bool bForceOneMesh)
{
	AnalysisObject* Result = new AnalysisObject();
	Result->Type = DATA_SOURCE_TYPE::MESH;
	Result->FilePath = FilePath;

	FEMesh* LoadedMesh = nullptr;
	FEObjLoader& OBJLoader = FEObjLoader::GetInstance();
	OBJLoader.ForceOneMesh(bForceOneMesh);
	OBJLoader.ForcePositionNormalization(true);
	OBJLoader.UseDoublePrecisionForReadingCoordinates(true);
	OBJLoader.DoubleVertexOnSeams(false);
	OBJLoader.ReadFile(FilePath);

	std::vector<FERawOBJData*>* LoadedObjects = OBJLoader.GetLoadedObjects();
	FERawOBJData* FirstObject = LoadedObjects->empty() ? nullptr : (*LoadedObjects)[0];

	if (FirstObject != nullptr)
	{
		if (!APPLICATION.HasConsoleWindow())
		{
			LoadedMesh = RESOURCE_MANAGER.RawDataToMesh(FirstObject->FVerC.data(), int(FirstObject->FVerC.size()),
														FirstObject->FTexC.data(), int(FirstObject->FTexC.size()),
														FirstObject->FNorC.data(), int(FirstObject->FNorC.size()),
														FirstObject->FTanC.data(), int(FirstObject->FTanC.size()),
														FirstObject->FInd.data(), int(FirstObject->FInd.size()),
														FirstObject->FColorsC.data(), int(FirstObject->FColorsC.size()),
														FirstObject->MaterialIDs.data(), int(FirstObject->MaterialIDs.size()), int(FirstObject->MaterialRecords.size()), "");
		}
		
		Result->EngineResource = LoadedMesh;
		Result->Type = DATA_SOURCE_TYPE::MESH;
		Result->FilePath = FilePath;
		Result->Name = FILE_SYSTEM.GetFileName(FilePath, false);
		Result->AnalysisData = ExtractAdditionalGeometryData(FirstObject->DVerC, FirstObject->FColorsC, FirstObject->FTexC, FirstObject->FTanC, FirstObject->FInd, FirstObject->FNorC);
		
		Result->AppliedShift = OBJLoader.GetLastAppliedShift();
	}
	
	return Result;
}

AnalysisObject* AnalysisObjectManager::LoadRUGFile(std::string FilePath)
{
	std::fstream File;
	File.open(FilePath, std::ios::in | std::ios::binary);
	if (!File.is_open())
	{
		LOG.Add(std::string("Can't open file: ") + FilePath + " in function LoadRUGFile.");
		return false;
	}

	File.seekg(0, std::ios::end);
	const std::streamsize FileSize = File.tellg();
	File.seekg(0, std::ios::beg);
	if (FileSize <= 0)
	{
		LOG.Add(std::string("Can't get file size: ") + FilePath + " in function LoadRUGFile.");
		return false;
	}

	char* Buffer = new char[4];
	long long ArraySize = 0;

	// Version of FEMesh file type
	File.read(Buffer, 4);
	const float Version = *(float*)Buffer;
	if (Version > APPLICATION_VERSION_FLOAT && abs(Version - APPLICATION_VERSION_FLOAT) > 0.0001)
	{
		LOG.Add(std::string("Can't load file: ") + FilePath + " in function LoadRUGFile. File was created in different Version of application!");
		return nullptr;
	}

	File.read(Buffer, 4);
	const int VertexCount = *(int*)Buffer;

	int BytesPerVertex = 8;
	if (Version < 0.87)
		BytesPerVertex = 4;
	ArraySize = long long(VertexCount) * long long(BytesPerVertex);
	char* VertexBuffer = new char[ArraySize];
	File.read(VertexBuffer, ArraySize);

	File.read(Buffer, 4);
	const int ColorCount = *(int*)Buffer;
	char* ColorBuffer = nullptr;
	if (ColorCount != 0)
	{
		ArraySize = long long(ColorCount) * long long(4);
		ColorBuffer = new char[ArraySize];
		File.read(ColorBuffer, ArraySize);
	}

	File.read(Buffer, 4);
	const int TexCout = *(int*)Buffer;
	ArraySize = long long(TexCout) * long long(4);
	char* TexBuffer = new char[ArraySize];
	File.read(TexBuffer, ArraySize);

	File.read(Buffer, 4);
	const int NormCout = *(int*)Buffer;
	ArraySize = long long(NormCout) * long long(4);
	char* NormBuffer = new char[ArraySize];
	File.read(NormBuffer, ArraySize);

	File.read(Buffer, 4);
	const int TangCout = *(int*)Buffer;
	ArraySize = long long(TangCout) * long long(4);
	char* TangBuffer = new char[ArraySize];
	File.read(TangBuffer, ArraySize);

	File.read(Buffer, 4);
	const int IndexCout = *(int*)Buffer;
	ArraySize = long long(IndexCout) * long long(4);
	char* IndexBuffer = new char[ArraySize];
	File.read(IndexBuffer, ArraySize);

	File.read(Buffer, 4);
	const int LayerCount = *(int*)Buffer;
	std::vector<DataLayer*> Layers;
	Layers.resize(LayerCount);

	for (size_t i = 0; i < Layers.size(); i++)
	{
		Layers[i] = new DataLayer();

		if (Version >= 0.55)
		{
			File.read(Buffer, 4);
			const int LayerType = *(int*)Buffer;
			Layers[i]->SetType(LAYER_TYPE(LayerType));
		}

		if (Version >= 0.62)
		{
			Layers[i]->ForceID(FILE_SYSTEM.ReadFEString(File));
		}

		Layers[i]->SetCaption(FILE_SYSTEM.ReadFEString(File));
		Layers[i]->SetNote(FILE_SYSTEM.ReadFEString(File));

		// ElementsToData
		File.read(Buffer, 4);
		const int ElementsToDataCout = *(int*)Buffer;
		std::vector<float> TrianglesData;
		Layers[i]->ElementsToData.resize(ElementsToDataCout);
		File.read((char*)Layers[i]->ElementsToData.data(), ElementsToDataCout * 4);

		// Debug info.
		File.read(Buffer, 4);
		const int DebugInfoPresent = *(int*)Buffer;
		if (DebugInfoPresent)
		{
			Layers[i]->DebugInfo = new DataLayerDebugInfo();
			Layers[i]->DebugInfo->FromFile(File);
		}
	}

	FEAABB MeshAABB;

	glm::vec3 Min;
	File.read(Buffer, 4);
	Min.x = *(float*)Buffer;
	File.read(Buffer, 4);
	Min.y = *(float*)Buffer;
	File.read(Buffer, 4);
	Min.z = *(float*)Buffer;

	glm::vec3 Max;
	File.read(Buffer, 4);
	Max.x = *(float*)Buffer;
	File.read(Buffer, 4);
	Max.y = *(float*)Buffer;
	File.read(Buffer, 4);
	Max.z = *(float*)Buffer;

	MeshAABB = FEAABB(Min, Max);

	File.close();

	std::vector<double> FEVertices;
	FEMesh* NewMesh = nullptr;
	if (Version < 0.87)
	{
		NewMesh = RESOURCE_MANAGER.RawDataToMesh((float*)VertexBuffer, VertexCount,
												 (float*)TexBuffer, TexCout,
												 (float*)NormBuffer, NormCout,
												 (float*)TangBuffer, TangCout,
												 (int*)IndexBuffer, IndexCout,
												 (float*)ColorBuffer, ColorCount,
												 nullptr, 0, 0, "");

		FEVertices.resize(VertexCount);
		for (size_t i = 0; i < VertexCount; i++)
		{
			FEVertices[i] = ((float*)VertexBuffer)[i];
		}
	}
	else
	{
		std::vector<float> FEFloatVertices;
		FEFloatVertices.resize(VertexCount);
		for (size_t i = 0; i < VertexCount; i++)
		{
			FEFloatVertices[i] = static_cast<float>(((double*)VertexBuffer)[i]);
		}


		NewMesh = RESOURCE_MANAGER.RawDataToMesh((float*)FEFloatVertices.data(), VertexCount,
												 (float*)TexBuffer, TexCout,
												 (float*)NormBuffer, NormCout,
												 (float*)TangBuffer, TangCout,
												 (int*)IndexBuffer, IndexCout,
												 (float*)ColorBuffer, ColorCount,
												 nullptr, 0, 0, "");

		FEVertices.resize(VertexCount);
		for (size_t i = 0; i < VertexCount; i++)
		{
			FEVertices[i] = ((double*)VertexBuffer)[i];
		}
	}

	std::vector<float> FEColors;
	FEColors.resize(ColorCount);
	for (size_t i = 0; i < ColorCount; i++)
	{
		FEColors[i] = ((float*)ColorBuffer)[i];
	}

	std::vector<float> FEUVs;
	FEUVs.resize(TexCout);
	for (size_t i = 0; i < TexCout; i++)
	{
		FEUVs[i] = ((float*)TexBuffer)[i];
	}

	std::vector<float> FETangents;
	FETangents.resize(TangCout);
	for (size_t i = 0; i < TangCout; i++)
	{
		FETangents[i] = ((float*)TangBuffer)[i];
	}

	std::vector<int> FEIndices;
	FEIndices.resize(IndexCout);
	for (size_t i = 0; i < IndexCout; i++)
	{
		FEIndices[i] = ((int*)IndexBuffer)[i];
	}

	std::vector<float> FENormals;
	FENormals.resize(NormCout);
	for (size_t i = 0; i < NormCout; i++)
	{
		FENormals[i] = ((float*)NormBuffer)[i];
	}
	
	AnalysisObject* Result = new AnalysisObject();
	Result->Type = DATA_SOURCE_TYPE::MESH;
	Result->FilePath = FilePath;
	Result->EngineResource = NewMesh;
	Result->AnalysisData = ExtractAdditionalGeometryData(FEVertices, FEColors, FEUVs, FETangents, FEIndices, FENormals);

	delete[] Buffer;
	delete[] VertexBuffer;
	delete[] TexBuffer;
	delete[] NormBuffer;
	delete[] TangBuffer;
	delete[] IndexBuffer;

	for (size_t i = 0; i < Layers.size(); i++)
		Result->AddLayer(Layers[i]);

	return Result;
}

void AnalysisObjectManager::OnAnalysisObjectLoad(AnalysisObject* NewObject)
{
	if (NewObject == nullptr)
		return;
	
	InitializeSceneObjects(NewObject);
	AnalysisObjects[NewObject->ID] = NewObject;
	SetActiveAnalysisObject(NewObject->ID);

	for (size_t i = 0; i < NewObject->Layers.size(); i++)
		NewObject->Layers[i]->ComputeStatistics();

	for (size_t i = 0; i < ClientOnLoadCallbacks.size(); i++)
	{
		if (ClientOnLoadCallbacks[i] == nullptr)
			continue;

		ClientOnLoadCallbacks[i](NewObject);
	}
}

AnalysisObject* AnalysisObjectManager::CreateAnalysisObject(std::vector<FEPointCloudVertex>& RawPointCloudData, std::string ObjectName)
{
	AnalysisObject* Result = nullptr;
	if (RawPointCloudData.empty())
		return Result;

	FEPointCloud* NewPointCloud = RESOURCE_MANAGER.RawDataToFEPointCloud(RawPointCloudData, "", "", false);
	NewPointCloud->SetAdvancedRenderingEnabled(true);

	Result = new AnalysisObject();
	Result->Type = DATA_SOURCE_TYPE::POINT_CLOUD;
	Result->EngineResource = NewPointCloud;
	Result->Name = ObjectName;
	Result->AnalysisData = ExtractAdditionalGeometryData(NewPointCloud);
	Result->AppliedShift = RESOURCE_MANAGER.GetLastLoadedPointCloudAppliedShift();

	OnAnalysisObjectLoad(Result);

	return Result;
}

void AnalysisObjectManager::LoadResource(std::string FilePath)
{
	AnalysisObject* LoadedResource = nullptr;
	if (!FILE_SYSTEM.DoesFileExist(FilePath.c_str()))
		return;

	std::string FileExtension = FILE_SYSTEM.GetFileExtension(FilePath.c_str());
	// Convert to lower case.
	std::transform(FileExtension.begin(), FileExtension.end(), FileExtension.begin(), [](const unsigned char Character) {
		return std::tolower(Character);
	});

	if (FileExtension == ".obj")
	{
		LoadedResource = ImportOBJ(FilePath.c_str(), true);
		LoadedResource->AppliedShift = FEObjLoader::GetInstance().GetLastAppliedShift();
	}
	else if (FileExtension == ".rug")
	{
		float Version = CheckRUGFileVersion(FilePath);
		if (abs(Version - 0.91f) < 0.0001f)
		{
			LoadRUGFile_V0_9_1(FilePath);
			return;
		}
		else
		{
			LoadedResource = LoadRUGFile(FilePath);
			if (LoadedResource == nullptr)
				return;

			LoadedResource->Name = FILE_SYSTEM.GetFileName(FilePath, false);
		}
	}
	else if (FileExtension == ".ply")
	{
		FEObject* LoadedObject = RESOURCE_MANAGER.ImportPLYFile(FilePath);
		if (LoadedObject == nullptr)
			return;

		if (LoadedObject->GetType() == FE_POINT_CLOUD)
		{
			LoadedResource = new AnalysisObject();
			LoadedResource->Type = DATA_SOURCE_TYPE::POINT_CLOUD;
			LoadedResource->FilePath = FilePath;
			LoadedResource->Name = FILE_SYSTEM.GetFileName(FilePath, false);
			LoadedResource->EngineResource = LoadedObject;
			LoadedResource->AnalysisData = ExtractAdditionalGeometryData(static_cast<FEPointCloud*>(LoadedObject));
			LoadedResource->AppliedShift = RESOURCE_MANAGER.GetLastLoadedPointCloudAppliedShift();
		}
		else if (LoadedObject->GetType() == FE_MESH)
		{
			FEMesh* LoadedMesh = static_cast<FEMesh*>(LoadedObject);

			std::vector<float> FEFloatVertices;
			FEFloatVertices.resize(LoadedMesh->GetPositionsCount());
			FE_GL_ERROR(glGetNamedBufferSubData(LoadedMesh->GetPositionsBufferID(), 0, sizeof(float) * LoadedMesh->GetPositionsCount(), FEFloatVertices.data()));
			std::vector<double> FEVertices(FEFloatVertices.begin(), FEFloatVertices.end());

			std::vector<int> FEIndices;
			FEIndices.resize(LoadedMesh->GetIndicesCount());
			FE_GL_ERROR(glGetNamedBufferSubData(LoadedMesh->GetIndicesBufferID(), 0, sizeof(int) * LoadedMesh->GetIndicesCount(), FEIndices.data()));

			std::vector<float> FEColors;
			if (LoadedMesh->GetColorCount() > 0)
			{
				FEColors.resize(LoadedMesh->GetColorCount());
				FE_GL_ERROR(glGetNamedBufferSubData(LoadedMesh->GetColorBufferID(), 0, sizeof(float) * LoadedMesh->GetColorCount(), FEColors.data()));
			}

			std::vector<float> FEUVs;
			if (LoadedMesh->GetUVCount() > 0)
			{
				FEUVs.resize(LoadedMesh->GetUVCount());
				FE_GL_ERROR(glGetNamedBufferSubData(LoadedMesh->GetUVBufferID(), 0, sizeof(float) * LoadedMesh->GetUVCount(), FEUVs.data()));
			}

			std::vector<float> FETangents;
			if (LoadedMesh->GetTangentsCount() > 0)
			{
				FETangents.resize(LoadedMesh->GetTangentsCount());
				FE_GL_ERROR(glGetNamedBufferSubData(LoadedMesh->GetTangentsBufferID(), 0, sizeof(float) * LoadedMesh->GetTangentsCount(), FETangents.data()));
			}

			std::vector<float> FENormals;
			if (LoadedMesh->GetNormalsCount() > 0)
			{
				FENormals.resize(LoadedMesh->GetNormalsCount());
				FE_GL_ERROR(glGetNamedBufferSubData(LoadedMesh->GetNormalsBufferID(), 0, sizeof(float) * LoadedMesh->GetNormalsCount(), FENormals.data()));
			}

			LoadedResource = new AnalysisObject();
			LoadedResource->Type = DATA_SOURCE_TYPE::MESH;
			LoadedResource->FilePath = FilePath;
			LoadedResource->Name = FILE_SYSTEM.GetFileName(FilePath, false);
			LoadedResource->EngineResource = LoadedMesh;
			LoadedResource->AnalysisData = ExtractAdditionalGeometryData(FEVertices, FEColors, FEUVs, FETangents, FEIndices, FENormals);
			// PLY mesh import does not shift positions, unlike OBJ and point cloud imports.
			LoadedResource->AppliedShift = glm::dvec3(0.0);
		}
	}
	else if (FileExtension == ".las" || FileExtension == ".laz")
	{
		if (APPLICATION.HasConsoleWindow())
		{
			if (!LAS_LOADER.ReadFile(FilePath))
				return;

			FELASData* LoadedData = nullptr;
			LAS_LOADER.TakeOwnershipOfLastLoadedData(LoadedData);
			if (LoadedData == nullptr)
				return;

			std::vector<FEPointCloudVertexDouble>& Vertices = LoadedData->PointCloudVertices;

			glm::dvec3 Min = glm::dvec3(std::numeric_limits<double>::max());
			glm::dvec3 Max = glm::dvec3(-std::numeric_limits<double>::max());

			for (size_t i = 0; i < Vertices.size(); i++)
			{
				if (Vertices[i].X < Min.x)
					Min.x = Vertices[i].X;

				if (Vertices[i].X > Max.x)
					Max.x = Vertices[i].X;

				if (Vertices[i].Y < Min.y)
					Min.y = Vertices[i].Y;

				if (Vertices[i].Y > Max.y)
					Max.y = Vertices[i].Y;

				if (Vertices[i].Z < Min.z)
					Min.z = Vertices[i].Z;

				if (Vertices[i].Z > Max.z)
					Max.z = Vertices[i].Z;
			}

			glm::dvec3 Extent = Max - Min;
			glm::dvec3 Center = Min + Extent / 2.0;

			// The centered coordinates are truncated to float because GUI path uses floats also.
			for (size_t i = 0; i < Vertices.size(); i++)
			{
				Vertices[i].X = static_cast<float>(Vertices[i].X - Center.x);
				Vertices[i].Y = static_cast<float>(Vertices[i].Y - Center.y);
				Vertices[i].Z = static_cast<float>(Vertices[i].Z - Center.z);
			}

			FEAABB PointCloudAABB = FEAABB(Min - Center, Max - Center);

			LoadedResource = new AnalysisObject();
			LoadedResource->Type = DATA_SOURCE_TYPE::POINT_CLOUD;
			LoadedResource->FilePath = FilePath;
			LoadedResource->Name = FILE_SYSTEM.GetFileName(FilePath, false);
			LoadedResource->EngineResource = nullptr;
			LoadedResource->AnalysisData = ExtractAdditionalGeometryData(LoadedData->PointCloudVertices, PointCloudAABB);
			LoadedResource->AppliedShift = Center;

			delete LoadedData;
		}
		else
		{
			FEPointCloud* PointCloud = RESOURCE_MANAGER.ImportPointCloud(FilePath);
			if (PointCloud == nullptr)
				return;

			LoadedResource = new AnalysisObject();
			LoadedResource->Type = DATA_SOURCE_TYPE::POINT_CLOUD;
			LoadedResource->FilePath = FilePath;
			LoadedResource->Name = FILE_SYSTEM.GetFileName(FilePath, false);
			LoadedResource->EngineResource = PointCloud;
			LoadedResource->AnalysisData = ExtractAdditionalGeometryData(PointCloud);
			LoadedResource->AppliedShift = RESOURCE_MANAGER.GetLastLoadedPointCloudAppliedShift();
		}
	}

	OnAnalysisObjectLoad(LoadedResource);
}

void AnalysisObjectManager::AddOnObjectLoadCallback(std::function<void(AnalysisObject*)> Callback)
{
	ClientOnLoadCallbacks.push_back(Callback);
}

std::vector<int> AnalysisObjectManager::GetVertexAttributeIndexes(int InterpolationLayerCount)
{
	const int LayerDataPerAttribute = 4;
	const int StartingAttributeIndex = 9;
	int AttributeCount = (InterpolationLayerCount + 3) / LayerDataPerAttribute;

	std::vector<int> Result;
	for (int i = 0; i < AttributeCount; i++)
		Result.push_back(StartingAttributeIndex + i);

	return Result;
};

void AnalysisObjectManager::ComplexityMetricDataToGPU(std::string LayerID, int GPULayerIndex)
{
	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr)
		return;

	DataLayer* CurrentLayer = ActiveObject->GetLayer(LayerID);
	if (CurrentLayer == nullptr)
		return;

	if (ActiveObject->GetType() == DATA_SOURCE_TYPE::MESH)
	{
		MeshAnalysisData* CurrentMeshAnalysisData = ActiveObject->GetMeshAnalysisData();
		if (CurrentMeshAnalysisData == nullptr)
			return;

		FEMesh* ActiveMesh = static_cast<FEMesh*>(ActiveObject->GetEngineResource());
		if (ActiveMesh == nullptr)
			return;

		LayerInterpolationData* CurrentInterpolationData = CurrentLayer->GetInterpolationData();
		if (CurrentLayer->GetType() == LAYER_TYPE::INTERPOLATION && CurrentInterpolationData == nullptr)
			return;

		CurrentLayer->FillRawData();

		FE_GL_ERROR(glBindVertexArray(ActiveMesh->GetVaoID()));

		if (GPULayerIndex == 0)
		{
			if (CurrentLayer->GetType() == LAYER_TYPE::INTERPOLATION)
			{
				for (size_t i = 0; i < CurrentMeshAnalysisData->InterpolationLayerBufferIDs.size(); i++)
				{
					if (CurrentMeshAnalysisData->InterpolationLayerBufferIDs[i] != GLuint(-1))
						FE_GL_ERROR(glDeleteBuffers(1, &CurrentMeshAnalysisData->InterpolationLayerBufferIDs[i]));
				}
				CurrentMeshAnalysisData->InterpolationLayerBufferIDs.clear();

				if (CurrentInterpolationData->RawData.size() == CurrentInterpolationData->GetLayerCount() && CurrentInterpolationData->GetLayerCount() > 0)
				{
					int PerLayerDataCount = static_cast<int>(CurrentInterpolationData->RawData[0].size());
					int InterpolationLayerCount = static_cast<int>(CurrentInterpolationData->GetLayerCount());
					std::vector<int> AttributeIndexes = GetVertexAttributeIndexes(InterpolationLayerCount);
					int BufferCount = static_cast<int>(AttributeIndexes.size());
					for (size_t i = 0; i < BufferCount; i++)
					{
						CurrentMeshAnalysisData->InterpolationLayerBufferIDs.push_back(GLuint(-1));
						FE_GL_ERROR(glGenBuffers(1, &CurrentMeshAnalysisData->InterpolationLayerBufferIDs.back()));
						FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, CurrentMeshAnalysisData->InterpolationLayerBufferIDs.back()));

						std::vector<glm::vec4> PackedData;
						for (size_t j = 0; j < PerLayerDataCount; j++)
						{
							glm::vec4 CurrentValue;
							CurrentValue.x = (i * 4 + 0) < InterpolationLayerCount ? CurrentInterpolationData->RawData[i * 4 + 0][j] : 0.0f;
							CurrentValue.y = (i * 4 + 1) < InterpolationLayerCount ? CurrentInterpolationData->RawData[i * 4 + 1][j] : 0.0f;
							CurrentValue.z = (i * 4 + 2) < InterpolationLayerCount ? CurrentInterpolationData->RawData[i * 4 + 2][j] : 0.0f;
							CurrentValue.w = (i * 4 + 3) < InterpolationLayerCount ? CurrentInterpolationData->RawData[i * 4 + 3][j] : 0.0f;

							PackedData.push_back(CurrentValue);
						}

						FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * PackedData.size() * 4, PackedData.data(), GL_STATIC_DRAW));
						FE_GL_ERROR(glVertexAttribPointer(AttributeIndexes[i], 4, GL_FLOAT, false, 0, nullptr));
					}
				}
			}
			else
			{
				if (CurrentMeshAnalysisData->FirstLayerBufferID != GLuint(-1))
					FE_GL_ERROR(glDeleteBuffers(1, &CurrentMeshAnalysisData->FirstLayerBufferID));

				CurrentMeshAnalysisData->FirstLayerBufferID = 0;
				FE_GL_ERROR(glGenBuffers(1, &CurrentMeshAnalysisData->FirstLayerBufferID));
				FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, CurrentMeshAnalysisData->FirstLayerBufferID));
				FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * CurrentLayer->RawData.size(), CurrentLayer->RawData.data(), GL_STATIC_DRAW));
				FE_GL_ERROR(glVertexAttribPointer(7, 1, GL_FLOAT, false, 0, nullptr));
			}
		}
		else
		{
			CurrentMeshAnalysisData->SecondLayerBufferID = 0;
			FE_GL_ERROR(glGenBuffers(1, &CurrentMeshAnalysisData->SecondLayerBufferID));
			FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, CurrentMeshAnalysisData->SecondLayerBufferID));
			FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * CurrentLayer->RawData.size(), CurrentLayer->RawData.data(), GL_STATIC_DRAW));
			FE_GL_ERROR(glVertexAttribPointer(8, 3, GL_FLOAT, false, 0, nullptr));
		}

		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}
	else if (ActiveObject->GetType() == DATA_SOURCE_TYPE::POINT_CLOUD)
	{
		PointCloudAnalysisData* CurrentPointCloudAnalysisData = ActiveObject->GetPointCloudAnalysisData();
		if (CurrentPointCloudAnalysisData == nullptr)
			return;

		FEPointCloud* PointCloud = static_cast<FEPointCloud*>(ActiveObject->GetEngineResource());
		if (PointCloud == nullptr)
			return;

		for (size_t i = 0; i < CurrentLayer->ValuesComputeShaderBuffers.size(); i++)
		{
			if (CurrentLayer->ValuesComputeShaderBuffers[i] != GLuint(-1))
				FE_GL_ERROR(glDeleteBuffers(1, &CurrentLayer->ValuesComputeShaderBuffers[i]));
		}
		CurrentLayer->ValuesComputeShaderBuffers.clear();

		for (size_t i = 0; i < PointCloud->GetPointCount(); i += FEPointCloud::MaxPointsPerBuffer)
		{
			CurrentLayer->ValuesComputeShaderBuffers.resize(CurrentLayer->ValuesComputeShaderBuffers.size() + 1);
			FE_GL_ERROR(glGenBuffers(1, &CurrentLayer->ValuesComputeShaderBuffers.back()));
			FE_GL_ERROR(glBindBuffer(GL_SHADER_STORAGE_BUFFER, CurrentLayer->ValuesComputeShaderBuffers.back()));
			FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, CurrentLayer->ValuesComputeShaderBuffers.back()));

			// Calculate the number of points for the current buffer
			size_t NumberOfElement = std::min(FEPointCloud::MaxPointsPerBuffer, CurrentLayer->ElementsToData.size() - i);
			FE_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * NumberOfElement, CurrentLayer->ElementsToData.data() + i, GL_DYNAMIC_DRAW));
		}
	}
}

int AnalysisObjectManager::GetTriangleIndexUnderMouse(float* HitDistance)
{
	int Result = -1;

	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr)
		return Result;

	FEEntity* ActiveEntity = ANALYSIS_OBJECT_MANAGER.GetActiveEntity();
	if (ActiveEntity == nullptr)
		return Result;

	FEMesh* ActiveMesh = static_cast<FEMesh*>(ActiveObject->GetEngineResource());
	if (ActiveMesh == nullptr)
		return Result;

	MeshAnalysisData* CurrentMeshAnalysisData = ActiveObject->GetMeshAnalysisData();
	if (CurrentMeshAnalysisData == nullptr)
		return Result;

	glm::dvec3 MouseRay = MAIN_SCENE_MANAGER.GetMouseRayDirection();

	double CurrentDistance = std::numeric_limits<double>::max();
	double LastDistance = std::numeric_limits<double>::max();

	for (int i = 0; i < CurrentMeshAnalysisData->Triangles.size(); i++)
	{
		std::vector<glm::dvec3> TranformedTrianglePoints = CurrentMeshAnalysisData->Triangles[i];
		for (size_t j = 0; j < TranformedTrianglePoints.size(); j++)
		{
			TranformedTrianglePoints[j] = ActiveEntity->GetComponent<FETransformComponent>().GetWorldMatrix() * glm::vec4(TranformedTrianglePoints[j], 1.0f);
		}

		const bool bHit = GEOMETRY.IsRayIntersectingTriangle(MAIN_SCENE_MANAGER.GetMainCamera()->GetComponent<FETransformComponent>().GetPosition(FE_WORLD_SPACE), MouseRay, TranformedTrianglePoints, CurrentDistance);

		if (bHit && CurrentDistance < LastDistance)
		{
			LastDistance = CurrentDistance;
			Result = i;
			if (HitDistance != nullptr)
				*HitDistance = static_cast<float>(CurrentDistance);
		}
	}

	return Result;
}

bool AnalysisObjectManager::SelectTriangleByIndex(int TriangleIndex)
{
	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr)
		return false;

	FEEntity* ActiveEntity = ANALYSIS_OBJECT_MANAGER.GetActiveEntity();
	if (ActiveEntity == nullptr)
		return false;

	FEMesh* ActiveMesh = static_cast<FEMesh*>(ActiveObject->GetEngineResource());
	if (ActiveMesh == nullptr)
		return false;

	MeshAnalysisData* CurrentMeshAnalysisData = ActiveObject->GetMeshAnalysisData();
	if (CurrentMeshAnalysisData == nullptr)
		return false;

	if (TriangleIndex < 0 || TriangleIndex >= CurrentMeshAnalysisData->Triangles.size())
		return false;

	CurrentMeshAnalysisData->TriangleSelected.clear();
	CurrentMeshAnalysisData->TriangleSelected.push_back(TriangleIndex);
	return true;
}

glm::vec3 AnalysisObjectManager::IntersectTriangle(glm::dvec3 MouseRay)
{
	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr)
		return glm::vec3(0.0f);

	FEEntity* ActiveEntity = ANALYSIS_OBJECT_MANAGER.GetActiveEntity();
	if (ActiveEntity == nullptr)
		return glm::vec3(0.0f);

	FEMesh* ActiveMesh = static_cast<FEMesh*>(ActiveObject->GetEngineResource());
	if (ActiveMesh == nullptr)
		return glm::vec3(0.0f);

	MeshAnalysisData* CurrentMeshAnalysisData = ActiveObject->GetMeshAnalysisData();
	if (CurrentMeshAnalysisData == nullptr)
		return glm::vec3(0.0f);

	double CurrentDistance = 0.0;
	double LastDistance = 9999.0;

	for (size_t i = 0; i < CurrentMeshAnalysisData->Triangles.size(); i++)
	{
		std::vector<glm::dvec3> TranformedTrianglePoints = CurrentMeshAnalysisData->Triangles[i];
		for (size_t j = 0; j < TranformedTrianglePoints.size(); j++)
		{
			TranformedTrianglePoints[j] = ActiveEntity->GetComponent<FETransformComponent>().GetWorldMatrix() * glm::vec4(TranformedTrianglePoints[j], 1.0f);
		}

		glm::dvec3 HitPosition;
		const bool bHit = GEOMETRY.IsRayIntersectingTriangle(MAIN_SCENE_MANAGER.GetMainCamera()->GetComponent<FETransformComponent>().GetPosition(FE_WORLD_SPACE), MouseRay, TranformedTrianglePoints, CurrentDistance, &HitPosition);

		if (bHit && CurrentDistance < LastDistance)
		{
			LastDistance = CurrentDistance;

			const glm::mat4 Inverse = glm::inverse(ActiveEntity->GetComponent<FETransformComponent>().GetWorldMatrix());
			return Inverse * glm::vec4(HitPosition, 1.0f);
		}
	}

	return glm::vec3(0.0f);
}

std::vector<int> AnalysisObjectManager::GetTriangleIndexesInRadius(float Radius)
{
	std::vector<int> Result;

	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr)
		return Result;

	FEEntity* ActiveEntity = ANALYSIS_OBJECT_MANAGER.GetActiveEntity();
	if (ActiveEntity == nullptr)
		return Result;

	FEMesh* ActiveMesh = static_cast<FEMesh*>(ActiveObject->GetEngineResource());
	if (ActiveMesh == nullptr)
		return Result;

	MeshAnalysisData* CurrentMeshAnalysisData = ActiveObject->GetMeshAnalysisData();
	if (CurrentMeshAnalysisData == nullptr)
		return Result;

	int TriangleIndexUnderMouse = GetTriangleIndexUnderMouse();
	if (TriangleIndexUnderMouse == -1)
		return Result;

	CurrentMeshAnalysisData->MeasuredRugosityAreaRadius = Radius;
	CurrentMeshAnalysisData->MeasuredRugosityAreaCenter = ActiveEntity->GetComponent<FETransformComponent>().GetWorldMatrix() * glm::vec4(CurrentMeshAnalysisData->TrianglesCentroids[TriangleIndexUnderMouse], 1.0f);

	const glm::dvec3 FirstSelectedTriangleCentroid = CurrentMeshAnalysisData->TrianglesCentroids[TriangleIndexUnderMouse];

	for (size_t i = 0; i < CurrentMeshAnalysisData->Triangles.size(); i++)
	{
		if (i == TriangleIndexUnderMouse)
			continue;

		if (glm::distance(FirstSelectedTriangleCentroid, CurrentMeshAnalysisData->TrianglesCentroids[i]) <= Radius)
			Result.push_back(static_cast<int>(i));
	}

	return Result;
}

bool AnalysisObjectManager::SelectTrianglesByIndexes(std::vector<int> TriangleIndexes)
{
	bool bResult = false;

	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr)
		return bResult;

	FEEntity* ActiveEntity = ANALYSIS_OBJECT_MANAGER.GetActiveEntity();
	if (ActiveEntity == nullptr)
		return bResult;

	FEMesh* ActiveMesh = static_cast<FEMesh*>(ActiveObject->GetEngineResource());
	if (ActiveMesh == nullptr)
		return bResult;

	MeshAnalysisData* CurrentMeshAnalysisData = ActiveObject->GetMeshAnalysisData();
	if (CurrentMeshAnalysisData == nullptr)
		return bResult;

	CurrentMeshAnalysisData->TriangleSelected.clear();
	for (size_t i = 0; i < TriangleIndexes.size(); i++)
	{
		if (TriangleIndexes[i] < 0 || TriangleIndexes[i] >= CurrentMeshAnalysisData->Triangles.size())
			continue;

		CurrentMeshAnalysisData->TriangleSelected.push_back(TriangleIndexes[i]);
		bResult = true;
	}
	
	return bResult;
}
#include "UI/UIManager.h"
void AnalysisObjectManager::UpdateMeshUniforms(AnalysisObject* Object)
{
	if (Object == nullptr)
		return;

	if (Object->GetType() == DATA_SOURCE_TYPE::MESH)
	{
		FEMesh* ActiveMesh = static_cast<FEMesh*>(Object->GetEngineResource());
		if (ActiveMesh == nullptr)
			return;

		MeshAnalysisData* CurrentMeshAnalysisData = Object->GetMeshAnalysisData();
		if (CurrentMeshAnalysisData == nullptr)
			return;

		DataLayer* ActiveLayer = Object->GetActiveLayer();

		ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("AmbientFactor", SETTINGS_WINDOW.GetAmbientLightFactor());
		ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("HaveColor", ActiveMesh->GetColorCount() == 0 ? 0 : 1);
		ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("HeatMapType", CurrentMeshAnalysisData->GetHeatMapType());
		ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("LayerIndex", Object->GetActiveLayerIndex());

		if (ActiveLayer != nullptr)
		{
			LayerInterpolationData* CurrentInterpolationData = ActiveLayer->GetInterpolationData();
			ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("InterpolationActive", CurrentInterpolationData == nullptr ? 0 : 1);
			if (CurrentInterpolationData != nullptr)
			{
				ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("InterpolationLayerCount", static_cast<int>(CurrentInterpolationData->GetLayerCount()));
				ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("InterpolationLayersMin", CurrentInterpolationData->GetLayersMinValues());
				ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("InterpolationLayersMax", CurrentInterpolationData->GetLayersMaxValues());

				ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("InterpolationFactor", CurrentInterpolationData->GetInterpolationFactor());

				ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("InterpolateMinMaxValues", CurrentInterpolationData->IsMinMaxInterpolationEnabled() ? 1 : 0);
			}
		}
		else
		{
			ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("InterpolationActive", 0);
		}
		
		ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("UnselectedAreaSaturationFactor", CurrentMeshAnalysisData->GetUnselectedAreaSaturationFactor());
		ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("UnselectedAreaBrightnessFactor", CurrentMeshAnalysisData->GetUnselectedAreaBrightnessFactor());

		if (ActiveLayer != nullptr)
		{
			ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("SelectedRangeMin", ActiveLayer->GetSelectedRangeMin());
			ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("SelectedRangeMax", ActiveLayer->GetSelectedRangeMax());
		}
		else
		{
			ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("SelectedRangeMin", 0.0f);
			ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("SelectedRangeMax", 0.0f);
		}

		if (ActiveLayer != nullptr)
		{
			ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("LayerMin", float(ActiveLayer->MinVisible));
			ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("LayerMax", float(ActiveLayer->MaxVisible));

			ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("LayerAbsoluteMin", float(ActiveLayer->GetMin()));
			ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("LayerAbsoluteMax", float(ActiveLayer->GetMax()));
		}

		AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
		ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("MeasuredRugosityAreaRadius", -1.0f);
		if (ActiveObject != nullptr)
		{
			if (CurrentMeshAnalysisData->TriangleSelected.size() > 1 && UI_INSPECTOR.GetMeshSelectionMode() == 2)
			{
				float TempMeasuredRugosityAreaRadius = 0.0f;
				glm::vec3 TempMeasuredRugosityAreaCenter = glm::vec3(0.0f);
				CurrentMeshAnalysisData->GetMeasuredRugosityArea(TempMeasuredRugosityAreaRadius, TempMeasuredRugosityAreaCenter);
				ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("MeasuredRugosityAreaRadius", TempMeasuredRugosityAreaRadius);
				ANALYSIS_OBJECT_MANAGER.CustomMeshShader->UpdateUniformData("MeasuredRugosityAreaCenter", TempMeasuredRugosityAreaCenter);
			}
		}
	}
}

size_t AnalysisObjectManager::GetAnalysisObjectCount()
{
	return AnalysisObjects.size();
}

AnalysisObject* AnalysisObjectManager::GetAnalysisObjectByID(std::string ID)
{
	if (AnalysisObjects.find(ID) != AnalysisObjects.end())
		return AnalysisObjects[ID];

	return nullptr;
}

AnalysisObject* AnalysisObjectManager::GetAnalysisObjectByEntityID(std::string EntityID)
{
	for (auto& CurrentPair : AnalysisObjects)
	{
		if (CurrentPair.second->Entity != nullptr && CurrentPair.second->Entity->GetObjectID() == EntityID)
			return CurrentPair.second;
	}

	return nullptr;
}

AnalysisObject* AnalysisObjectManager::GetActiveAnalysisObject()
{
	return GetAnalysisObjectByID(ActiveAnalysisObjectID);
}

bool AnalysisObjectManager::SetActiveAnalysisObject(std::string ID)
{
	if (ID == ActiveAnalysisObjectID)
		return true;

	if (ID.empty())
	{
		ActiveAnalysisObjectID = "";
		for (size_t i = 0; i < ClientOnActiveObjectChangeCallbacks.size(); i++)
		{
			if (ClientOnActiveObjectChangeCallbacks[i] == nullptr)
				continue;
			ClientOnActiveObjectChangeCallbacks[i](nullptr);
		}

		return true;
	}

	if (AnalysisObjects.find(ID) != AnalysisObjects.end())
	{
		ActiveAnalysisObjectID = ID;
		AnalysisObject* NewActiveObject = GetActiveAnalysisObject();
		DataLayer* ActiveLayer = NewActiveObject->GetActiveLayer();
		if (ActiveLayer != nullptr)
			NewActiveObject->SetActiveLayer(ActiveLayer->GetID(), true);

		for (size_t i = 0; i < ClientOnActiveObjectChangeCallbacks.size(); i++)
		{
			if (ClientOnActiveObjectChangeCallbacks[i] == nullptr)
				continue;
			ClientOnActiveObjectChangeCallbacks[i](GetActiveAnalysisObject());
		}

		return true;
	}

	return false;
}

std::vector<std::string> AnalysisObjectManager::GetAnalysisObjectsIDList()
{
	FE_MAP_TO_STR_VECTOR(AnalysisObjects)
}

PointCloudAnalysisData* AnalysisObjectManager::ExtractAdditionalGeometryData(FEPointCloud* PointCloud)
{
	PointCloudAnalysisData* Result = new PointCloudAnalysisData();

	std::vector<FEPointCloudVertex> TemporaryRawData = PointCloud->GetRawData();
	Result->RawPointCloudData.resize(TemporaryRawData.size());
	Result->OriginalColors.resize(TemporaryRawData.size());
	for (size_t i = 0; i < TemporaryRawData.size(); i++)
	{
		Result->RawPointCloudData[i].X = static_cast<double>(TemporaryRawData[i].X);
		Result->RawPointCloudData[i].Y = static_cast<double>(TemporaryRawData[i].Y);
		Result->RawPointCloudData[i].Z = static_cast<double>(TemporaryRawData[i].Z);

		Result->RawPointCloudData[i].R = TemporaryRawData[i].R;
		Result->RawPointCloudData[i].G = TemporaryRawData[i].G;
		Result->RawPointCloudData[i].B = TemporaryRawData[i].B;
		Result->RawPointCloudData[i].A = TemporaryRawData[i].A;

		Result->OriginalColors[i] = { TemporaryRawData[i].R, TemporaryRawData[i].G, TemporaryRawData[i].B, TemporaryRawData[i].A };
	}

	Result->AABB = PointCloud->GetAABB();
	return Result;
}

PointCloudAnalysisData* AnalysisObjectManager::ExtractAdditionalGeometryData(std::vector<FEPointCloudVertexDouble>& PointCloudVertices, FEAABB PointCloudAABB)
{
	PointCloudAnalysisData* Result = new PointCloudAnalysisData();

	Result->RawPointCloudData = std::move(PointCloudVertices);
	Result->OriginalColors.resize(Result->RawPointCloudData.size());
	for (size_t i = 0; i < Result->RawPointCloudData.size(); i++)
	{
		FEPointCloudVertexDouble& CurrentVertex = Result->RawPointCloudData[i];
		Result->OriginalColors[i] = { CurrentVertex.R, CurrentVertex.G, CurrentVertex.B, CurrentVertex.A };
	}

	Result->AABB = PointCloudAABB;
	return Result;
}

FEEntity* AnalysisObjectManager::GetActiveEntity()
{
	AnalysisObject* ActiveObject = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (ActiveObject == nullptr)
		return nullptr;

	return ActiveObject->GetEntity();
}

bool AnalysisObjectManager::DeleteAnalysisObject(std::string ID)
{
	AnalysisObject* ObjectToDelete = GetAnalysisObjectByID(ID);
	if (ObjectToDelete == nullptr)
		return false;

	for (size_t i = 0; i < ClientOnObjectDeleteCallbacks.size(); i++)
	{
		if (ClientOnObjectDeleteCallbacks[i] == nullptr)
			continue;

		ClientOnObjectDeleteCallbacks[i](ObjectToDelete);
	}

	if (GetActiveAnalysisObject() == ObjectToDelete)
		SetActiveAnalysisObject("");

	FEEntity* EntityToDelete = ObjectToDelete->GetEntity();
	if (EntityToDelete != nullptr)
	{
		RENDERER.RemoveBeforeRenderCallback(ObjectToDelete->Entity, AnalysisObjectManager::BeforeRender);
		MAIN_SCENE_MANAGER.GetMainScene()->DeleteEntity(EntityToDelete);
	}

	if (ObjectToDelete->GetType() == DATA_SOURCE_TYPE::MESH)
	{
		FEMesh* MeshToDelete = static_cast<FEMesh*>(ObjectToDelete->GetEngineResource());
		if (MeshToDelete != nullptr)
			RESOURCE_MANAGER.DeleteFEMesh(MeshToDelete);
	}
	else if (ObjectToDelete->GetType() == DATA_SOURCE_TYPE::POINT_CLOUD)
	{
		FEPointCloud* PointCloudToDelete = static_cast<FEPointCloud*>(ObjectToDelete->GetEngineResource());
		if (PointCloudToDelete != nullptr)
			RESOURCE_MANAGER.DeleteFEPointCloud(PointCloudToDelete);
	}

	delete ObjectToDelete;
	AnalysisObjects.erase(ID);
	return true;
}

void AnalysisObjectManager::InitializeSceneObjects(AnalysisObject* NewAnalysisObject)
{
	if (APPLICATION.HasConsoleWindow())
		return;

	if (NewAnalysisObject == nullptr)
		return;

	if (NewAnalysisObject->GetType() == DATA_SOURCE_TYPE::MESH)
	{
		FEMesh* ActiveMesh = static_cast<FEMesh*>(NewAnalysisObject->GetEngineResource());
		if (ActiveMesh == nullptr)
			return;

		FEGameModel* NewGameModel = RESOURCE_MANAGER.CreateGameModel(ActiveMesh, ANALYSIS_OBJECT_MANAGER.CustomMaterial);
		NewAnalysisObject->Entity = MAIN_SCENE_MANAGER.GetMainScene()->CreateEntity("Mesh entity");
		NewAnalysisObject->Entity->AddComponent<FEGameModelComponent>(NewGameModel);
	}
	else if (NewAnalysisObject->GetType() == DATA_SOURCE_TYPE::POINT_CLOUD)
	{
		FEPointCloud* PointCloud = static_cast<FEPointCloud*>(NewAnalysisObject->GetEngineResource());
		NewAnalysisObject->Entity = MAIN_SCENE_MANAGER.GetMainScene()->CreateEntity("Point cloud entity");
		NewAnalysisObject->Entity->AddComponent<FEPointCloudComponent>(PointCloud);
		PointCloud->SetAdvancedRenderingEnabled(true);
	}

	RENDERER.AddBeforeRenderCallback(NewAnalysisObject->Entity, AnalysisObjectManager::BeforeRender);
}

void AnalysisObjectManager::SaveToRUGFileAskForFilePath()
{
	std::string FilePath;
	FILE_SYSTEM.ShowFileSaveDialog(FilePath, RUGOSITY_SAVE_FILE_FILTER, 1);

	SaveToRUGFile(FilePath);
}

void AnalysisObjectManager::SaveAnalysisDataToRUGFile(std::fstream& File, AnalysisObject* Object)
{
	ResourceAnalysisData* AnalysisData = Object->GetAnalysisData();
	if (AnalysisData == nullptr)
		return;

	switch (Object->GetType())
	{
		case DATA_SOURCE_TYPE::MESH:
			SaveMeshDataToRUGFile(File, Object);
		break;

		case DATA_SOURCE_TYPE::POINT_CLOUD:
			SavePointCloudToRUGFile(File, Object);
		break;
	default:
		break;
	}
}

void AnalysisObjectManager::SaveMeshDataToRUGFile(std::fstream& File, AnalysisObject* Object)
{
	MeshAnalysisData* CurrentMeshAnalysisData = Object->GetMeshAnalysisData();
	if (CurrentMeshAnalysisData == nullptr)
		return;

	int DebugWrittenBytes = 0;

	File.write((char*)&Object->AppliedShift.x, sizeof(double));
	File.write((char*)&Object->AppliedShift.y, sizeof(double));
	File.write((char*)&Object->AppliedShift.z, sizeof(double));

	int Count = static_cast<int>(CurrentMeshAnalysisData->Vertices.size());
	File.write((char*)&Count, sizeof(int));
	DebugWrittenBytes += sizeof(int);
	File.write((char*)CurrentMeshAnalysisData->Vertices.data(), sizeof(double) * Count);
	DebugWrittenBytes += sizeof(double) * Count;

	Count = static_cast<int>(CurrentMeshAnalysisData->Colors.size());
	File.write((char*)&Count, sizeof(int));
	DebugWrittenBytes += sizeof(int);
	File.write((char*)CurrentMeshAnalysisData->Colors.data(), sizeof(float) * Count);
	DebugWrittenBytes += sizeof(float) * Count;

	Count = static_cast<int>(CurrentMeshAnalysisData->UVs.size());
	File.write((char*)&Count, sizeof(int));
	DebugWrittenBytes += sizeof(int);
	File.write((char*)CurrentMeshAnalysisData->UVs.data(), sizeof(float) * Count);
	DebugWrittenBytes += sizeof(float) * Count;

	Count = static_cast<int>(CurrentMeshAnalysisData->Normals.size());
	File.write((char*)&Count, sizeof(int));
	DebugWrittenBytes += sizeof(int);
	File.write((char*)CurrentMeshAnalysisData->Normals.data(), sizeof(float) * Count);
	DebugWrittenBytes += sizeof(float) * Count;

	Count = static_cast<int>(CurrentMeshAnalysisData->Tangents.size());
	File.write((char*)&Count, sizeof(int));
	DebugWrittenBytes += sizeof(int);
	File.write((char*)CurrentMeshAnalysisData->Tangents.data(), sizeof(float) * Count);
	DebugWrittenBytes += sizeof(float) * Count;

	Count = static_cast<int>(CurrentMeshAnalysisData->Indices.size());
	File.write((char*)&Count, sizeof(int));
	DebugWrittenBytes += sizeof(int);
	File.write((char*)CurrentMeshAnalysisData->Indices.data(), sizeof(int) * Count);
	DebugWrittenBytes += sizeof(int) * Count;

	FEAABB CurrentAABB = CurrentMeshAnalysisData->GetAABB();
	File.write((char*)&CurrentAABB.GetMin()[0], sizeof(float));
	File.write((char*)&CurrentAABB.GetMin()[1], sizeof(float));
	File.write((char*)&CurrentAABB.GetMin()[2], sizeof(float));

	File.write((char*)&CurrentAABB.GetMax()[0], sizeof(float));
	File.write((char*)&CurrentAABB.GetMax()[1], sizeof(float));
	File.write((char*)&CurrentAABB.GetMax()[2], sizeof(float));

	DebugWrittenBytes += sizeof(float) * 6;
}

void AnalysisObjectManager::SavePointCloudToRUGFile(std::fstream& File, AnalysisObject* Object)
{
	PointCloudAnalysisData* CurrentPointCloudAnalysisData = Object->GetPointCloudAnalysisData();
	if (CurrentPointCloudAnalysisData == nullptr)
		return;

	File.write((char*)&Object->AppliedShift.x, sizeof(double));
	File.write((char*)&Object->AppliedShift.y, sizeof(double));
	File.write((char*)&Object->AppliedShift.z, sizeof(double));

	std::vector<FEPointCloudVertexDouble>& Vertices = CurrentPointCloudAnalysisData->RawPointCloudData;

	size_t Count = Vertices.size() * 3;
	std::vector<float> Positions(Count);
	for (size_t i = 0; i < Vertices.size(); i++)
	{
		Positions[i * 3] = static_cast<float>(Vertices[i].X);
		Positions[i * 3 + 1] = static_cast<float>(Vertices[i].Y);
		Positions[i * 3 + 2] = static_cast<float>(Vertices[i].Z);
	}

	File.write((char*)&Count, sizeof(size_t));
	File.write((char*)Positions.data(), sizeof(float) * Count);

	Count = Vertices.size() * 4;
	std::vector<unsigned char> Colors(Count);
	// Use original colors, because point cloud colors can be modified in the application.
	for (size_t i = 0; i < Vertices.size(); i++)
	{
		std::vector<unsigned char>& OriginalColor = CurrentPointCloudAnalysisData->OriginalColors[i];

		Colors[i * 4] = OriginalColor[0];
		Colors[i * 4 + 1] = OriginalColor[1];
		Colors[i * 4 + 2] = OriginalColor[2];
		Colors[i * 4 + 3] = OriginalColor[3];
	}

	File.write((char*)&Count, sizeof(size_t));
	File.write((char*)Colors.data(), sizeof(unsigned char) * Count);
}

void AnalysisObjectManager::SaveLayersDataToRUGFile(std::fstream& File, AnalysisObject* Object)
{
	int Count = static_cast<int>(Object->Layers.size());
	File.write((char*)&Count, sizeof(int));
	for (size_t i = 0; i < Object->Layers.size(); i++)
	{
		DataLayer* CurrentLayer = Object->Layers[i];
		LAYER_TYPE LayerType = CurrentLayer->GetType();
		File.write((char*)&LayerType, sizeof(LAYER_TYPE));

		int LayerIDSize = static_cast<int>(CurrentLayer->GetID().size() + 1);
		File.write((char*)&LayerIDSize, sizeof(int));
		File.write((char*)CurrentLayer->GetID().c_str(), sizeof(char) * LayerIDSize);

		int ParentIDSize = (int)CurrentLayer->ParentObjectIDs.size();
		File.write((char*)&ParentIDSize, sizeof(int));
		for (size_t j = 0; j < CurrentLayer->ParentObjectIDs.size(); j++)
		{
			int SingleParentIDSize = static_cast<int>(CurrentLayer->ParentObjectIDs[j].size() + 1);
			File.write((char*)&SingleParentIDSize, sizeof(int));
			File.write((char*)CurrentLayer->ParentObjectIDs[j].c_str(), sizeof(char) * SingleParentIDSize);
		}

		Count = static_cast<int>(CurrentLayer->GetCaption().size());
		File.write((char*)&Count, sizeof(int));
		File.write((char*)CurrentLayer->GetCaption().c_str(), sizeof(char) * Count);

		Count = static_cast<int>(CurrentLayer->GetNote().size());
		File.write((char*)&Count, sizeof(int));
		File.write((char*)CurrentLayer->GetNote().c_str(), sizeof(char) * Count);

		LayerInterpolationData* InterpolationData = CurrentLayer->GetInterpolationData();
		if (InterpolationData != nullptr)
		{
			std::vector<std::string> UsedLayerIDs = InterpolationData->GetUsedLayerIDs();
			int UsedLayerIDSize = (int)UsedLayerIDs.size();
			File.write((char*)&UsedLayerIDSize, sizeof(int));
			for (size_t j = 0; j < UsedLayerIDs.size(); j++)
			{
				int SingleUsedLayerIDSize = static_cast<int>(UsedLayerIDs[j].size() + 1);
				File.write((char*)&SingleUsedLayerIDSize, sizeof(int));
				File.write((char*)UsedLayerIDs[j].c_str(), sizeof(char) * SingleUsedLayerIDSize);
			}

			float InterpolationFactor = InterpolationData->GetInterpolationFactor();
			File.write((char*)&InterpolationFactor, sizeof(float));

			int MinMaxInterpolationEnabled = InterpolationData->IsMinMaxInterpolationEnabled();
			File.write((char*)&MinMaxInterpolationEnabled, sizeof(int));

			int ElementPerLayerCount = static_cast<int>(InterpolationData->RawData[0].size());
			File.write((char*)&ElementPerLayerCount, sizeof(int));
			for (size_t j = 0; j < InterpolationData->RawData.size(); j++)
			{
				File.write((char*)InterpolationData->RawData[j].data(), sizeof(float) * ElementPerLayerCount);
			}
		}
		else
		{
			Count = static_cast<int>(CurrentLayer->ElementsToData.size());
			File.write((char*)&Count, sizeof(int));
			File.write((char*)CurrentLayer->ElementsToData.data(), sizeof(float) * Count);
		}

		Count = CurrentLayer->DebugInfo != nullptr;
		File.write((char*)&Count, sizeof(int));
		if (Count)
			CurrentLayer->DebugInfo->ToFile(File);
	}
}

void AnalysisObjectManager::SaveAnnotationsDataToRUGFile(std::fstream& File, AnalysisObject* Object)
{
	int Indicator = 0;
	AnnotationData* CurrentAnnotationData = ANNOTATION_MANAGER.GetAnnotationDataByAnalysisObjectID(Object->GetID());
	if (CurrentAnnotationData == nullptr)
	{
		File.write((char*)&Indicator, sizeof(int));
		return;
	}

	Indicator = 1;
	File.write((char*)&Indicator, sizeof(int));

	std::vector<AnnotationInfo> AllAnnotationInfos = CurrentAnnotationData->GetAllAnnotationInfos();
	int AnnotationInfoCount = static_cast<int>(AllAnnotationInfos.size());
	File.write((char*)&AnnotationInfoCount, sizeof(int));
	for (size_t i = 0; i < AllAnnotationInfos.size(); i++)
	{
		File.write((char*)&AllAnnotationInfos[i].ID, sizeof(int));

		int StringSize = static_cast<int>(AllAnnotationInfos[i].Name.size());
		File.write((char*)&StringSize, sizeof(int));
		File.write((char*)AllAnnotationInfos[i].Name.c_str(), sizeof(char) * StringSize);

		StringSize = static_cast<int>(AllAnnotationInfos[i].Description.size());
		File.write((char*)&StringSize, sizeof(int));
		File.write((char*)AllAnnotationInfos[i].Description.c_str(), sizeof(char) * StringSize);

		File.write((char*)&AllAnnotationInfos[i].Color, sizeof(glm::vec4));
	}

	int VectorSize = static_cast<int>(CurrentAnnotationData->PerElementID.size());
	File.write((char*)&VectorSize, sizeof(int));
	File.write((char*)&CurrentAnnotationData->PerElementID.data()[0], sizeof(int) * VectorSize);
}

void AnalysisObjectManager::LoadAnnotationsDataFromRUGFile(std::fstream& File, AnalysisObject* Object)
{
	int Indicator = 0;
	File.read((char*)&Indicator, sizeof(int));
	if (Indicator == 0)
		return;

	if (!ANNOTATION_MANAGER.AddAnnotationToAnalysisObject(Object->GetID()))
		return;

	AnnotationData* NewAnnotationData = ANNOTATION_MANAGER.GetAnnotationDataByAnalysisObjectID(Object->GetID());
	if (NewAnnotationData == nullptr)
		return;

	int AnnotationInfoCount = 0;
	File.read((char*)&AnnotationInfoCount, sizeof(int));

	std::vector<AnnotationInfo> ReadAnnotationInfo;
	ReadAnnotationInfo.resize(AnnotationInfoCount);
	for (int i = 0; i < AnnotationInfoCount; i++)
	{
		File.read((char*)&ReadAnnotationInfo[i].ID, sizeof(int));
		ReadAnnotationInfo[i].Name = FILE_SYSTEM.ReadFEString(File);
		ReadAnnotationInfo[i].Description = FILE_SYSTEM.ReadFEString(File);
		File.read((char*)&ReadAnnotationInfo[i].Color, sizeof(glm::vec4));
	}

	NewAnnotationData->UsedAnnotations = ReadAnnotationInfo;

	int VectorSize = 0;
	File.read((char*)&VectorSize, sizeof(int));
	NewAnnotationData->PerElementID.resize(VectorSize);
	File.read((char*)NewAnnotationData->PerElementID.data(), VectorSize * sizeof(int));

	ANNOTATION_MANAGER.InitalizeBuffer(NewAnnotationData);
	ANNOTATION_MANAGER.UpdateBuffer(NewAnnotationData);
}

bool AnalysisObjectManager::SaveToRUGFile(std::string FilePath)
{
	if (FilePath.empty())
		return false;

	if (FilePath.find(".rug") == std::string::npos)
		FilePath += ".rug";

	std::fstream File;
	File.open(FilePath, std::ios::out | std::ios::binary);
	if (!File.is_open())
	{
		LOG.Add(std::string("Can't open file: ") + FilePath + " in function SaveToRUGFile.");
		return false;
	}

	float Version = APPLICATION_VERSION_FLOAT;
	File.write((char*)&Version, sizeof(float));

	size_t ObjectCount = ANALYSIS_OBJECT_MANAGER.AnalysisObjects.size();
	File.write((char*)&ObjectCount, sizeof(size_t));

	auto ObjectsMapIterator = ANALYSIS_OBJECT_MANAGER.AnalysisObjects.begin();
	while (ObjectsMapIterator != ANALYSIS_OBJECT_MANAGER.AnalysisObjects.end())
	{
		AnalysisObject* CurrentObject = ObjectsMapIterator->second;
		if (CurrentObject != nullptr)
		{
			int ObjectIDSize = static_cast<int>(CurrentObject->GetID().size() + 1);
			File.write((char*)&ObjectIDSize, sizeof(int));
			File.write((char*)CurrentObject->GetID().c_str(), sizeof(char) * ObjectIDSize);

			int ObjectNameSize = static_cast<int>(CurrentObject->GetName().size() + 1);
			File.write((char*)&ObjectNameSize, sizeof(int));
			File.write((char*)CurrentObject->GetName().c_str(), sizeof(char) * ObjectNameSize);

			DATA_SOURCE_TYPE ObjectType = CurrentObject->GetType();
			File.write((char*)&ObjectType, sizeof(DATA_SOURCE_TYPE));

			int FilePathSize = static_cast<int>(CurrentObject->GetFilePath().size() + 1);
			File.write((char*)&FilePathSize, sizeof(int));
			File.write((char*)CurrentObject->GetFilePath().c_str(), sizeof(char) * FilePathSize);

			FEEntity* CurrentEntity = CurrentObject->GetEntity();
			bool bVisibleInScene = false;
			if (CurrentEntity != nullptr)
				bVisibleInScene = CurrentEntity->IsVisible();
			int VisibleInScene = bVisibleInScene;
			if (APPLICATION.HasConsoleWindow())
				VisibleInScene = 1;
			File.write((char*)&VisibleInScene, sizeof(int));

			SaveAnalysisDataToRUGFile(File, CurrentObject);
			SaveLayersDataToRUGFile(File, CurrentObject);
			SaveAnnotationsDataToRUGFile(File, CurrentObject);
		}

		ObjectsMapIterator++;
	}

	File.close();
	return true;
}

void AnalysisObjectManager::LoadMeshDataFromRUGFile(std::fstream& File, AnalysisObject* Object)
{
	char* Buffer = new char[4];
	char* DoubleBuffer = new char[8];
	long long ArraySize = 0;

	File.read(DoubleBuffer, 8);
	Object->AppliedShift.x = *(double*)DoubleBuffer;
	File.read(DoubleBuffer, 8);
	Object->AppliedShift.y = *(double*)DoubleBuffer;
	File.read(DoubleBuffer, 8);
	Object->AppliedShift.z = *(double*)DoubleBuffer;

	File.read(Buffer, 4);
	const int VertexCount = *(int*)Buffer;

	int BytesPerVertex = 8;
	ArraySize = long long(VertexCount) * long long(BytesPerVertex);
	char* VertexBuffer = new char[ArraySize];
	File.read(VertexBuffer, ArraySize);

	File.read(Buffer, 4);
	const int ColorCount = *(int*)Buffer;
	char* ColorBuffer = nullptr;
	if (ColorCount != 0)
	{
		ArraySize = long long(ColorCount) * long long(4);
		ColorBuffer = new char[ArraySize];
		File.read(ColorBuffer, ArraySize);
	}

	File.read(Buffer, 4);
	const int TexCout = *(int*)Buffer;
	ArraySize = long long(TexCout) * long long(4);
	char* TexBuffer = new char[ArraySize];
	File.read(TexBuffer, ArraySize);

	File.read(Buffer, 4);
	const int NormCout = *(int*)Buffer;
	ArraySize = long long(NormCout) * long long(4);
	char* NormBuffer = new char[ArraySize];
	File.read(NormBuffer, ArraySize);

	File.read(Buffer, 4);
	const int TangCout = *(int*)Buffer;
	ArraySize = long long(TangCout) * long long(4);
	char* TangBuffer = new char[ArraySize];
	File.read(TangBuffer, ArraySize);

	File.read(Buffer, 4);
	const int IndexCout = *(int*)Buffer;
	ArraySize = long long(IndexCout) * long long(4);
	char* IndexBuffer = new char[ArraySize];
	File.read(IndexBuffer, ArraySize);
	
	FEAABB MeshAABB;
	glm::vec3 Min;
	File.read(Buffer, 4);
	Min.x = *(float*)Buffer;
	File.read(Buffer, 4);
	Min.y = *(float*)Buffer;
	File.read(Buffer, 4);
	Min.z = *(float*)Buffer;

	glm::vec3 Max;
	File.read(Buffer, 4);
	Max.x = *(float*)Buffer;
	File.read(Buffer, 4);
	Max.y = *(float*)Buffer;
	File.read(Buffer, 4);
	Max.z = *(float*)Buffer;

	MeshAABB = FEAABB(Min, Max);

	std::vector<double> FEVertices;
	FEMesh* NewMesh = nullptr;

	std::vector<float> FEFloatVertices;
	FEFloatVertices.resize(VertexCount);
	for (size_t i = 0; i < VertexCount; i++)
		FEFloatVertices[i] = static_cast<float>(((double*)VertexBuffer)[i]);
	
	NewMesh = RESOURCE_MANAGER.RawDataToMesh((float*)FEFloatVertices.data(), VertexCount,
											 (float*)TexBuffer, TexCout,
											 (float*)NormBuffer, NormCout,
											 (float*)TangBuffer, TangCout,
											 (int*)IndexBuffer, IndexCout,
											 (float*)ColorBuffer, ColorCount,
											 nullptr, 0, 0, "");

	FEVertices.resize(VertexCount);
	for (size_t i = 0; i < VertexCount; i++)
	{
		FEVertices[i] = ((double*)VertexBuffer)[i];
	}

	std::vector<float> FEColors;
	FEColors.resize(ColorCount);
	for (size_t i = 0; i < ColorCount; i++)
		FEColors[i] = ((float*)ColorBuffer)[i];
	
	std::vector<float> FEUVs;
	FEUVs.resize(TexCout);
	for (size_t i = 0; i < TexCout; i++)
		FEUVs[i] = ((float*)TexBuffer)[i];
	
	std::vector<float> FETangents;
	FETangents.resize(TangCout);
	for (size_t i = 0; i < TangCout; i++)
		FETangents[i] = ((float*)TangBuffer)[i];
	
	std::vector<int> FEIndices;
	FEIndices.resize(IndexCout);
	for (size_t i = 0; i < IndexCout; i++)
		FEIndices[i] = ((int*)IndexBuffer)[i];
	
	std::vector<float> FENormals;
	FENormals.resize(NormCout);
	for (size_t i = 0; i < NormCout; i++)
		FENormals[i] = ((float*)NormBuffer)[i];
	
	Object->EngineResource = NewMesh;
	Object->AnalysisData = ExtractAdditionalGeometryData(FEVertices, FEColors, FEUVs, FETangents, FEIndices, FENormals);

	delete[] Buffer;
	delete[] DoubleBuffer;
	delete[] VertexBuffer;
	delete[] TexBuffer;
	delete[] NormBuffer;
	delete[] TangBuffer;
	delete[] IndexBuffer;
	if (ColorBuffer != nullptr)
		delete[] ColorBuffer;
}

void AnalysisObjectManager::LoadPointCloudDataFromRUGFile(std::fstream& File, AnalysisObject* Object)
{
	char* Buffer_8Byte = new char[8];

	File.read(Buffer_8Byte, sizeof(double));
	Object->AppliedShift.x = *(double*)Buffer_8Byte;
	File.read(Buffer_8Byte, sizeof(double));
	Object->AppliedShift.y = *(double*)Buffer_8Byte;
	File.read(Buffer_8Byte, sizeof(double));
	Object->AppliedShift.z = *(double*)Buffer_8Byte;

	File.read(Buffer_8Byte, sizeof(size_t));
	const size_t PositionFloatCount = *(size_t*)Buffer_8Byte;
	char* PositionBuffer = new char[PositionFloatCount * sizeof(float)];
	File.read(PositionBuffer, PositionFloatCount * sizeof(float));

	File.read(Buffer_8Byte, sizeof(size_t));
	const size_t ColorByteCount = *(size_t*)Buffer_8Byte;
	char* ColorBuffer = new char[ColorByteCount * sizeof(unsigned char)];
	File.read(ColorBuffer, ColorByteCount * sizeof(unsigned char));

	std::vector<FEPointCloudVertex> PointCloudData;
	for (size_t i = 0; i < PositionFloatCount / 3; i++)
	{
		PointCloudData.push_back(FEPointCloudVertex());
		PointCloudData[i].X = *(float*)(PositionBuffer + i * 3 * sizeof(float));
		PointCloudData[i].Y = *(float*)(PositionBuffer + i * 3 * sizeof(float) + sizeof(float));
		PointCloudData[i].Z = *(float*)(PositionBuffer + i * 3 * sizeof(float) + sizeof(float) * 2);

		PointCloudData[i].R = *(unsigned char*)(ColorBuffer + i * 4 * sizeof(unsigned char));
		PointCloudData[i].G = *(unsigned char*)(ColorBuffer + i * 4 * sizeof(unsigned char) + sizeof(unsigned char));
		PointCloudData[i].B = *(unsigned char*)(ColorBuffer + i * 4 * sizeof(unsigned char) + sizeof(unsigned char) * 2);
		PointCloudData[i].A = *(unsigned char*)(ColorBuffer + i * 4 * sizeof(unsigned char) + sizeof(unsigned char) * 3);
	}

	FEPointCloud* NewPointCloud = RESOURCE_MANAGER.RawDataToFEPointCloud(PointCloudData, "", "", false);
	NewPointCloud->SetAdvancedRenderingEnabled(true);
	Object->EngineResource = NewPointCloud;
	Object->AnalysisData = ExtractAdditionalGeometryData(NewPointCloud);

	delete[] Buffer_8Byte;
	delete[] PositionBuffer;
	delete[] ColorBuffer;
}

void AnalysisObjectManager::LoadLayersDataFromRUGFile(std::fstream& File, AnalysisObject* Object)
{
	char* Buffer = new char[4];
	File.read(Buffer, 4);
	const int LayerCount = *(int*)Buffer;
	Object->Layers.resize(LayerCount);

	for (size_t i = 0; i < Object->Layers.size(); i++)
	{
		Object->Layers[i] = new DataLayer();

		File.read(Buffer, 4);
		const int LayerType = *(int*)Buffer;
		Object->Layers[i]->SetType(LAYER_TYPE(LayerType));

		Object->Layers[i]->ForceID(FILE_SYSTEM.ReadFEString(File));

		File.read(Buffer, 4);
		const int ParentsIDsCount = *(int*)Buffer;
		Object->Layers[i]->ParentObjectIDs.resize(ParentsIDsCount);
		for (size_t j = 0; j < ParentsIDsCount; j++)
			Object->Layers[i]->ParentObjectIDs[j] = FILE_SYSTEM.ReadFEString(File);
		
		Object->Layers[i]->SetCaption(FILE_SYSTEM.ReadFEString(File));
		Object->Layers[i]->SetNote(FILE_SYSTEM.ReadFEString(File));

		if (Object->Layers[i]->GetType() == LAYER_TYPE::INTERPOLATION)
		{
			Object->Layers[i]->InterpolationData = new LayerInterpolationData();

			File.read(Buffer, 4);
			const int UsedLayersCount = *(int*)Buffer;
			Object->Layers[i]->InterpolationData->UsedLayerIDs.resize(UsedLayersCount);
			for (size_t j = 0; j < UsedLayersCount; j++)
				Object->Layers[i]->InterpolationData->UsedLayerIDs[j] = FILE_SYSTEM.ReadFEString(File);

			File.read(Buffer, 4);
			const float InterpolationFactor = *(float*)Buffer;
			Object->Layers[i]->InterpolationData->SetInterpolationFactor(InterpolationFactor);

			File.read(Buffer, 4);
			const int MinMaxInterpolationEnabled = *(int*)Buffer;
			Object->Layers[i]->InterpolationData->SetMinMaxInterpolationEnabled(MinMaxInterpolationEnabled != 0);

			File.read(Buffer, 4);
			const int ElementPerLayerCount = *(int*)Buffer;
			Object->Layers[i]->InterpolationData->RawData.resize(Object->Layers[i]->InterpolationData->UsedLayerIDs.size());
			for (size_t j = 0; j < Object->Layers[i]->InterpolationData->UsedLayerIDs.size(); j++)
			{
				Object->Layers[i]->InterpolationData->RawData[j].resize(ElementPerLayerCount);
				File.read((char*)Object->Layers[i]->InterpolationData->RawData[j].data(), ElementPerLayerCount * 4);
			}
		}
		else
		{
			// ElementsToData
			File.read(Buffer, 4);
			const int ElementsToDataCout = *(int*)Buffer;
			std::vector<float> TrianglesData;
			Object->Layers[i]->ElementsToData.resize(ElementsToDataCout);
			File.read((char*)Object->Layers[i]->ElementsToData.data(), ElementsToDataCout * 4);
		}

		// Debug info.
		File.read(Buffer, 4);
		const int DebugInfoPresent = *(int*)Buffer;
		if (DebugInfoPresent)
		{
			Object->Layers[i]->DebugInfo = new DataLayerDebugInfo();
			Object->Layers[i]->DebugInfo->FromFile(File);
		}
	}

	delete[] Buffer;
}

bool AnalysisObjectManager::LoadRUGFile_V0_9_1(std::string FilePath)
{
	std::fstream File;
	File.open(FilePath, std::ios::in | std::ios::binary);
	if (!File.is_open())
	{
		LOG.Add(std::string("Can't open file: ") + FilePath + " in function LoadRUGFile_V0_9_1.");
		return false;
	}

	File.seekg(0, std::ios::end);
	const std::streamsize FileSize = File.tellg();
	File.seekg(0, std::ios::beg);
	if (FileSize <= 0)
	{
		LOG.Add(std::string("Can't get file size: ") + FilePath + " in function LoadRUGFile_V0_9_1.");
		return false;
	}

	char* Buffer32 = new char[4];
	char* Buffer64 = new char[8];
	long long ArraySize = 0;

	File.read(Buffer32, 4);
	const float Version = *(float*)Buffer32;
	if (glm::epsilonNotEqual(Version, APPLICATION_VERSION_FLOAT, 0.0001f))
	{
		if (Version - 0.91f > 0.0001f)
		{
			LOG.Add(std::string("Can't load file: ") + FilePath + " in function LoadRUGFile_V0_9_1. File was created in unsupported version of the application!");
			return false;
		}
	}

	File.read(Buffer64, 8);
	const size_t AnalysisObjectCount = *(size_t*)Buffer64;

	for (size_t i = 0; i < AnalysisObjectCount; i++)
	{
		AnalysisObject* NewAnalysisObject = new AnalysisObject();

		File.read(Buffer32, 4);
		const int ObjectIDSize = *(int*)Buffer32;
		char* ObjectIDBuffer = new char[ObjectIDSize];
		File.read(ObjectIDBuffer, ObjectIDSize);
		const std::string ObjectID = std::string(ObjectIDBuffer);

		// FE_FIX_ME: It is not good solution, it would not delete all previously loaded objects.
		// Better solution would to have header in the file with all object IDs and check it before loading.
		if (AnalysisObjects.find(ObjectID) != AnalysisObjects.end())
		{
			LOG.Add(std::string("Can't load file: ") + FilePath + " in function LoadRUGFile_V0_9_1. Object with ID " + ObjectID + " already exists in the scene!");
			delete NewAnalysisObject;
			delete[] ObjectIDBuffer;
			delete[] Buffer32;
			delete[] Buffer64;
			File.close();
			return false;
		}
		NewAnalysisObject->ID = ObjectID;
		delete[] ObjectIDBuffer;

		File.read(Buffer32, 4);
		const int ObjectNameSize = *(int*)Buffer32;
		char* ObjectNameBuffer = new char[ObjectNameSize];
		File.read(ObjectNameBuffer, ObjectNameSize);
		const std::string ObjectName = std::string(ObjectNameBuffer);
		NewAnalysisObject->Name = ObjectName;
		delete[] ObjectNameBuffer;

		File.read(Buffer32, 4);
		const DATA_SOURCE_TYPE ObjectType = *(DATA_SOURCE_TYPE*)Buffer32;
		NewAnalysisObject->Type = ObjectType;
		File.read(Buffer32, 4);

		const int FilePathSize = *(int*)Buffer32;
		char* FilePathBuffer = new char[FilePathSize];
		File.read(FilePathBuffer, FilePathSize);
		const std::string ObjectFilePath = std::string(FilePathBuffer);
		NewAnalysisObject->FilePath = ObjectFilePath;
		delete[] FilePathBuffer;

		File.read(Buffer32, 4);
		const int VisibleInScene = *(int*)Buffer32;

		switch (ObjectType)
		{
			case DATA_SOURCE_TYPE::MESH:
			{
				LoadMeshDataFromRUGFile(File, NewAnalysisObject);
				break;
			}

			case DATA_SOURCE_TYPE::POINT_CLOUD:
			{
				LoadPointCloudDataFromRUGFile(File, NewAnalysisObject);
				break;
			}
			
			default:
			{
				LOG.Add(std::string("Can't load file: ") + FilePath + " in function LoadRUGFile_V0_9_1. Unknown data source type!");
				delete NewAnalysisObject;
				continue;
				break;
			}
		}

		LoadLayersDataFromRUGFile(File, NewAnalysisObject);
		AnalysisObjects[NewAnalysisObject->GetID()] = NewAnalysisObject;
		OnAnalysisObjectLoad(NewAnalysisObject);
		LoadAnnotationsDataFromRUGFile(File, NewAnalysisObject);

		FEEntity* CurrentEntity = NewAnalysisObject->GetEntity();
		if (CurrentEntity != nullptr)
			CurrentEntity->SetVisible(VisibleInScene);
	}

	delete[] Buffer32;
	delete[] Buffer64;
	File.close();

	return true;
}

void AnalysisObjectManager::BeforeRender(FEEntity* CurrentEntity)
{
	auto ObjectsMapIterator = ANALYSIS_OBJECT_MANAGER.AnalysisObjects.begin();
	while (ObjectsMapIterator != ANALYSIS_OBJECT_MANAGER.AnalysisObjects.end())
	{
		AnalysisObject* CurrentObject = ObjectsMapIterator->second;
		if (CurrentObject == nullptr || CurrentObject->GetEntity() == nullptr || CurrentObject->GetEntity() != CurrentEntity)
		{
			ObjectsMapIterator++;
			continue;
		}

		if (CurrentObject->GetType() == DATA_SOURCE_TYPE::MESH)
		{
			FEMesh* ActiveMesh = static_cast<FEMesh*>(CurrentObject->GetEngineResource());
			if (ActiveMesh != nullptr)
			{
				if (SETTINGS_WINDOW.GetWireFrameMode())
				{
					CurrentEntity->GetComponent<FEGameModelComponent>().SetWireframeMode(true);
				}
				else
				{
					CurrentEntity->GetComponent<FEGameModelComponent>().SetWireframeMode(false);
				}

				ANALYSIS_OBJECT_MANAGER.UpdateMeshUniforms(CurrentObject);

				FE_GL_ERROR(glBindVertexArray(ActiveMesh->GetVaoID()));

				if (ActiveMesh->GetColorCount() > 0) FE_GL_ERROR(glEnableVertexAttribArray(1));
				MeshAnalysisData* CurrentMeshAnalysisData = CurrentObject->GetMeshAnalysisData();
				if (CurrentMeshAnalysisData->GetFirstLayerBufferID() > 0) FE_GL_ERROR(glEnableVertexAttribArray(7));
				if (CurrentMeshAnalysisData->GetSecondLayerBufferID() > 0) FE_GL_ERROR(glEnableVertexAttribArray(8));

				DataLayer* ActiveLayer = CurrentObject->GetActiveLayer();
				if (ActiveLayer != nullptr && ActiveLayer->GetType() == LAYER_TYPE::INTERPOLATION)
				{
					LayerInterpolationData* CurrentInterpolationData = ActiveLayer->GetInterpolationData();
					if (CurrentInterpolationData != nullptr)
					{
						std::vector<int> AttribArrayToEnable = GetVertexAttributeIndexes(static_cast<int>(CurrentInterpolationData->GetLayerCount()));
						for (size_t i = 0; i < AttribArrayToEnable.size(); i++)
							FE_GL_ERROR(glEnableVertexAttribArray(AttribArrayToEnable[i]));
					}
				}
			}
		}
		else if (CurrentObject->GetType() == DATA_SOURCE_TYPE::POINT_CLOUD)
		{
			FEPointCloud* PointCloud = static_cast<FEPointCloud*>(CurrentObject->GetEngineResource());
			DataLayer* ActiveLayer = CurrentObject->GetActiveLayer();
			bool bLayerActive = ActiveLayer != nullptr && !ActiveLayer->ValuesComputeShaderBuffers.empty();

			AnnotationData* CurrentAnnotationData = ANNOTATION_MANAGER.GetAnnotationDataByAnalysisObjectID(CurrentObject->GetID());
			bool bAnnotationDataReady = CurrentAnnotationData != nullptr && !CurrentAnnotationData->AnnotationIDComputeShaderBuffers.empty();
			bool bAnnotationVisualizationActive = false;
			if (bAnnotationDataReady)
			{
				FEEntity* AnnotationEntity = CurrentAnnotationData->GetEntity();
				if (AnnotationEntity != nullptr && AnnotationEntity->IsVisible())
					bAnnotationVisualizationActive = true;
			}

			if (!bLayerActive && !bAnnotationDataReady)
			{
				ObjectsMapIterator++;
				continue;
			}

			ANALYSIS_OBJECT_MANAGER.PointCloudRecoloringShader->Start();

			if (bLayerActive)
			{
				ANALYSIS_OBJECT_MANAGER.PointCloudRecoloringShader->UpdateUniformData("SelectedRangeMin", ActiveLayer->GetSelectedRangeMin());
				ANALYSIS_OBJECT_MANAGER.PointCloudRecoloringShader->UpdateUniformData("SelectedRangeMax", ActiveLayer->GetSelectedRangeMax());

				ANALYSIS_OBJECT_MANAGER.PointCloudRecoloringShader->UpdateUniformData("LayerMinValue", ActiveLayer->MinVisible);
				ANALYSIS_OBJECT_MANAGER.PointCloudRecoloringShader->UpdateUniformData("LayerMaxValue", ActiveLayer->MaxVisible);

				ANALYSIS_OBJECT_MANAGER.PointCloudRecoloringShader->UpdateUniformData("LayerAbsoluteMin", float(ActiveLayer->GetMin()));
				ANALYSIS_OBJECT_MANAGER.PointCloudRecoloringShader->UpdateUniformData("LayerAbsoluteMax", float(ActiveLayer->GetMax()));
			}
			else
			{
				ANALYSIS_OBJECT_MANAGER.PointCloudRecoloringShader->UpdateUniformData("SelectedRangeMin", 0.0f);
				ANALYSIS_OBJECT_MANAGER.PointCloudRecoloringShader->UpdateUniformData("SelectedRangeMax", 0.0f);
			}

			ANALYSIS_OBJECT_MANAGER.PointCloudRecoloringShader->UpdateUniformData("LayerActive", bLayerActive ? 1 : 0);
			ANALYSIS_OBJECT_MANAGER.PointCloudRecoloringShader->UpdateUniformData("AnnotationVisualizationActive", bAnnotationVisualizationActive ? 1 : 0);

			ANALYSIS_OBJECT_MANAGER.PointCloudRecoloringShader->LoadUniformsDataToGPU();

			FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, ANALYSIS_OBJECT_MANAGER.TurboColorBuffer));
			FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ANALYSIS_OBJECT_MANAGER.TurboColorBuffer));

			if (bAnnotationDataReady)
				FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, CurrentAnnotationData->AnnotationSSBO));

			// If we have more points than the maximum points per buffer, we will run the compute shader multiple times.
			if (PointCloud->GetPointCount() > FEPointCloud::MaxPointsPerBuffer)
			{
				std::vector<GLuint> BufferIDs;
				PointCloud->GetComputeShaderBuffers(BufferIDs);

				size_t BufferIndex = 0;
				for (size_t i = 0; i < PointCloud->GetPointCount(); i += FEPointCloud::MaxPointsPerBuffer)
				{
					if (BufferIndex >= BufferIDs.size())
					{
						LOG.Add("AnalysisObjectManager::BeforeRender: BufferIndex is out of range", "ANALYSIS_OBJECT_MANAGER", FE_LOG_ERROR);
					}
					else
					{
						FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, BufferIDs[BufferIndex]));
						FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, BufferIDs[BufferIndex]));

						if (bLayerActive && BufferIndex < ActiveLayer->ValuesComputeShaderBuffers.size())
						{
							FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, ActiveLayer->ValuesComputeShaderBuffers[BufferIndex]));
							FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ActiveLayer->ValuesComputeShaderBuffers[BufferIndex]));
						}

						if (bAnnotationDataReady && BufferIndex < CurrentAnnotationData->AnnotationIDComputeShaderBuffers.size())
						{
							FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, CurrentAnnotationData->AnnotationIDComputeShaderBuffers[BufferIndex]));
							FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, CurrentAnnotationData->OriginalColorComputeShaderBuffers[BufferIndex]));
						}
						BufferIndex++;

						// Calculate the number of points for the current buffer
						size_t NumberOfPoints = std::min(FEPointCloud::MaxPointsPerBuffer, PointCloud->GetPointCount() - i);
						ANALYSIS_OBJECT_MANAGER.PointCloudRecoloringShader->Dispatch(static_cast<GLuint>((NumberOfPoints / 1024) + 1), 1, 1);
						FE_GL_ERROR(glMemoryBarrier(GL_ALL_BARRIER_BITS));
					}
				}
			}
			else
			{
				GLuint BufferID;
				PointCloud->GetComputeShaderBuffer(BufferID);

				FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, BufferID));
				FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, BufferID));

				if (bLayerActive)
				{
					FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, ActiveLayer->ValuesComputeShaderBuffers[0]));
					FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ActiveLayer->ValuesComputeShaderBuffers[0]));
				}

				if (bAnnotationDataReady)
				{
					FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, CurrentAnnotationData->AnnotationIDComputeShaderBuffers[0]));
					FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, CurrentAnnotationData->OriginalColorComputeShaderBuffers[0]));
				}

				FE_GL_ERROR(ANALYSIS_OBJECT_MANAGER.PointCloudRecoloringShader->Dispatch(static_cast<GLuint>((PointCloud->GetPointCount()/*FEPointCloud::MaxPointsPerBuffer*/ / 1024) + 1), 1, 1));
				FE_GL_ERROR(glMemoryBarrier(GL_ALL_BARRIER_BITS));
			}
		}

		ObjectsMapIterator++;
	}
}

void AnalysisObjectManager::AddOnActiveObjectChangeCallback(std::function<void(AnalysisObject*)> Callback)
{
	ClientOnActiveObjectChangeCallbacks.push_back(Callback);
}

FEAABB AnalysisObjectManager::GetAllObjectsAABB()
{
	FEAABB Result;
	bool bFirst = true;
	auto ObjectsMapIterator = ANALYSIS_OBJECT_MANAGER.AnalysisObjects.begin();
	while (ObjectsMapIterator != ANALYSIS_OBJECT_MANAGER.AnalysisObjects.end())
	{
		AnalysisObject* CurrentObject = ObjectsMapIterator->second;
		if (CurrentObject != nullptr)
		{
			if (bFirst)
			{
				Result = CurrentObject->AnalysisData->GetAABB();
				bFirst = false;
			}
			else
			{
				Result = Result.Merge(CurrentObject->AnalysisData->GetAABB());
			}
		}

		ObjectsMapIterator++;
	}

	return Result;
}

double AnalysisObjectManager::GetAllMeshObjectsTotalArea()
{
	double Result = 0.0;
	auto ObjectsMapIterator = ANALYSIS_OBJECT_MANAGER.AnalysisObjects.begin();
	while (ObjectsMapIterator != ANALYSIS_OBJECT_MANAGER.AnalysisObjects.end())
	{
		AnalysisObject* CurrentObject = ObjectsMapIterator->second;
		if (CurrentObject != nullptr && CurrentObject->GetType() == DATA_SOURCE_TYPE::MESH)
		{
			MeshAnalysisData* CurrentMeshAnalysisData = CurrentObject->GetMeshAnalysisData();
			if (CurrentMeshAnalysisData != nullptr)
				Result += CurrentMeshAnalysisData->GetTotalArea();
		}

		ObjectsMapIterator++;
	}

	return Result;
}

glm::vec3 AnalysisObjectManager::GetAllMeshObjectsAverageNormal()
{
	glm::vec3 Result = glm::vec3(0.0f);
	double TotalArea = GetAllMeshObjectsTotalArea();
	auto ObjectsMapIterator = ANALYSIS_OBJECT_MANAGER.AnalysisObjects.begin();
	while (ObjectsMapIterator != ANALYSIS_OBJECT_MANAGER.AnalysisObjects.end())
	{
		AnalysisObject* CurrentObject = ObjectsMapIterator->second;
		if (CurrentObject != nullptr && CurrentObject->GetType() == DATA_SOURCE_TYPE::MESH)
		{
			MeshAnalysisData* CurrentMeshAnalysisData = CurrentObject->GetMeshAnalysisData();
			if (CurrentMeshAnalysisData != nullptr)
			{
				glm::vec3 CurrentNormal = CurrentMeshAnalysisData->GetAverageNormal();
				double AreaFactor = CurrentMeshAnalysisData->GetTotalArea() / TotalArea;
				Result += CurrentNormal * float(AreaFactor);
			}
		}
		ObjectsMapIterator++;
	}

	return Result;
}

void AnalysisObjectManager::AddOnObjectDeleteCallback(std::function<void(AnalysisObject*)> Callback)
{
	ClientOnObjectDeleteCallbacks.push_back(Callback);
}

void AnalysisObjectManager::ClearAll()
{
	SetActiveAnalysisObject("");

	std::vector<std::string> ObjectsIDs = GetAnalysisObjectsIDList();
	for (size_t i = 0; i < ObjectsIDs.size(); i++)
		DeleteAnalysisObject(ObjectsIDs[i]);

	AnalysisObjects.clear();
}