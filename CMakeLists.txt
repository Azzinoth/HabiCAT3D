cmake_minimum_required(VERSION 3.1)

set(BUILD_TYPE "Debug and Release" CACHE STRING "Choose Build type")
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CONFIGURATION_TYPES Debug Release)

# set the project name
project(RugosityCalculator LANGUAGES CXX)

set(CMAKE_CXX_FLAGS_DEBUG "/MP /Zi /INCREMENTAL:NO /DEBUG /OPT:REF /OPT:ICF /Od")
set(CMAKE_CXX_FLAGS_RELEASE "/MP")

# Turn on the ability to create folders to organize projects (.vcproj)
# It creates "CMakePredefinedTargets" folder by default and adds CMake
# defined projects like INSTALL.vcproj and ZERO_CHECK.vcproj
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# *************** THIRD_PARTY ***************

file(GLOB lodepng_SRC
	"ThirdParty/lodepng/lodepng.cpp"
	"ThirdParty/lodepng/lodepng.h"
)

file(GLOB imgui_SRC
	"SubSystems/FEBasicApplication/ThirdParty/imgui/imgui.cpp"
	"SubSystems/FEBasicApplication/ThirdParty/imgui/imgui_demo.cpp"
	"SubSystems/FEBasicApplication/ThirdParty/imgui/imgui_draw.cpp"
	"SubSystems/FEBasicApplication/ThirdParty/imgui/imgui_impl_glfw.cpp"
	"SubSystems/FEBasicApplication/ThirdParty/imgui/imgui_impl_opengl3.cpp"
	"SubSystems/FEBasicApplication/ThirdParty/imgui/imgui_tables.cpp"
	"SubSystems/FEBasicApplication/ThirdParty/imgui/imgui_widgets.cpp"
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
	"SubSystems/RugosityManager.cpp"
	"SubSystems/RugosityManager.h"
)

file(GLOB Layers_SRC
	"SubSystems/Layers/HeightLayerProducer.cpp"
	"SubSystems/Layers/HeightLayerProducer.h"
	"SubSystems/Layers/CompareLayerProducer.cpp"
	"SubSystems/Layers/CompareLayerProducer.h"
	"SubSystems/Layers/LayerManager.cpp"
	"SubSystems/Layers/LayerManager.h"
)

file(GLOB SubSystems_UI_SRC
	"SubSystems/UI/UIComponents.cpp"
	"SubSystems/UI/UIComponents.h"
	"SubSystems/UI/NewLayerWindow.cpp"
	"SubSystems/UI/NewLayerWindow.h"
	"SubSystems/UI/UIManager.cpp"
	"SubSystems/UI/UIManager.h"
)

file(GLOB BasicApplicationSubSystem_SRC
	"SubSystems/FEBasicApplication/FEBasicApplication.cpp"
	"SubSystems/FEBasicApplication/FEBasicApplication.h"
	"SubSystems/FEBasicApplication/FEThreadPool.cpp"
	"SubSystems/FEBasicApplication/FEThreadPool.h"
	"SubSystems/FEBasicApplication/FELog.cpp"
	"SubSystems/FEBasicApplication/FELog.h"
	"SubSystems/FEBasicApplication/FETime.cpp"
	"SubSystems/FEBasicApplication/FETime.h"
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
	"FEMesh.cpp"
	"FEMesh.h"
	"main.cpp"
)

link_directories("SubSystems/FEBasicApplication/ThirdParty/glew2/lib/$(PlatformTarget)")
link_directories("SubSystems/FEBasicApplication/ThirdParty/GLFW/lib/$(PlatformTarget)")

# add the executable
add_executable(RugosityCalculator WIN32
								  ${source_SRC}
								  # *************** SUB_SYSTEM ***************
								  ${SubSystems_SRC}
								  ${Layers_SRC}
								  ${SubSystems_UI_SRC}
								  ${BasicApplicationSubSystem_SRC}
								  # *************** THIRD_PARTY ***************
								  ${lodepng_SRC}
								  ${imgui_SRC}
)

target_link_libraries(RugosityCalculator glew32.lib)
target_link_libraries(RugosityCalculator glfw3.lib)
target_link_libraries(RugosityCalculator glfw3dll.lib)
target_link_libraries(RugosityCalculator opengl32.lib)

source_group("Source Files" FILES ${source_SRC})
# *************** SUB_SYSTEM ***************
source_group("Source Files/SubSystems" FILES ${SubSystems_SRC})
source_group("Source Files/SubSystems/Layers" FILES ${Layers_SRC})
source_group("Source Files/SubSystems/UI" FILES ${SubSystems_UI_SRC})
source_group("Source Files/SubSystems/BasicApplication" FILES ${BasicApplicationSubSystem_SRC})
# *************** THIRD_PARTY ***************
source_group("Source Files/ThirdParty/lodepng" FILES ${lodepng_SRC})
source_group("Source Files/ThirdParty/imgui" FILES ${imgui_SRC})

# set FEBasicApplication as the startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT RugosityCalculator)

include_directories(
	"SubSystems/FEBasicApplication/ThirdParty/glew2/include"
	"SubSystems/FEBasicApplication/ThirdParty/"
	"ThirdParty/"
)