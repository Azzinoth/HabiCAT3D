#pragma once

#include "AnalysisObjectManager.h"
#include "MCPPluginInterface.h"
using namespace FocalEngine;

class MCPToolManager
{
public:
	SINGLETON_PUBLIC_PART(MCPToolManager)

	// Registers HabiCAT tools and starts MCP server, does nothing if MCPPlugin is not linked.
	void Initialize();
private:
	SINGLETON_PRIVATE_PART(MCPToolManager)

	static constexpr const char* OwnerName = "HabiCAT3D";

	static MCPPluginInterface* GetMCPPlugin();

	static Json::Value ParseArguments(const std::string& ArgumentsJSON);
	static MCPPluginInterface::ToolResult MakeResult(const Json::Value& Value);
	static MCPPluginInterface::ToolResult MakeErrorResult(const std::string& Text);
	static MCPPluginInterface::ToolResult MakeAnalysisObjectNotFoundResult(const std::string& AnalysisObjectID);
	static MCPPluginInterface::ToolResult MakeBusyResult();

	static std::string DataSourceTypeToString(DATA_SOURCE_TYPE Type);
	static std::string LayerTypeToString(LAYER_TYPE Type);
	static Json::Value StatisticToJSON(float Value);

	static MCPPluginInterface::ToolResult ListAnalysisObjects(const std::string& ArgumentsJSON);
	static MCPPluginInterface::ToolResult GetAnalysisObjectInfo(const std::string& ArgumentsJSON);
	static MCPPluginInterface::ToolResult ListLayers(const std::string& ArgumentsJSON);
	static MCPPluginInterface::ToolResult GetLayerInfo(const std::string& ArgumentsJSON);

	static MCPPluginInterface::ToolResult GetActiveAnalysisObject(const std::string& ArgumentsJSON);
	static MCPPluginInterface::ToolResult SetActiveAnalysisObject(const std::string& ArgumentsJSON);
	static MCPPluginInterface::ToolResult GetActiveLayer(const std::string& ArgumentsJSON);
	static MCPPluginInterface::ToolResult SetActiveLayer(const std::string& ArgumentsJSON);
};

#define MCP_TOOL_MANAGER MCPToolManager::GetInstance()