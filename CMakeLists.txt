cmake_minimum_required(VERSION 3.10)

set(BUILD_TYPE "Debug and Release" CACHE STRING "Choose Build type")
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CONFIGURATION_TYPES Debug Release)

# set the project name
project(HabiCAT3D)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS_DEBUG "/MP /Zi /MTd /INCREMENTAL:NO /DEBUG /OPT:REF /OPT:ICF /Od")
set(CMAKE_CXX_FLAGS_RELEASE "/MP /MT")
# Set the stack reserve size to 10 MB
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:10485760")

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

if(USE_STATIC_RUNTIME)
    # Add standard FocalEngine native scripts
    # TODO: This should be done in the FocalEngine project, or make it simpler to add it user projects
    file(GLOB Unpacked_NativeScript_2B7956623302254F620A675F_SRC
	    "SubSystems/FocalEngine/Resources/UserScriptsData/ExtractedScriptModules/2B7956623302254F620A675F/FirstScript.h"
	    "SubSystems/FocalEngine/Resources/UserScriptsData/ExtractedScriptModules/2B7956623302254F620A675F/FirstScript.cpp"
    )
endif()

# *************** SUB_SYSTEM ***************

file(GLOB SubSystems_SRC
	"SubSystems/CGALDeclarations.h"
	"SubSystems/FELinesRenderer.cpp"
	"SubSystems/FELinesRenderer.h"
	"SubSystems/SceneResources.cpp"
	"SubSystems/SceneResources.h"
	"SubSystems/SphereVectors.h"
	"SubSystems/LayerRasterizationManager.cpp"
	"SubSystems/LayerRasterizationManager.h"
	"SubSystems/ScreenshotManager.cpp"
	"SubSystems/ScreenshotManager.h"
)

file(GLOB SubSystems_Layers_Producers_SRC
	"SubSystems/ComplexityCore/Layers/Producers/HeightLayerProducer.cpp"
	"SubSystems/ComplexityCore/Layers/Producers/HeightLayerProducer.h"
	"SubSystems/ComplexityCore/Layers/Producers/AreaLayerProducer.cpp"
	"SubSystems/ComplexityCore/Layers/Producers/AreaLayerProducer.h"
	"SubSystems/ComplexityCore/Layers/Producers/TriangleEdgeLayerProducer.cpp"
	"SubSystems/ComplexityCore/Layers/Producers/TriangleEdgeLayerProducer.h"
	"SubSystems/ComplexityCore/Layers/Producers/CompareLayerProducer.cpp"
	"SubSystems/ComplexityCore/Layers/Producers/CompareLayerProducer.h"
	"SubSystems/ComplexityCore/Layers/Producers/RugosityLayerProducer.cpp" 
	"SubSystems/ComplexityCore/Layers/Producers/RugosityLayerProducer.h"
	"SubSystems/ComplexityCore/Layers/Producers/VectorDispersionLayerProducer.cpp"
	"SubSystems/ComplexityCore/Layers/Producers/VectorDispersionLayerProducer.h"
	"SubSystems/ComplexityCore/Layers/Producers/FractalDimensionLayerProducer.cpp"
	"SubSystems/ComplexityCore/Layers/Producers/FractalDimensionLayerProducer.h"
	"SubSystems/ComplexityCore/Layers/Producers/TriangleCountLayerProducer.cpp"
	"SubSystems/ComplexityCore/Layers/Producers/TriangleCountLayerProducer.h"
	"SubSystems/ComplexityCore/Layers/Producers/PointDensityLayerProducer.cpp"
	"SubSystems/ComplexityCore/Layers/Producers/PointDensityLayerProducer.h"
)

file(GLOB SubSystems_Layers_SRC
	"SubSystems/ComplexityCore/Layers/DataLayer.cpp"
	"SubSystems/ComplexityCore/Layers/DataLayer.h"
	"SubSystems/ComplexityCore/Layers/LayerManager.cpp"
	"SubSystems/ComplexityCore/Layers/LayerManager.h"
)

file(GLOB SubSystems_ComplexityCore_SRC
	"SubSystems/ComplexityCore/AnalysisObjectManager.cpp"
	"SubSystems/ComplexityCore/AnalysisObjectManager.h"
	"SubSystems/ComplexityCore/MeasurementGrid.cpp"
	"SubSystems/ComplexityCore/MeasurementGrid.h"
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
	"SubSystems/ConsoleJobs/ConsoleJob.h"
	"SubSystems/ConsoleJobs/ConsoleJob.cpp"
	"SubSystems/ConsoleJobs/ExitJob.h"
	"SubSystems/ConsoleJobs/ExitJob.cpp"
	"SubSystems/ConsoleJobs/HelpJob.h"
	"SubSystems/ConsoleJobs/HelpJob.cpp"
	"SubSystems/ConsoleJobs/FileLoadJob.h"
	"SubSystems/ConsoleJobs/FileLoadJob.cpp"
	"SubSystems/ConsoleJobs/FileSaveJob.h"
	"SubSystems/ConsoleJobs/FileSaveJob.cpp"
	"SubSystems/ConsoleJobs/ComplexityJob.h"
	"SubSystems/ConsoleJobs/ComplexityJob.cpp"
	"SubSystems/ConsoleJobs/EvaluationJob.h"
	"SubSystems/ConsoleJobs/EvaluationJob.cpp"
	"SubSystems/ConsoleJobs/GlobalSettingJob.h"
	"SubSystems/ConsoleJobs/GlobalSettingJob.cpp"
	"SubSystems/ConsoleJobs/ExportLayerAsImageJob.h"
	"SubSystems/ConsoleJobs/ExportLayerAsImageJob.cpp"
	"SubSystems/ConsoleJobs/QueryJob.h"
	"SubSystems/ConsoleJobs/QueryJob.cpp"
	"SubSystems/ConsoleJobs/ConsoleJobManager.h"
	"SubSystems/ConsoleJobs/ConsoleJobManager.cpp"
)

file(GLOB SubSystems_VRManager_SRC
	"SubSystems/VRManager/VRManager.h"
	"SubSystems/VRManager/VRManager.cpp"
)

set(MAIN_SOURCE_FILES "")
list(APPEND MAIN_SOURCE_FILES
    "EngineInclude.h"
	"EngineInclude.cpp"
	"main.cpp"
	"HabiCAT3D.rc"
)

set(EXTRACTED_NATIVE_SCRIPTS "")
if(USE_STATIC_RUNTIME)
	list(APPEND EXTRACTED_NATIVE_SCRIPTS
		${Unpacked_NativeScript_2B7956623302254F620A675F_SRC}
	)

    add_definitions(-DGLEW_STATIC)
endif()

link_directories(${GLEW_LIB_DIR})
link_directories(${GLFW_LIB_DIR})

# add the executable
add_executable(HabiCAT3D WIN32
					     ${MAIN_SOURCE_FILES}
						 ${EXTRACTED_NATIVE_SCRIPTS}
					     # *************** SUB_SYSTEM ***************
					     ${SubSystems_SRC}
					     ${SubSystems_Layers_SRC}
						 ${SubSystems_Layers_Producers_SRC}
					     ${SubSystems_UI_SRC}
					     ${SubSystems_ConsoleJobs_SRC}
					     ${SubSystems_ComplexityCore_SRC}
						 ${SubSystems_VRManager_SRC}
)

target_link_libraries(HabiCAT3D
	PRIVATE
		FocalEngine
	 PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/GDAL/gdal_i.lib
)

source_group("Source Files" FILES ${MAIN_SOURCE_FILES})
if(USE_STATIC_RUNTIME)
	source_group("Source Files/FocalEngine/2B7956623302254F620A675F" FILES ${EXTRACTED_NATIVE_SCRIPTS})
endif()
# *************** SUB_SYSTEM ***************
source_group("Source Files/SubSystems" FILES ${SubSystems_SRC})
source_group("Source Files/SubSystems/ComplexityCore" FILES ${SubSystems_ComplexityCore_SRC})
source_group("Source Files/SubSystems/ComplexityCore/Layers" FILES ${SubSystems_Layers_SRC})
source_group("Source Files/SubSystems/ComplexityCore/Layers/Producers" FILES ${SubSystems_Layers_Producers_SRC})
source_group("Source Files/SubSystems/UI" FILES ${SubSystems_UI_SRC})
source_group("Source Files/SubSystems/ConsoleJobs" FILES ${SubSystems_ConsoleJobs_SRC})
source_group("Source Files/SubSystems/VRManager" FILES ${SubSystems_VRManager_SRC})

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

if(LASZIP_FINAL_DLL)
	add_custom_command(TARGET HabiCAT3D POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${LASZIP_FINAL_DLL}
        "$<TARGET_FILE_DIR:HabiCAT3D>"
	)
endif()