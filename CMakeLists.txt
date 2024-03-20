cmake_minimum_required(VERSION 3.10)

set(BUILD_TYPE "Debug and Release" CACHE STRING "Choose Build type")
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CONFIGURATION_TYPES Debug Release)

# set the project name
project(HabiCAT3D LANGUAGES CXX)

set(CMAKE_CXX_FLAGS_DEBUG "/MP /Zi /MDd /INCREMENTAL:NO /DEBUG /OPT:REF /OPT:ICF /Od")
set(CMAKE_CXX_FLAGS_RELEASE "/MP")

# Because of we are catching CGAL exceptions in the loop, we need to enable this flag
if(MSVC)
    add_compile_options(/bigobj)
endif()

# Turn on the ability to create folders to organize projects (.vcproj)
# It creates "CMakePredefinedTargets" folder by default and adds CMake
# defined projects like INSTALL.vcproj and ZERO_CHECK.vcproj
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(NOT TARGET FocalEngine)
	add_subdirectory(SubSystems/FocalEngine)
endif()

# Extract the relative path of the engine folder
file(RELATIVE_PATH RELATIVE_PATH_TO_ENGINE_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}" "${ENGINE_FOLDER}")
# We need to re-configure the Config.h file for the engine
# This is necessary for any project that will include FocalEngine
# to make sure that the engine can find its dependencies
set(ENGINE_FOLDER "${RELATIVE_PATH_TO_ENGINE_FOLDER}")
configure_file(${ENGINE_FOLDER}/ResourceManager/Config.h.in ${ENGINE_FOLDER}/ResourceManager/Config.h @ONLY)

# *************** SUB_SYSTEM ***************

file(GLOB SubSystems_SRC
	"SubSystems/CGALDeclarations.h"
	"SubSystems/FELinesRenderer.cpp"
	"SubSystems/FELinesRenderer.h"
	"SubSystems/MeshManager.cpp"
	"SubSystems/MeshManager.h"
	"SubSystems/SphereVectors.h"
	"SubSystems/MeshRenderer.cpp"
	"SubSystems/MeshRenderer.h"
	"SubSystems/LayerRasterizationManager.cpp"
	"SubSystems/LayerRasterizationManager.h"
	"SubSystems/ScreenshotManager.cpp"
	"SubSystems/ScreenshotManager.h"
)

file(GLOB SubSystems_Layers_SRC
	"SubSystems/ComplexityCore/Layers/HeightLayerProducer.cpp"
	"SubSystems/ComplexityCore/Layers/HeightLayerProducer.h"
	"SubSystems/ComplexityCore/Layers/AreaLayerProducer.cpp"
	"SubSystems/ComplexityCore/Layers/AreaLayerProducer.h"
	"SubSystems/ComplexityCore/Layers/TriangleEdgeLayerProducer.cpp"
	"SubSystems/ComplexityCore/Layers/TriangleEdgeLayerProducer.h"
	"SubSystems/ComplexityCore/Layers/CompareLayerProducer.cpp"
	"SubSystems/ComplexityCore/Layers/CompareLayerProducer.h"
	"SubSystems/ComplexityCore/Layers/RugosityLayerProducer.cpp" 
	"SubSystems/ComplexityCore/Layers/RugosityLayerProducer.h"
	"SubSystems/ComplexityCore/Layers/VectorDispersionLayerProducer.cpp"
	"SubSystems/ComplexityCore/Layers/VectorDispersionLayerProducer.h"
	"SubSystems/ComplexityCore/Layers/FractalDimensionLayerProducer.cpp"
	"SubSystems/ComplexityCore/Layers/FractalDimensionLayerProducer.h"
	"SubSystems/ComplexityCore/Layers/TriangleCountLayerProducer.cpp"
	"SubSystems/ComplexityCore/Layers/TriangleCountLayerProducer.h"
	"SubSystems/ComplexityCore/Layers/LayerManager.cpp"
	"SubSystems/ComplexityCore/Layers/LayerManager.h"
)

file(GLOB SubSystems_ComplexityCore_SRC
	"SubSystems/ComplexityCore/MeasurementGrid.cpp"
	"SubSystems/ComplexityCore/MeasurementGrid.h"
	"SubSystems/ComplexityCore/ComplexityMetricInfo.cpp"
	"SubSystems/ComplexityCore/ComplexityMetricInfo.h"
	"SubSystems/ComplexityCore/ComplexityMetricManager.cpp"
	"SubSystems/ComplexityCore/ComplexityMetricManager.h"
	"SubSystems/ComplexityCore/JitterManager.cpp"
	"SubSystems/ComplexityCore/JitterManager.h"
)

file(GLOB SubSystems_UI_SRC
	"SubSystems/UI/UIComponents.cpp"
	"SubSystems/UI/UIComponents.h"
	"SubSystems/UI/UIWeightedHistogram.cpp"
	"SubSystems/UI/UIWeightedHistogram.h"
	"SubSystems/UI/NewLayerWindow.cpp"
	"SubSystems/UI/NewLayerWindow.h"
	"SubSystems/UI/UIManager.cpp"
	"SubSystems/UI/UIManager.h"
)

file(GLOB SubSystems_ConsoleJobs_SRC
	"SubSystems/ConsoleJobs/ConsoleJobManager.h"
	"SubSystems/ConsoleJobs/ConsoleJobManager.cpp"
)

file(GLOB source_SRC
	"EngineInclude.h"
	"main.cpp"
)

link_directories(${GLEW_LIB_DIR})
link_directories(${GLFW_LIB_DIR})

# add the executable
add_executable(HabiCAT3D WIN32
					     ${source_SRC}
					     # *************** SUB_SYSTEM ***************
					     ${SubSystems_SRC}
					     ${SubSystems_Layers_SRC}
					     ${SubSystems_UI_SRC}
					     ${SubSystems_ConsoleJobs_SRC}
					     ${SubSystems_ComplexityCore_SRC}
)

target_link_libraries(HabiCAT3D
	PRIVATE
		FocalEngine
	 PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/GDAL/gdal_i.lib
)

source_group("Source Files" FILES ${source_SRC})
# *************** SUB_SYSTEM ***************
source_group("Source Files/SubSystems" FILES ${SubSystems_SRC})
source_group("Source Files/SubSystems/ComplexityCore" FILES ${SubSystems_ComplexityCore_SRC})
source_group("Source Files/SubSystems/ComplexityCore/Layers" FILES ${SubSystems_Layers_SRC})
source_group("Source Files/SubSystems/UI" FILES ${SubSystems_UI_SRC})
source_group("Source Files/SubSystems/ConsoleJobs" FILES ${SubSystems_ConsoleJobs_SRC})

# set as the startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT HabiCAT3D)

include_directories(
	${GLEW_INCLUDE_DIR}
	${BASICAPP_THIRDPARTY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/SubSystems/FocalEngine/SubSystems/FEBasicApplication/ThirdParty/imgui
	${BASICAPP_DIR}
	${FOCAL_ENGINE_THIRD_PARTY_DIR}
	"ThirdParty/GDAL"
	"ThirdParty/GDAL/ogr"
	"ThirdParty/GDAL/port"
	"ThirdParty/"
)