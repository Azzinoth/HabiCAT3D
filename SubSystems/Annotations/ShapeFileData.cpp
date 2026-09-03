#include "ShapeFileData.h"
using namespace FocalEngine;

FEAABB ShapeFileData::GetBounds() const
{
	return Bounds;
}

const std::vector<ShapeFileFieldInfo>& ShapeFileData::GetFieldDefinitions() const
{
	return FieldDefinitions;
}

bool ShapeFileData::HasField(const std::string& Key) const
{
	for (size_t i = 0; i < FieldDefinitions.size(); i++)
	{
		if (FieldDefinitions[i].Name == Key)
			return true;
	}
	
	return false;
}

const std::vector<ShapeFileFeature>& ShapeFileData::GetFeatures() const
{
	return Features;
}

void ShapeFileData::ReadFields(OGRFeature* Feature, ShapeFileFeature& Result)
{
	for (int i = 0; i < static_cast<int>(FieldDefinitions.size()); i++)
	{
		if (!Feature->IsFieldSetAndNotNull(i))
			continue;

		const std::string& FieldName = FieldDefinitions[i].Name;
		switch (FieldDefinitions[i].Type)
		{
			case OFTInteger:
			{
				Result.Fields[FieldName] = static_cast<int64_t>(Feature->GetFieldAsInteger(i));
				break;
			}
			case OFTInteger64:
			{
				Result.Fields[FieldName] = Feature->GetFieldAsInteger64(i);
				break;
			}
			case OFTReal:
			{
				Result.Fields[FieldName] = Feature->GetFieldAsDouble(i);
				break;
			}
			default:
			{
				Result.Fields[FieldName] = std::string(Feature->GetFieldAsString(i));
				break;
			}
		}
	}
}

void ShapeFileData::ReadGeometry(OGRGeometry* Geometry, ShapeFileFeature& Result)
{
	if (auto* PointGeometry = dynamic_cast<OGRPoint*>(Geometry))
	{
		Result.Type = ShapeFileGeometryType::Point;
		Result.Point = glm::vec2(PointGeometry->getX(), PointGeometry->getY());

		std::vector<glm::vec3> AABBPoints{ {Result.Point, 0} };
		Bounds = FEAABB(AABBPoints);
	}
	else if (auto* LineGeometry = dynamic_cast<OGRLineString*>(Geometry))
	{
		Result.Type = ShapeFileGeometryType::Polyline;
		int PointCount = LineGeometry->getNumPoints();
		Result.Polyline.resize(PointCount);
		std::vector<glm::vec3> AABBPoints;
		for (int i = 0; i < PointCount; i++)
		{
			Result.Polyline[i] = glm::vec2(LineGeometry->getX(i), LineGeometry->getY(i));
			AABBPoints.push_back({ Result.Polyline[i], 0 });
		}
		
		Bounds = FEAABB(AABBPoints);
	}
	else if (auto* PolygonGeometry = dynamic_cast<OGRPolygon*>(Geometry))
	{
		Result.Type = ShapeFileGeometryType::Polygon;
		OGRLinearRing* Ring = PolygonGeometry->getExteriorRing();
		int PointCount = Ring->getNumPoints();
		std::vector<glm::vec2> Points(PointCount);
		std::vector<glm::vec3> AABBPoints;
		for (int i = 0; i < PointCount; i++)
		{
			Points[i] = glm::vec2(Ring->getX(i), Ring->getY(i));
			AABBPoints.push_back({ Points[i], 0 });
		}
		Result.Polygons.push_back(std::move(Points));
		Bounds = Bounds.Merge(FEAABB(AABBPoints));
	}
	else if (auto* MultiPolygonGeometry = dynamic_cast<OGRMultiPolygon*>(Geometry))
	{
		Result.Type = ShapeFileGeometryType::Polygon;
		for (int i = 0; i < MultiPolygonGeometry->getNumGeometries(); i++)
		{
			auto* CurrentPolygon = dynamic_cast<OGRPolygon*>(MultiPolygonGeometry->getGeometryRef(i));
			if (!CurrentPolygon)
				continue;

			OGRLinearRing* Ring = CurrentPolygon->getExteriorRing();
			int PointCount = Ring->getNumPoints();
			std::vector<glm::vec2> Points(PointCount);
			std::vector<glm::vec3> AABBPoints;
			for (int j = 0; j < PointCount; j++)
			{
				Points[j] = glm::vec2(Ring->getX(j), Ring->getY(j));
				AABBPoints.push_back({ Points[j], 0 });
			}
			Result.Polygons.push_back(std::move(Points));
			Bounds = Bounds.Merge(FEAABB(AABBPoints));
		}
	}
}

bool ShapeFileData::Load(const std::string& Filepath)
{
	auto* Dataset = (GDALDataset*)GDALOpenEx(Filepath.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr);
	if (Dataset == nullptr)
		return false;

	LayersInfo.resize(Dataset->GetLayerCount());
	for (int i = 0; i < LayersInfo.size(); i++)
	{
		OGRLayer* layer = Dataset->GetLayer(i);
		if (layer == nullptr)
			continue;

		LayersInfo[i].Name = layer->GetName();
		LayersInfo[i].FeatureCount = static_cast<int>(layer->GetFeatureCount());
		LayersInfo[i].GeometryType = OGRGeometryTypeToName(layer->GetGeomType());
	}

	auto* CurrentLayer = Dataset->GetLayer(0);
	OGRFeatureDefn* FeatureDefinition = CurrentLayer->GetLayerDefn();
	int FieldCount = FeatureDefinition->GetFieldCount();
	FieldDefinitions.resize(FieldCount);
	for (int i = 0; i < FieldCount; i++)
	{
		OGRFieldDefn* FieldDefinition = FeatureDefinition->GetFieldDefn(i);
		FieldDefinitions[i] = {
			FieldDefinition->GetNameRef(),
			FieldDefinition->GetType(),
			FieldDefinition->GetWidth(),
			FieldDefinition->GetPrecision()
		};
	}

	for (auto& OriginalFeature : CurrentLayer)
	{
		ShapeFileFeature NewFeature;
		ReadFields(OriginalFeature.get(), NewFeature);

		OGRGeometry* Geometry = OriginalFeature->GetGeometryRef();
		if (Geometry != nullptr)
			ReadGeometry(Geometry, NewFeature);

		Features.push_back(std::move(NewFeature));
	}

	GDALClose(Dataset);
	return true;
}