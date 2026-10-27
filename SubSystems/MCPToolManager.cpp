#include "MCPToolManager.h"
#include "UI/UIManager.h"
using namespace FocalEngine;

MCPToolManager::MCPToolManager() {}
MCPToolManager::~MCPToolManager() {}

void MCPToolManager::Initialize()
{
	MCPPluginInterface* MCPPlugin = GetMCPPlugin();
	if (MCPPlugin == nullptr)
		return;

	MCPPlugin->RegisterTool(OwnerName, "list_analysis_objects",
		"Lists IDs of analysis objects, meshes and point clouds loaded in HabiCAT. Result: {\"analysis_object_ids\": [...]}.",
		R"({"type": "object", "properties": {}, "additionalProperties": false})",
		ListAnalysisObjects);

	MCPPlugin->RegisterTool(OwnerName, "get_analysis_object_info",
		"Gets name and type of an analysis object, type is Mesh or Point cloud. Result: {\"analysis_object_id\", \"name\", \"type\"}.",
		R"({
			"type": "object",
			"properties": {"analysis_object_id": {"type": "string", "description": "ID from list_analysis_objects."}},
			"required": ["analysis_object_id"],
			"additionalProperties": false
		})",
		GetAnalysisObjectInfo);

	MCPPlugin->RegisterTool(OwnerName, "list_layers",
		"Lists IDs of layers of an analysis object, layer has a value for every triangle of mesh or every point of point cloud. "
		"Result: {\"analysis_object_id\", \"layer_ids\": [...]}.",
		R"({
			"type": "object",
			"properties": {"analysis_object_id": {"type": "string", "description": "ID from list_analysis_objects."}},
			"required": ["analysis_object_id"],
			"additionalProperties": false
		})",
		ListLayers);

	MCPPlugin->RegisterTool(OwnerName, "get_layer_info",
		"Gets caption, type, user note and statistics of a layer, statistic is null when it was not computed. "
		"Result: {\"analysis_object_id\", \"layer_id\", \"caption\", \"type\", \"note\", \"min\", \"max\", \"mean\", \"median\"}.",
		R"({
			"type": "object",
			"properties": {
				"analysis_object_id": {"type": "string", "description": "ID from list_analysis_objects."},
				"layer_id": {"type": "string", "description": "ID from list_layers."}
			},
			"required": ["analysis_object_id", "layer_id"],
			"additionalProperties": false
		})",
		GetLayerInfo);

	MCPPlugin->RegisterTool(OwnerName, "get_active_analysis_object",
		"Gets ID of active analysis object, the one HabiCAT shows with its active layer, null when no object is active. Result: {\"analysis_object_id\"}.",
		R"({"type": "object", "properties": {}, "additionalProperties": false})",
		GetActiveAnalysisObject);

	MCPPlugin->RegisterTool(OwnerName, "set_active_analysis_object",
		"Makes analysis object active, it is shown with the layer that was active in it before. Result: {\"analysis_object_id\"}.",
		R"({
			"type": "object",
			"properties": {"analysis_object_id": {"type": "string", "description": "ID from list_analysis_objects."}},
			"required": ["analysis_object_id"],
			"additionalProperties": false
		})",
		SetActiveAnalysisObject);

	MCPPlugin->RegisterTool(OwnerName, "get_active_layer",
		"Gets ID of active layer of active analysis object, values of active layer are shown as colors, layer_id is null when no layer is active. "
		"Result: {\"analysis_object_id\", \"layer_id\"}.",
		R"({"type": "object", "properties": {}, "additionalProperties": false})",
		GetActiveLayer);

	MCPPlugin->RegisterTool(OwnerName, "set_active_layer",
		"Makes layer of active analysis object active, so its values are shown as colors. "
		"To use layer of other analysis object, make that object active first with set_active_analysis_object. Result: {\"analysis_object_id\", \"layer_id\"}.",
		R"({
			"type": "object",
			"properties": {"layer_id": {"type": ["string", "null"], "description": "ID from list_layers of active analysis object, null shows no layer."}},
			"required": ["layer_id"],
			"additionalProperties": false
		})",
		SetActiveLayer);

	// Editor uses default port 5187, so both could run at the same time.
	MCPPlugin->StartServer(5188);

	// Plugins are unloaded when the last frame ends, tools should be unregistered before that.
	PLUGIN_MANAGER.SetBeforeShutdownCallback([]()
	{
		MCPPluginInterface* Plugin = GetMCPPlugin();
		if (Plugin != nullptr)
			Plugin->UnregisterToolsOfOwner(OwnerName);
	});
}

MCPPluginInterface* MCPToolManager::GetMCPPlugin()
{
	MCPPluginInterface* MCPPlugin = PLUGIN_MANAGER.GetPlugin<MCPPluginInterface>(UNIQUE_ID.FromString(MCPPluginInterface::PluginID));
	if (MCPPlugin == nullptr || MCPPlugin->GetInterfaceVersion() < MCPPluginInterface::InterfaceVersion)
		return nullptr;

	return MCPPlugin;
}

// Arguments are already checked against input schema of the tool.
Json::Value MCPToolManager::ParseArguments(const std::string& ArgumentsJSON)
{
	Json::Value Result;
	Json::CharReaderBuilder Builder;
	std::unique_ptr<Json::CharReader> Reader(Builder.newCharReader());
	std::string Errors;
	Reader->parse(ArgumentsJSON.data(), ArgumentsJSON.data() + ArgumentsJSON.size(), &Result, &Errors);
	return Result;
}

MCPPluginInterface::ToolResult MCPToolManager::MakeResult(const Json::Value& Value)
{
	Json::StreamWriterBuilder Builder;
	Builder["indentation"] = "";
	// Values are floats, with more digits result would show only noise of conversion to double.
	Builder["precision"] = 7;

	MCPPluginInterface::ToolResult Result;
	Result.Text = Json::writeString(Builder, Value);
	return Result;
}

MCPPluginInterface::ToolResult MCPToolManager::MakeErrorResult(const std::string& Text)
{
	MCPPluginInterface::ToolResult Result;
	Result.bIsError = true;
	Result.Text = Text;
	return Result;
}

MCPPluginInterface::ToolResult MCPToolManager::MakeAnalysisObjectNotFoundResult(const std::string& AnalysisObjectID)
{
	return MakeErrorResult("Analysis object " + AnalysisObjectID + " does not exist, list_analysis_objects shows existing analysis objects");
}

// UI does not let user change anything while progress popup is open, calculation could still use active object.
MCPPluginInterface::ToolResult MCPToolManager::MakeBusyResult()
{
	return MakeErrorResult("HabiCAT is busy with a calculation, try again after it finishes");
}

std::string MCPToolManager::DataSourceTypeToString(DATA_SOURCE_TYPE Type)
{
	switch (Type)
	{
		case DATA_SOURCE_TYPE::MESH:
			return "Mesh";
		case DATA_SOURCE_TYPE::POINT_CLOUD:
			return "Point cloud";
		default:
			return "Unknown";
	}
}

std::string MCPToolManager::LayerTypeToString(LAYER_TYPE Type)
{
	switch (Type)
	{
		case LAYER_TYPE::HEIGHT:
			return "Height";
		case LAYER_TYPE::TRIANGLE_EDGE:
			return "Triangle edges";
		case LAYER_TYPE::TRIANGLE_AREA:
			return "Triangle area";
		case LAYER_TYPE::COMPARE:
			return "Compare layers";
		case LAYER_TYPE::STANDARD_DEVIATION:
			return "Standard deviation";
		case LAYER_TYPE::RUGOSITY:
			return "Rugosity";
		case LAYER_TYPE::VECTOR_DISPERSION:
			return "Vector dispersion";
		case LAYER_TYPE::FRACTAL_DIMENSION:
			return "Fractal dimension";
		case LAYER_TYPE::TRIANGLE_DENSITY:
			return "Triangle density";
		case LAYER_TYPE::POINT_DENSITY:
			return "Point density";
		case LAYER_TYPE::STRUCTURAL_ROUGHNESS:
			return "Structural roughness";
		case LAYER_TYPE::INTERPOLATION:
			return "Interpolation";
		default:
			return "Unknown";
	}
}

// DataLayer keeps FLT_MAX or -FLT_MAX in statistics that were not computed.
Json::Value MCPToolManager::StatisticToJSON(float Value)
{
	if (Value == FLT_MAX || Value == -FLT_MAX)
		return Json::Value();

	return Json::Value(static_cast<double>(Value));
}

MCPPluginInterface::ToolResult MCPToolManager::ListAnalysisObjects(const std::string& ArgumentsJSON)
{
	Json::Value AnalysisObjectIDs(Json::arrayValue);
	std::vector<FEUUID> IDList = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectsIDList();
	for (size_t i = 0; i < IDList.size(); i++)
		AnalysisObjectIDs.append(UNIQUE_ID.ToString(IDList[i]));

	Json::Value Result;
	Result["analysis_object_ids"] = AnalysisObjectIDs;
	return MakeResult(Result);
}

MCPPluginInterface::ToolResult MCPToolManager::GetAnalysisObjectInfo(const std::string& ArgumentsJSON)
{
	Json::Value Arguments = ParseArguments(ArgumentsJSON);
	std::string AnalysisObjectID = Arguments["analysis_object_id"].asString();
	AnalysisObject* Object = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectByID(UNIQUE_ID.FromString(AnalysisObjectID));
	if (Object == nullptr)
		return MakeAnalysisObjectNotFoundResult(AnalysisObjectID);

	Json::Value Result;
	Result["analysis_object_id"] = UNIQUE_ID.ToString(Object->GetID());
	Result["name"] = Object->GetName();
	Result["type"] = DataSourceTypeToString(Object->GetType());
	return MakeResult(Result);
}

MCPPluginInterface::ToolResult MCPToolManager::ListLayers(const std::string& ArgumentsJSON)
{
	Json::Value Arguments = ParseArguments(ArgumentsJSON);
	std::string AnalysisObjectID = Arguments["analysis_object_id"].asString();
	AnalysisObject* Object = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectByID(UNIQUE_ID.FromString(AnalysisObjectID));
	if (Object == nullptr)
		return MakeAnalysisObjectNotFoundResult(AnalysisObjectID);

	Json::Value LayerIDs(Json::arrayValue);
	for (size_t i = 0; i < Object->Layers.size(); i++)
		LayerIDs.append(UNIQUE_ID.ToString(Object->Layers[i]->GetID()));

	Json::Value Result;
	Result["analysis_object_id"] = UNIQUE_ID.ToString(Object->GetID());
	Result["layer_ids"] = LayerIDs;
	return MakeResult(Result);
}

MCPPluginInterface::ToolResult MCPToolManager::GetLayerInfo(const std::string& ArgumentsJSON)
{
	Json::Value Arguments = ParseArguments(ArgumentsJSON);
	std::string AnalysisObjectID = Arguments["analysis_object_id"].asString();
	AnalysisObject* Object = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectByID(UNIQUE_ID.FromString(AnalysisObjectID));
	if (Object == nullptr)
		return MakeAnalysisObjectNotFoundResult(AnalysisObjectID);

	std::string LayerID = Arguments["layer_id"].asString();
	DataLayer* Layer = Object->GetLayer(UNIQUE_ID.FromString(LayerID));
	if (Layer == nullptr)
		return MakeErrorResult("Layer " + LayerID + " does not exist in analysis object " + AnalysisObjectID + ", list_layers shows layers of analysis object");

	Json::Value Result;
	Result["analysis_object_id"] = UNIQUE_ID.ToString(Object->GetID());
	Result["layer_id"] = UNIQUE_ID.ToString(Layer->GetID());
	Result["caption"] = Layer->GetCaption();
	Result["type"] = LayerTypeToString(Layer->GetType());
	Result["note"] = Layer->GetNote();
	Result["min"] = StatisticToJSON(Layer->GetMin());
	Result["max"] = StatisticToJSON(Layer->GetMax());
	Result["mean"] = StatisticToJSON(Layer->GetMean());
	Result["median"] = StatisticToJSON(Layer->GetMedian());
	return MakeResult(Result);
}

MCPPluginInterface::ToolResult MCPToolManager::GetActiveAnalysisObject(const std::string& ArgumentsJSON)
{
	AnalysisObject* Object = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();

	Json::Value Result;
	Result["analysis_object_id"] = Object != nullptr ? Json::Value(UNIQUE_ID.ToString(Object->GetID())) : Json::Value();
	return MakeResult(Result);
}

MCPPluginInterface::ToolResult MCPToolManager::SetActiveAnalysisObject(const std::string& ArgumentsJSON)
{
	if (UI.IsProgressModalPopupOpen())
		return MakeBusyResult();

	Json::Value Arguments = ParseArguments(ArgumentsJSON);
	std::string AnalysisObjectID = Arguments["analysis_object_id"].asString();
	AnalysisObject* Object = ANALYSIS_OBJECT_MANAGER.GetAnalysisObjectByID(UNIQUE_ID.FromString(AnalysisObjectID));
	if (Object == nullptr)
		return MakeAnalysisObjectNotFoundResult(AnalysisObjectID);

	ANALYSIS_OBJECT_MANAGER.SetActiveAnalysisObject(Object->GetID());
	return GetActiveAnalysisObject(ArgumentsJSON);
}

MCPPluginInterface::ToolResult MCPToolManager::GetActiveLayer(const std::string& ArgumentsJSON)
{
	AnalysisObject* Object = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	DataLayer* Layer = Object != nullptr ? Object->GetActiveLayer() : nullptr;

	Json::Value Result;
	Result["analysis_object_id"] = Object != nullptr ? Json::Value(UNIQUE_ID.ToString(Object->GetID())) : Json::Value();
	Result["layer_id"] = Layer != nullptr ? Json::Value(UNIQUE_ID.ToString(Layer->GetID())) : Json::Value();
	return MakeResult(Result);
}

// Only layer of active object could be made active, because HabiCAT applies active layer change to active object.
MCPPluginInterface::ToolResult MCPToolManager::SetActiveLayer(const std::string& ArgumentsJSON)
{
	if (UI.IsProgressModalPopupOpen())
		return MakeBusyResult();

	AnalysisObject* Object = ANALYSIS_OBJECT_MANAGER.GetActiveAnalysisObject();
	if (Object == nullptr)
		return MakeErrorResult("No analysis object is active, set_active_analysis_object makes analysis object active");

	Json::Value Arguments = ParseArguments(ArgumentsJSON);
	FEUUID LayerID;
	if (!Arguments["layer_id"].isNull())
	{
		std::string LayerIDText = Arguments["layer_id"].asString();
		LayerID = UNIQUE_ID.FromString(LayerIDText);
		if (Object->GetLayer(LayerID) == nullptr)
			return MakeErrorResult("Layer " + LayerIDText + " does not belong to active analysis object " + UNIQUE_ID.ToString(Object->GetID()) + ", list_layers shows its layers");
	}

	Object->SetActiveLayer(LayerID);
	return GetActiveLayer(ArgumentsJSON);
}