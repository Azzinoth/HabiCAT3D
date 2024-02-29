#include "ObjLoader.h"
using namespace FocalEngine;

ObjLoader* ObjLoader::Instance = nullptr;

ObjLoader::ObjLoader()
{
	
}

ObjLoader::~ObjLoader()
{

}

void ObjLoader::ReadLine(std::stringstream& LineStream, RawOBJData* Data)
{
	std::string STemp;

	LineStream >> STemp;
	// To lower case
	std::transform(STemp.begin(), STemp.end(), STemp.begin(), [](const unsigned char C) { return std::tolower(C); });

	// if it is comment or object declaration or other not relevant info
	if (STemp[0] == '#' || STemp[0] == 'o')
	{
		// get to the next line
		return;
	}

	// if this line contains vertex coordinates
	if (STemp[0] == 'v' && STemp.size() == 1)
	{
		glm::dvec3 NewVec;
		for (int i = 0; i <= 2; i++)
		{
			LineStream >> STemp;
			NewVec[i] = std::stod(STemp);
		}

		Data->RawVertexCoordinates.push_back(NewVec);

		// File could contain RGB.
		if (!LineStream.eof())
		{
			for (int i = 0; i <= 2; i++)
			{
				LineStream >> STemp;
				if (STemp == "")
					break;

				NewVec[i] = std::stof(STemp);
				bHaveColors = true;
			}

			Data->RawVertexColors.push_back(NewVec);
		}
	}
	// if this line contains vertex texture coordinates
	else if (STemp[0] == 'v' && STemp.size() == 2 && STemp[1] == 't')
	{
		bHaveTextureCoord = true;

		glm::vec2 NewVec;
		for (int i = 0; i <= 1; i++)
		{
			LineStream >> STemp;
			NewVec[i] = std::stof(STemp);
		}

		Data->RawTextureCoordinates.push_back(NewVec);
	}
	// if this line contains vertex texture coordinates
	else if (STemp[0] == 'v' && STemp.size() == 2 && STemp[1] == 'n')
	{
		bHaveNormalCoord = true;

		glm::vec3 NewVec;
		for (int i = 0; i <= 2; i++)
		{
			LineStream >> STemp;
			NewVec[i] = std::stof(STemp);
		}
		
		glm::vec3 NormilizedVector = glm::normalize(NewVec);

		if (isnan(NormilizedVector.x) || isnan(NormilizedVector.y) || isnan(NormilizedVector.z))
			NormilizedVector = glm::vec3(0.0f);

		Data->RawNormalCoordinates.push_back(NormilizedVector);
	}
	// if this line contains indices
	else if (STemp[0] == 'f' && STemp.size() == 1)
	{
		for (int i = 0; i <= 2; i++)
		{
			LineStream >> STemp;

			std::stringstream TempLineStrem;
			TempLineStrem << STemp;

			int iterations = 0;
			while (std::getline(TempLineStrem, STemp, '/'))
			{
				if (!STemp.empty())
				{
					Data->RawIndices.push_back(std::stoi(STemp));
				}
				else
				{
					// Texture coordinates are optional.
					if (iterations == 1)
					{
						// It is not proper fix!
						Data->RawIndices.push_back(1);
					}
					else
					{
						LOG.Add(std::string("Texture coordinates was absent in face description in function FEObjLoader::readFile."), "FE_LOG_LOADING", FE_LOG_ERROR);
					}
				}

				if (!Data->MaterialRecords.empty())
				{
					if (iterations == 0)
					{
						if (Data->MaterialRecords.back().MinVertexIndex > static_cast<unsigned int>(Data->RawIndices.back()))
							Data->MaterialRecords.back().MinVertexIndex = static_cast<unsigned int>(Data->RawIndices.back());

						if (Data->MaterialRecords.back().MaxVertexIndex < static_cast<unsigned int>(Data->RawIndices.back()))
							Data->MaterialRecords.back().MaxVertexIndex = static_cast<unsigned int>(Data->RawIndices.back());
					}
					else if (iterations == 1)
					{
						if (Data->MaterialRecords.back().MinTextureIndex > static_cast<unsigned int>(Data->RawIndices.back()))
							Data->MaterialRecords.back().MinTextureIndex = static_cast<unsigned int>(Data->RawIndices.back());

						if (Data->MaterialRecords.back().MaxTextureIndex < static_cast<unsigned int>(Data->RawIndices.back()))
							Data->MaterialRecords.back().MaxTextureIndex = static_cast<unsigned int>(Data->RawIndices.back());
					}
					else if (iterations == 2)
					{
						if (Data->MaterialRecords.back().MinNormalIndex > static_cast<unsigned int>(Data->RawIndices.back()))
							Data->MaterialRecords.back().MinNormalIndex = static_cast<unsigned int>(Data->RawIndices.back());

						if (Data->MaterialRecords.back().MaxNormalIndex < static_cast<unsigned int>(Data->RawIndices.back()))
							Data->MaterialRecords.back().MaxNormalIndex = static_cast<unsigned int>(Data->RawIndices.back());
					}

					Data->MaterialRecords.back().FaceCount++;
				}
				
				iterations++;
			}
		}
	}
	// if this line contains new material declaration
	else if (STemp.find("usemtl") != std::string::npos)
	{
		Data->MaterialRecords.push_back(materialRecord());
		if (Data->MaterialRecords.size() > 1)
			Data->MaterialRecords.back().FacesSeenBefore = Data->MaterialRecords[Data->MaterialRecords.size() - 2].FacesSeenBefore + Data->MaterialRecords[Data->MaterialRecords.size() - 2].FaceCount;
		
		LineStream >> Data->MaterialRecords.back().Name;
	}
	// file with materials data
	else if (STemp.find("mtllib") != std::string::npos)
	{
		LineStream >> MaterialFileName;
	}
}

void ObjLoader::ReadFile(const char* FileName)
{
	bHaveColors = false;
	bHaveTextureCoord = false;
	bHaveNormalCoord = false;
	CurrentFilePath = FileName;
	MaterialFileName = "";
	CurrentMaterialObject = nullptr;
	for (size_t i = 0; i < LoadedObjects.size(); i++)
	{
		delete LoadedObjects[i];
	}
	LoadedObjects.clear();
	LoadedObjects.push_back(new RawOBJData());

	if (FileName == nullptr)
	{
		LOG.Add(std::string("No file name in function FEObjLoader::readFile."), "FE_LOG_LOADING", FE_LOG_ERROR);
		return;
	}

	std::ifstream File(FileName, std::ios::binary);
	const auto begin = File.tellg();
	File.seekg(0, std::ios::end);
	const auto end = File.tellg();
	const auto fsize = static_cast<size_t>(end - begin);

	File.seekg(0, 0);

	std::string CurrentLine;
	for (size_t i = 0; i < fsize; i++)
	{
		char NewChar;
		File.read(&NewChar, 1);
		CurrentLine += NewChar;
			
		if (NewChar == '\n')
		{
			CurrentLine.erase(CurrentLine.end() - 1, CurrentLine.end());
			ReadLine(std::stringstream(CurrentLine), LoadedObjects.back());

			CurrentLine = "";
		}
	}

	if (!bForceOneMesh)
	{
		// Each material should represented by different FERawOBJData
		std::vector<RawOBJData*> ObjectsPerMaterialList;
		for (size_t i = 0; i < LoadedObjects.size(); i++)
		{
			for (size_t j = 0; j < LoadedObjects[i]->MaterialRecords.size(); j++)
			{
				RawOBJData* TempObject = new RawOBJData();
				TempObject->MaterialRecords.push_back(materialRecord(LoadedObjects[i]->MaterialRecords[j]));

				size_t StartIndex = LoadedObjects[i]->MaterialRecords[j].MinVertexIndex - 1;
				size_t EndIndex = LoadedObjects[i]->MaterialRecords[j].MaxVertexIndex;
				for (size_t k = StartIndex; k < EndIndex; k++)
				{
					TempObject->RawVertexCoordinates.push_back(LoadedObjects[i]->RawVertexCoordinates[k]);
				}

				StartIndex = LoadedObjects[i]->MaterialRecords[j].MinTextureIndex - 1;
				EndIndex = LoadedObjects[i]->MaterialRecords[j].MaxTextureIndex;
				for (size_t k = StartIndex; k < EndIndex; k++)
				{
					TempObject->RawTextureCoordinates.push_back(LoadedObjects[i]->RawTextureCoordinates[k]);
				}

				StartIndex = LoadedObjects[i]->MaterialRecords[j].MinNormalIndex - 1;
				EndIndex = LoadedObjects[i]->MaterialRecords[j].MaxNormalIndex;
				for (size_t k = StartIndex; k < EndIndex; k++)
				{
					TempObject->RawNormalCoordinates.push_back(LoadedObjects[i]->RawNormalCoordinates[k]);
				}

				StartIndex = LoadedObjects[i]->MaterialRecords[j].FacesSeenBefore;
				EndIndex = StartIndex + LoadedObjects[i]->MaterialRecords[j].FaceCount;
				for (size_t k = StartIndex; k < EndIndex; k += 3)
				{
					TempObject->RawIndices.push_back(LoadedObjects[i]->RawIndices[k] - LoadedObjects[i]->MaterialRecords[j].MinVertexIndex + 1);
					TempObject->RawIndices.push_back(LoadedObjects[i]->RawIndices[k + 1] - LoadedObjects[i]->MaterialRecords[j].MinTextureIndex + 1);
					TempObject->RawIndices.push_back(LoadedObjects[i]->RawIndices[k + 2] - LoadedObjects[i]->MaterialRecords[j].MinNormalIndex + 1);
				}

				ObjectsPerMaterialList.push_back(TempObject);
			}

			if (LoadedObjects[i]->MaterialRecords.size() == 0)
				ObjectsPerMaterialList.push_back(new RawOBJData(*LoadedObjects[i]));
		}

		for (size_t i = 0; i < LoadedObjects.size(); i++)
		{
			delete LoadedObjects[i];
		}
		LoadedObjects.clear();
		LoadedObjects = ObjectsPerMaterialList;
	}

	ReadMaterialFile(FileName);

	for (size_t i = 0; i < LoadedObjects.size(); i++)
	{
		ProcessRawData(LoadedObjects[i]);
	}
}

glm::vec3 ObjLoader::CalculateNormal(glm::dvec3 V0, glm::dvec3 V1, glm::dvec3 V2)
{
	glm::dvec3 Edge_0 = V2 - V1;
	glm::dvec3 Edge_1 = V2 - V0;

	glm::dvec3 Normal = glm::normalize(glm::cross(Edge_1, Edge_0));

	if (isnan(Normal.x) || isnan(Normal.y) || isnan(Normal.z))
		Normal = glm::dvec3();

	return Normal;
}

void ObjLoader::CalculateNormals(RawOBJData* Data)
{
	int IndexShift = 3;
	// We assume that there were no normals info read.
	for (size_t i = 0; i < Data->FInd.size() - 1; i+=3)
	{
		glm::dvec3 V0 = { Data->FVerC[Data->FInd[i] * IndexShift], Data->FVerC[Data->FInd[i] * IndexShift + 1], Data->FVerC[Data->FInd[i] * IndexShift + 2] };
		glm::dvec3 V1 = { Data->FVerC[Data->FInd[i + 1] * IndexShift], Data->FVerC[Data->FInd[i + 1] * IndexShift + 1], Data->FVerC[Data->FInd[i + 1] * IndexShift + 2] };
		glm::dvec3 V2 = { Data->FVerC[Data->FInd[i + 2] * IndexShift], Data->FVerC[Data->FInd[i + 2] * IndexShift + 1], Data->FVerC[Data->FInd[i + 2] * IndexShift + 2] };

		glm::vec3 Normal = CalculateNormal(V0, V1, V2);

		Data->FNorC[Data->FInd[i] * IndexShift] = Normal.x;
		Data->FNorC[Data->FInd[i] * IndexShift + 1] = Normal.y;
		Data->FNorC[Data->FInd[i] * IndexShift + 2] = Normal.z;

		Data->FNorC[Data->FInd[i + 1] * IndexShift] = Normal.x;
		Data->FNorC[Data->FInd[i + 1] * IndexShift + 1] = Normal.y;
		Data->FNorC[Data->FInd[i + 1] * IndexShift + 2] = Normal.z;

		Data->FNorC[Data->FInd[i + 2] * IndexShift] = Normal.x;
		Data->FNorC[Data->FInd[i + 2] * IndexShift + 1] = Normal.y;
		Data->FNorC[Data->FInd[i + 2] * IndexShift + 2] = Normal.z;
	}
}

glm::vec3 ObjLoader::CalculateTangent(glm::vec3 V0, glm::vec3 V1, glm::vec3 V2, std::vector<glm::vec2>&& Textures)
{
	const glm::vec3 Q1 = V1 - V0;
	const glm::vec3 Q2 = V2 - V0;
	const glm::vec2 UV0 = Textures[0];
	const glm::vec2 UV1 = Textures[1];
	const glm::vec2 UV2 = Textures[2];

	const float T1 = UV1.y - UV0.y;
	const float T2 = UV2.y - UV0.y;

	const glm::vec3 Tangent = T1 * Q2 - T2 * Q1;

	return Tangent;
}

void ObjLoader::CalculateTangents(RawOBJData* Data)
{
	for (size_t i = 0; i < Data->FInd.size() - 1; i += 3)
	{
		const glm::vec3 V0 = { Data->FVerC[Data->FInd[i] * 3], Data->FVerC[Data->FInd[i] * 3 + 1], Data->FVerC[Data->FInd[i] * 3 + 2] };
		const glm::vec3 V1 = { Data->FVerC[Data->FInd[i + 1] * 3], Data->FVerC[Data->FInd[i + 1] * 3 + 1], Data->FVerC[Data->FInd[i + 1] * 3 + 2] };
		const glm::vec3 V2 = { Data->FVerC[Data->FInd[i + 2] * 3], Data->FVerC[Data->FInd[i + 2] * 3 + 1], Data->FVerC[Data->FInd[i + 2] * 3 + 2] };

		glm::vec2 T0 = { Data->FTexC[Data->FInd[i] * 2], Data->FTexC[Data->FInd[i] * 2 + 1] };
		glm::vec2 T1 = { Data->FTexC[Data->FInd[i + 1] * 2], Data->FTexC[Data->FInd[i + 1] * 2 + 1] };
		glm::vec2 T2 = { Data->FTexC[Data->FInd[i + 2] * 2], Data->FTexC[Data->FInd[i + 2] * 2 + 1] };

		glm::vec3 Tangent = CalculateTangent(V0, V1, V2, { T0, T1, T2 });
		// To eliminate NaN values after normalization.
		// I encounter this problem if triangle has same texture coordinates.
		if (Tangent.x != 0 || Tangent.y != 0 || Tangent.z != 0)
		{
			Tangent = glm::normalize(Tangent);
		}
		else
		{
			glm::vec3 Normal = { Data->FNorC[Data->FInd[i] * 3], Data->FNorC[Data->FInd[i] * 3 + 1], Data->FNorC[Data->FInd[i] * 3 + 2] };
			glm::vec3 TangentOne = glm::cross(Normal, glm::vec3(0.0f, 0.0f, 1.0f));
			glm::vec3 TangentTwo = glm::cross(Normal, glm::vec3(0.0f, 1.0f, 0.0f));
			// Choosing candidate with bigger length/magnitude.
			// Length/magnitude of cross product depend on sine of angle between vectors
			// and sine of 90 degrees is 1.0(max value), so basically we are choosing cross product in which vectors was closer to perpendicular(assuming both vectors are unit vectors).
			Tangent = glm::length(TangentOne) > glm::length(TangentTwo) ? TangentOne : TangentTwo;
			Tangent = glm::normalize(Tangent);
		}	

		Data->FTanC[Data->FInd[i] * 3] = Tangent.x;
		Data->FTanC[Data->FInd[i] * 3 + 1] = Tangent.y;
		Data->FTanC[Data->FInd[i] * 3 + 2] = Tangent.z;

		Data->FTanC[Data->FInd[i + 1] * 3] = Tangent.x;
		Data->FTanC[Data->FInd[i + 1] * 3 + 1] = Tangent.y;
		Data->FTanC[Data->FInd[i + 1] * 3 + 2] = Tangent.z;

		Data->FTanC[Data->FInd[i + 2] * 3] = Tangent.x;
		Data->FTanC[Data->FInd[i + 2] * 3 + 1] = Tangent.y;
		Data->FTanC[Data->FInd[i + 2] * 3 + 2] = Tangent.z;
	}
}

void ObjLoader::NormalizeVertexPositions(RawOBJData* Data)
{
	double MinX = DBL_MAX;
	double MaxX = -DBL_MAX;
	double MinY = DBL_MAX;
	double MaxY = -DBL_MAX;
	double MinZ = DBL_MAX;
	double MaxZ = -DBL_MAX;

	for (size_t i = 0; i < Data->RawVertexCoordinates.size(); i++)
	{
		if (MinX > Data->RawVertexCoordinates[i].x)
			MinX = Data->RawVertexCoordinates[i].x;

		if (MaxX < Data->RawVertexCoordinates[i].x)
			MaxX = Data->RawVertexCoordinates[i].x;

		if (MinY > Data->RawVertexCoordinates[i].y)
			MinY = Data->RawVertexCoordinates[i].y;

		if (MaxY < Data->RawVertexCoordinates[i].y)
			MaxY = Data->RawVertexCoordinates[i].y;

		if (MinZ > Data->RawVertexCoordinates[i].z)
			MinZ = Data->RawVertexCoordinates[i].z;

		if (MaxZ < Data->RawVertexCoordinates[i].z)
			MaxZ = Data->RawVertexCoordinates[i].z;
	}

	double RangeX = abs(MaxX - MinX);
	double RangeY = abs(MaxY - MinY);
	double RangeZ = abs(MaxZ - MinZ);

	double MinRange = std::min(std::min(RangeX, RangeY), RangeZ);
	double ScaleFactor = 1.0;

	if (MinRange < 1.0)
	{
		ScaleFactor = 1.0 / MinRange;
	}

	for (size_t i = 0; i < Data->RawVertexCoordinates.size(); i++)
	{
		Data->RawVertexCoordinates[i].x -= MinX;
		Data->RawVertexCoordinates[i].y -= MinY;
		Data->RawVertexCoordinates[i].z -= MinZ;

		Data->RawVertexCoordinates[i] *= ScaleFactor;
	}
}

void ObjLoader::ProcessRawData(RawOBJData* Data)
{
	NormalizeVertexPositions(Data);

	if (bHaveTextureCoord && bHaveNormalCoord)
	{
#ifdef DOUBLE_VERTEX_ON_SEAMS
		std::vector<ObjLoader::VertexThatNeedDoubling> VertexList;
		std::unordered_map<int, int> IndexesMap;

		for (size_t i = 0; i < Data->RawIndices.size(); i += 3)
		{
			IndexesMap[Data->RawIndices[i]] = static_cast<int>(i);
		}

		for (size_t i = 0; i < Data->RawIndices.size(); i += 3)
		{
			if (IndexesMap.find(Data->RawIndices[i]) != IndexesMap.end())
			{
				size_t j = IndexesMap.find(Data->RawIndices[i])->second;
				std::swap(i, j);

				const bool TexD = Data->RawIndices[i + 1] != Data->RawIndices[j + 1];
				const bool NormD = Data->RawIndices[i + 2] != Data->RawIndices[j + 2];
				if (Data->RawIndices[i] == Data->RawIndices[j] && (TexD || NormD))
				{
					// we do not need to add first appearance of vertex that we need to double
					ObjLoader::VertexThatNeedDoubling NewVertex = ObjLoader::VertexThatNeedDoubling(int(j), Data->RawIndices[j], Data->RawIndices[j + 1], Data->RawIndices[j + 2]);
					if (std::find(VertexList.begin(), VertexList.end(), NewVertex) == VertexList.end())
					{
						VertexList.push_back(NewVertex);
					}
				}

				std::swap(i, j);
			}
		}

		std::vector<std::pair<int, float>> DoubledVertexMatIndecies;
		for (auto& ver : VertexList)
		{
			if (ver.bWasDone) continue;

			Data->RawVertexCoordinates.push_back(Data->RawVertexCoordinates[ver.AcctualIndex - 1]);

			int NewVertexIndex = static_cast<int>(Data->RawVertexCoordinates.size());
			Data->RawIndices[ver.IndexInArray] = NewVertexIndex;
			ver.bWasDone = true;

			// preserve matIndex!
			for (size_t i = 0; i < Data->MaterialRecords.size(); i++)
			{
				if (ver.AcctualIndex >= (static_cast<int>(Data->MaterialRecords[i].MinVertexIndex + 1)) && ver.AcctualIndex <= (static_cast<int>(Data->MaterialRecords[i].MaxVertexIndex + 1)))
				{
					DoubledVertexMatIndecies.push_back(std::make_pair(NewVertexIndex, static_cast<float>(i)));
				}
			}

			for (auto& VerNext : VertexList)
			{
				if (VerNext.bWasDone) continue;
				if (ver.IndexInArray == VerNext.IndexInArray && (ver.TexIndex == VerNext.TexIndex || ver.NormIndex == VerNext.NormIndex))
				{
					Data->RawIndices[VerNext.IndexInArray] = NewVertexIndex;
					VerNext.bWasDone = true;
				}
			}
		}
#endif // DOUBLE_VERTEX_ON_SEAMS

		Data->FVerC.resize(Data->RawVertexCoordinates.size() * 3);
		Data->FTexC.resize(Data->RawVertexCoordinates.size() * 2);
		Data->FNorC.resize(Data->RawVertexCoordinates.size() * 3);
		Data->FTanC.resize(Data->RawVertexCoordinates.size() * 3);
		Data->FInd.resize(0);
		Data->MatIDs.resize(Data->RawVertexCoordinates.size());

		for (size_t i = 0; i < Data->RawIndices.size(); i += 3)
		{
			// faces index in OBJ file begins from 1 not 0.
			int VIndex = Data->RawIndices[i] - 1;
			const int TIndex = Data->RawIndices[i + 1] - 1;
			const int NIndex = Data->RawIndices[i + 2] - 1;

			const int ShiftInVerArr = VIndex * 3;
			const int ShiftInTexArr = VIndex * 2;

			Data->FVerC[ShiftInVerArr] = Data->RawVertexCoordinates[VIndex][0];
			Data->FVerC[ShiftInVerArr + 1] = Data->RawVertexCoordinates[VIndex][1];
			Data->FVerC[ShiftInVerArr + 2] = Data->RawVertexCoordinates[VIndex][2];

			// saving material ID in vertex attribute array
			for (size_t j = 0; j < Data->MaterialRecords.size(); j++)
			{
				if (VIndex >= static_cast<int>(Data->MaterialRecords[j].MinVertexIndex - 1) && VIndex <= static_cast<int>(Data->MaterialRecords[j].MaxVertexIndex - 1))
				{
					Data->MatIDs[VIndex] = static_cast<float>(j);
				}
			}

			Data->FTexC[ShiftInTexArr] = Data->RawTextureCoordinates[TIndex][0];
			Data->FTexC[ShiftInTexArr + 1] = 1 - Data->RawTextureCoordinates[TIndex][1];

			Data->FNorC[ShiftInVerArr] = Data->RawNormalCoordinates[NIndex][0];
			Data->FNorC[ShiftInVerArr + 1] = Data->RawNormalCoordinates[NIndex][1];
			Data->FNorC[ShiftInVerArr + 2] = Data->RawNormalCoordinates[NIndex][2];
	
			Data->FInd.push_back(VIndex);
		}

#ifdef DOUBLE_VERTEX_ON_SEAMS
		for (size_t j = 0; j < DoubledVertexMatIndecies.size(); j++)
		{
			Data->MatIDs[DoubledVertexMatIndecies[j].first - 1] = DoubledVertexMatIndecies[j].second;
		}
#endif // DOUBLE_VERTEX_ON_SEAMS

		CalculateTangents(Data);
	}
	else if (bHaveTextureCoord)
	{
		Data->FVerC.resize(Data->RawVertexCoordinates.size() * 3);
		Data->FTexC.resize(Data->RawVertexCoordinates.size() * 2);
		Data->FNorC.resize(Data->RawVertexCoordinates.size() * 3);
		Data->FTanC.resize(Data->RawVertexCoordinates.size() * 3);
		Data->FInd.resize(0);
		Data->MatIDs.resize(Data->RawVertexCoordinates.size());

		if (bHaveColors)
			Data->fColorsC.resize(Data->RawVertexCoordinates.size() * 3);

		for (size_t i = 0; i < Data->RawIndices.size(); i += 2)
		{
			// faces index in OBJ file begins from 1 not 0.
			int VIndex = Data->RawIndices[i] - 1;
			int TIndex = Data->RawIndices[i + 1] - 1;

			int ShiftInVerArr = VIndex * 3;
			int ShiftInTexArr = VIndex * 2;

			Data->FVerC[ShiftInVerArr] = Data->RawVertexCoordinates[VIndex][0];
			Data->FVerC[ShiftInVerArr + 1] = Data->RawVertexCoordinates[VIndex][1];
			Data->FVerC[ShiftInVerArr + 2] = Data->RawVertexCoordinates[VIndex][2];

			if (bHaveColors)
			{
				Data->fColorsC[ShiftInVerArr] = Data->RawVertexColors[VIndex][0];
				Data->fColorsC[ShiftInVerArr + 1] = Data->RawVertexColors[VIndex][1];
				Data->fColorsC[ShiftInVerArr + 2] = Data->RawVertexColors[VIndex][2];
			}

			// saving material ID in vertex attribute array
			for (size_t i = 0; i < Data->MaterialRecords.size(); i++)
			{
				if (VIndex >= int(Data->MaterialRecords[i].MinVertexIndex - 1) && VIndex <= int(Data->MaterialRecords[i].MaxVertexIndex - 1))
				{
					Data->MatIDs[VIndex] = float(i);
				}
			}

			Data->FTexC[ShiftInTexArr] = Data->RawTextureCoordinates[TIndex][0];
			Data->FTexC[ShiftInTexArr + 1] = 1 - Data->RawTextureCoordinates[TIndex][1];

			Data->FInd.push_back(VIndex);
		}

		CalculateNormals(Data);
		CalculateTangents(Data);
	}
	else
	{
		Data->FVerC.resize(Data->RawVertexCoordinates.size() * 3);
		Data->FTexC.resize(0);
		Data->FNorC.resize(0);
		Data->FTanC.resize(0);
		Data->FInd.resize(0);
		Data->MatIDs.resize(0);

		for (size_t i = 0; i < Data->RawIndices.size(); i++)
		{
			// faces index in OBJ file begins from 1 not 0.
			int VIndex = Data->RawIndices[i] - 1;
			//int tIndex = Data->RawIndices[i + 1] - 1;
			//int nIndex = Data->RawIndices[i + 2] - 1;

			int ShiftInVerArr = VIndex * 3;
			//int shiftInTexArr = vIndex * 2;

			Data->FVerC[ShiftInVerArr] = Data->RawVertexCoordinates[VIndex][0];
			Data->FVerC[ShiftInVerArr + 1] = Data->RawVertexCoordinates[VIndex][1];
			Data->FVerC[ShiftInVerArr + 2] = Data->RawVertexCoordinates[VIndex][2];

			//Data->fTexC[shiftInTexArr] = Data->rawTextureCoordinates[tIndex][0];
			//Data->fTexC[shiftInTexArr + 1] = 1 - Data->rawTextureCoordinates[tIndex][1];

			//Data->fNorC[shiftInVerArr] = Data->rawNormalCoordinates[nIndex][0];
			//Data->fNorC[shiftInVerArr + 1] = Data->rawNormalCoordinates[nIndex][1];
			//Data->fNorC[shiftInVerArr + 2] = Data->rawNormalCoordinates[nIndex][2];

			Data->FInd.push_back(VIndex);
		}

		Data->FNorC.resize(Data->RawVertexCoordinates.size() * 3);
		CalculateNormals(Data);
	}
}

void ObjLoader::ReadMaterialFile(const char* OriginalObjFile)
{
	if (MaterialFileName.empty() || OriginalObjFile == "")
		return;

	std::string MaterialFileFullPath = FILE_SYSTEM.GetDirectoryPath(OriginalObjFile);
	MaterialFileFullPath += MaterialFileName;
	if (!FILE_SYSTEM.CheckFile(MaterialFileFullPath.c_str()))
	{
		LOG.Add(std::string("material file: ") + MaterialFileName + " was indicated in OBJ file but this file can't be located.", "FE_LOG_LOADING", FE_LOG_ERROR);
		return;
	}

	std::ifstream File;
	File.open(MaterialFileFullPath);

	if ((File.rdstate() & std::ifstream::failbit) != 0)
	{
		LOG.Add(std::string("can't load material file: ") + MaterialFileFullPath + " in function FEObjLoader::readMaterialFile.", "FE_LOG_LOADING", FE_LOG_ERROR);
		return;
	}

	std::stringstream FileData;
	FileData << File.rdbuf();
	File.close();

	std::string Line;
	while (std::getline(FileData, Line))
	{
		// read next line
		std::stringstream LineStream;
		LineStream << Line;

		ReadMaterialLine(LineStream);
	}
}

bool ObjLoader::CheckCurrentMaterialObject()
{
	if (CurrentMaterialObject == nullptr)
	{
		LOG.Add("currentMaterialObject is nullptr in function FEObjLoader::readMaterialLine.", "FE_LOG_LOADING", FE_LOG_ERROR);
		return false;
	}

	if (CurrentMaterialObject->MaterialRecords.empty())
	{
		LOG.Add("materialRecords is empty in function FEObjLoader::readMaterialLine.", "FE_LOG_LOADING", FE_LOG_ERROR);
		return false;
	}

	return true;
}

void ObjLoader::ReadMaterialLine(std::stringstream& LineStream)
{
	auto LookForFile = [&](std::string& FilePath) {
		if (CurrentMaterialObject->MaterialRecords[0].Name.find('/') != std::string::npos)
		{
			std::string name = CurrentMaterialObject->MaterialRecords[0].Name;
			for (size_t i = name.size() - 1; i > 0; i--)
			{
				if (name[i] == '/')
				{
					name.erase(name.begin() + i, name.end());
					break;
				}
			}

			const std::string NewPath = std::string(FILE_SYSTEM.GetDirectoryPath(CurrentFilePath.c_str())) + name + "/" + FilePath;
			FilePath = NewPath;
		}
		else
		{
			const std::string NewPath = std::string(FILE_SYSTEM.GetDirectoryPath(CurrentFilePath.c_str())) + FilePath;
			FilePath = NewPath;
		}
	};

	std::string STemp;
	std::string* StringEdited = nullptr;

	LineStream >> STemp;
	// To lower case
	std::transform(STemp.begin(), STemp.end(), STemp.begin(), [](const unsigned char C) { return std::tolower(C); });

	// if it is comment or object declaration or other not relevant info
	if (STemp[0] == '#' || STemp[0] == 'o')
	{
		// get to the next line
		return;
	}
	// if this line contains material declaration
	else if (STemp.find("newmtl") != std::string::npos)
	{
		std::string MaterialName;
		LineStream >> MaterialName;

		for (size_t i = 0; i < LoadedObjects.size(); i++)
		{
			for (size_t j = 0; j < LoadedObjects[i]->MaterialRecords.size(); j++)
			{
				if (LoadedObjects[i]->MaterialRecords[j].Name == MaterialName)
				{
					CurrentMaterialObject = LoadedObjects[i];
					return;
				}
			}
		}
		
		LOG.Add(std::string("can't find material: ") + MaterialName + " from material file in function FEObjLoader::readMaterialLine.", "FE_LOG_LOADING", FE_LOG_ERROR);
	}
	// The diffuse texture map.
	else if (STemp.find("map_kd") != std::string::npos)
	{
		if (!CheckCurrentMaterialObject())
			return;

		StringEdited = &CurrentMaterialObject->MaterialRecords[0].AlbedoMapFile;
		std::getline(LineStream, *StringEdited);

		if ((*StringEdited)[0] == ' ')
			StringEdited->erase(StringEdited->begin());

		if (!FILE_SYSTEM.CheckFile(StringEdited->c_str()))
			LookForFile(*StringEdited);
	}
	// Specular color texture map
	else if (STemp.find("map_ks") != std::string::npos)
	{
		if (!CheckCurrentMaterialObject())
			return;

		StringEdited = &CurrentMaterialObject->MaterialRecords[0].SpecularMapFile;
		std::getline(LineStream, *StringEdited);

		if ((*StringEdited)[0] == ' ')
			StringEdited->erase(StringEdited->begin());

		if (!FILE_SYSTEM.CheckFile(StringEdited->c_str()))
			LookForFile(*StringEdited);
	}
	// Specular highlight component
	else if (STemp.find("map_ns") != std::string::npos)
	{
		if (!CheckCurrentMaterialObject())
			return;

		StringEdited = &CurrentMaterialObject->MaterialRecords[0].SpecularHighlightMapFile;
		std::getline(LineStream, *StringEdited);

		if ((*StringEdited)[0] == ' ')
			StringEdited->erase(StringEdited->begin());

		if (!FILE_SYSTEM.CheckFile(StringEdited->c_str()))
			LookForFile(*StringEdited);
	}
	// The alpha texture map
	else if (STemp.find("map_d") != std::string::npos)
	{
		if (!CheckCurrentMaterialObject())
			return;

		StringEdited = &CurrentMaterialObject->MaterialRecords[0].AlphaMapFile;
		std::getline(LineStream, *StringEdited);

		if ((*StringEdited)[0] == ' ')
			StringEdited->erase(StringEdited->begin());

		if (!FILE_SYSTEM.CheckFile(StringEdited->c_str()))
			LookForFile(*StringEdited);
	}
	// Some implementations use 'map_bump' instead of 'bump' below
	else if (STemp.find("map_bump") != std::string::npos)
	{
		if (!CheckCurrentMaterialObject())
			return;

		StringEdited = &CurrentMaterialObject->MaterialRecords[0].NormalMapFile;
		std::getline(LineStream, *StringEdited);

		if ((*StringEdited)[0] == ' ')
			StringEdited->erase(StringEdited->begin());

		if (!FILE_SYSTEM.CheckFile(StringEdited->c_str()))
			LookForFile(*StringEdited);
	}
	// Bump map (which by default uses luminance channel of the image)
	else if (STemp.find("bump") != std::string::npos)
	{
		if (!CheckCurrentMaterialObject())
			return;

		StringEdited = &CurrentMaterialObject->MaterialRecords[0].NormalMapFile;
		std::getline(LineStream, *StringEdited);

		if ((*StringEdited)[0] == ' ')
			StringEdited->erase(StringEdited->begin());

		if (!FILE_SYSTEM.CheckFile(StringEdited->c_str()))
			LookForFile(*StringEdited);
	}
	// Displacement map
	else if (STemp.find("disp") != std::string::npos)
	{
		if (!CheckCurrentMaterialObject())
			return;

		StringEdited = &CurrentMaterialObject->MaterialRecords[0].DisplacementMapFile;
		std::getline(LineStream, *StringEdited);

		if ((*StringEdited)[0] == ' ')
			StringEdited->erase(StringEdited->begin());

		if (!FILE_SYSTEM.CheckFile(StringEdited->c_str()))
			LookForFile(*StringEdited);
	}
	// Stencil decal texture (defaults to 'matte' channel of the image)
	else if (STemp.find("decal") != std::string::npos)
	{
		if (!CheckCurrentMaterialObject())
			return;

		StringEdited = &CurrentMaterialObject->MaterialRecords[0].StencilDecalMapFile;
		std::getline(LineStream, *StringEdited);

		if ((*StringEdited)[0] == ' ')
			StringEdited->erase(StringEdited->begin());

		if (!FILE_SYSTEM.CheckFile(StringEdited->c_str()))
			LookForFile(*StringEdited);
	}
}