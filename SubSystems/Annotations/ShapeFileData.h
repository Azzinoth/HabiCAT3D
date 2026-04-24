#pragma once

#include "../AnalysisObjectManager.h"
using namespace FocalEngine;
#include "ogrsf_frmts.h"

struct ShapeFileLayerInfo
{
	std::string Name;
	std::string GeometryType;
	int FeatureCount;
};

struct ShapeFileFieldInfo
{
	std::string Name;
	OGRFieldType Type;
	int Width;
	int Precision;
};

enum class ShapeFileGeometryType
{
    Unknown,
    Point,
    Polyline,
    Polygon
};

struct ShapeFileFeature
{
    // Only one of these will be populated per feature.
    std::vector<std::vector<glm::vec2>> Polygons;
    std::vector<glm::vec2> Polyline;
    glm::vec2 Point{ 0,0 };

    ShapeFileGeometryType Type = ShapeFileGeometryType::Unknown;
    std::unordered_map<std::string, std::variant<int64_t, double, std::string>> Fields;

    template<typename T>
    T GetField(const std::string& Key, T DefaultValue = {}) const {
        auto MapIterator = Fields.find(Key);
        if (MapIterator == Fields.end())
            return DefaultValue;

        if (auto* Value = std::get_if<T>(&MapIterator->second))
            return *Value;

        return DefaultValue;
    }
};

class ShapeFileData
{
	std::vector<ShapeFileFeature> Features;
	std::vector<ShapeFileLayerInfo> LayersInfo;
	std::vector<ShapeFileFieldInfo> FieldDefinitions;

	FEAABB Bounds;

    void ReadFields(OGRFeature* Feature, ShapeFileFeature& Result);
    void ReadGeometry(OGRGeometry* Geometry, ShapeFileFeature& Result);
public:
	ShapeFileData() = default;
	~ShapeFileData() = default;

	bool Load(const std::string& Filepath);

    FEAABB GetBounds() const;
    const std::vector<ShapeFileFieldInfo>& GetFieldDefinitions() const;
    bool HasField(const std::string& Key) const;

    const std::vector<ShapeFileFeature>& GetFeatures() const;
};