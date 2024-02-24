cmake_minimum_required(VERSION 3.10)

set(BUILD_TYPE "Debug and Release" CACHE STRING "Choose Build type")
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CONFIGURATION_TYPES Debug Release)

# set the project name
project(RugosityCalculator LANGUAGES CXX)

set(CMAKE_CXX_FLAGS_DEBUG "/MP /Zi /INCREMENTAL:NO /DEBUG /OPT:REF /OPT:ICF /Od")
set(CMAKE_CXX_FLAGS_RELEASE "/MP")

# Because of we are catching CGAL exceptions in the loop, we need to enable this flag
if(MSVC)
    add_compile_options(/bigobj)
endif()

# Turn on the ability to create folders to organize projects (.vcproj)
# It creates "CMakePredefinedTargets" folder by default and adds CMake
# defined projects like INSTALL.vcproj and ZERO_CHECK.vcproj
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(NOT TARGET FEBasicApplication)
  add_subdirectory(SubSystems/FEBasicApplication)
endif()

# *************** THIRD_PARTY ***************

file(GLOB lodepng_SRC
	"ThirdParty/lodepng/lodepng.cpp"
	"ThirdParty/lodepng/lodepng.h"
)

# *************** SUB_SYSTEM ***************

file(GLOB SubSystems_SRC
	"SubSystems/FEObjLoader.h"
	"SubSystems/FEObjLoader.cpp"
	"SubSystems/FEBasicCamera.h"
	"SubSystems/FEBasicCamera.cpp"
	"SubSystems/FEFreeCamera.h"
	"SubSystems/FEFreeCamera.cpp"
	"SubSystems/FEModelViewCamera.h"
	"SubSystems/FEModelViewCamera.cpp"
	"SubSystems/FEFileSystem.cpp"
	"SubSystems/FEFileSystem.h"
	"SubSystems/FETransformComponent.cpp"
	"SubSystems/FETransformComponent.h"
	"SubSystems/FECGALWrapper.cpp"
	"SubSystems/FECGALWrapper.h"
	"SubSystems/ExcelWrapper.cpp"
	"SubSystems/ExcelWrapper.h"
	"SubSystems/FEGeometricTools.cpp"
	"SubSystems/FEGeometricTools.h"
	"SubSystems/FELinesRenderer.cpp"
	"SubSystems/FELinesRenderer.h"
	"SubSystems/MeshManager.cpp"
	"SubSystems/MeshManager.h"
	"SubSystems/FESDF.cpp"
	"SubSystems/FESDF.h"
	"SubSystems/JitterManager.cpp"
	"SubSystems/JitterManager.h"
	"SubSystems/SphereVectors.h"
	"SubSystems/MeshRenderer.cpp"
	"SubSystems/MeshRenderer.h"
	"SubSystems/ScreenshotManager.cpp"
	"SubSystems/ScreenshotManager.h"
)

file(GLOB SubSystems_Layers_SRC
	"SubSystems/Layers/HeightLayerProducer.cpp"
	"SubSystems/Layers/HeightLayerProducer.h"
	"SubSystems/Layers/AreaLayerProducer.cpp"
	"SubSystems/Layers/AreaLayerProducer.h"
	"SubSystems/Layers/TriangleEdgeLayerProducer.cpp"
	"SubSystems/Layers/TriangleEdgeLayerProducer.h"
	"SubSystems/Layers/CompareLayerProducer.cpp"
	"SubSystems/Layers/CompareLayerProducer.h"
	"SubSystems/Layers/RugosityLayerProducer.cpp" 
	"SubSystems/Layers/RugosityLayerProducer.h"
	"SubSystems/Layers/VectorDispersionLayerProducer.cpp"
	"SubSystems/Layers/VectorDispersionLayerProducer.h"
	"SubSystems/Layers/FractalDimensionLayerProducer.cpp"
	"SubSystems/Layers/FractalDimensionLayerProducer.h"
	"SubSystems/Layers/TriangleCountLayerProducer.cpp"
	"SubSystems/Layers/TriangleCountLayerProducer.h"
	"SubSystems/Layers/LayerManager.cpp"
	"SubSystems/Layers/LayerManager.h"
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
	"FECoreIncludes.h"
	"FEFileSystem.cpp"
	"FEFileSystem.h"
	"FEObject.cpp"
	"FEObject.h"
	"FEShader.cpp"
	"FEShader.h"
	"FETexture.cpp"
	"FETexture.h"
	"ComplexityMetricInfo.cpp"
	"ComplexityMetricInfo.h"
	"ComplexityMetricManager.cpp"
	"ComplexityMetricManager.h"
	"FEMesh.cpp"
	"FEMesh.h"
	"main.cpp"
)

link_directories(${GLEW_LIB_DIR})
link_directories(${GLFW_LIB_DIR})

# add the executable
add_executable(RugosityCalculator WIN32
								  ${source_SRC}
								  # *************** SUB_SYSTEM ***************
								  ${SubSystems_SRC}
								  ${SubSystems_Layers_SRC}
								  ${SubSystems_UI_SRC}
								  ${SubSystems_ConsoleJobs_SRC}
								  # *************** THIRD_PARTY ***************
								  ${lodepng_SRC}
								  ${imgui_SRC}
)

target_link_libraries(RugosityCalculator FEBasicApplication)

target_link_libraries(RugosityCalculator glew32s.lib)
target_link_libraries(RugosityCalculator glfw3.lib)
target_link_libraries(RugosityCalculator glfw3dll.lib)
target_link_libraries(RugosityCalculator opengl32.lib)

source_group("Source Files" FILES ${source_SRC})
# *************** SUB_SYSTEM ***************
source_group("Source Files/SubSystems" FILES ${SubSystems_SRC})
source_group("Source Files/SubSystems/Layers" FILES ${SubSystems_Layers_SRC})
source_group("Source Files/SubSystems/UI" FILES ${SubSystems_UI_SRC})
source_group("Source Files/SubSystems/ConsoleJobs" FILES ${SubSystems_ConsoleJobs_SRC})
# *************** THIRD_PARTY ***************
source_group("Source Files/ThirdParty/lodepng" FILES ${lodepng_SRC})

# set as the startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT RugosityCalculator)

include_directories(
	${GLEW_INCLUDE_DIR}
	${BASICAPP_THIRDPARTY_DIR}
	"ThirdParty/"
)