#pragma once
#pragma warning (disable: 4752)     // found Intel(R) Advanced Vector Extensions; consider using / arch:AVX	FocalEnginePrivate
#pragma warning (disable: 4334)     // '<<': result of 32-bit shift implicitly converted to 64 bits (was 64-bit shift intended?) in lodepng.cpp

#include "FEBasicApplication/FEBasicApplication.h"

#include <iostream>
#include <fstream>
#include <sstream>

// run time checking(/RTC) makes the Microsoft implementation of std containers 
// *very* slow in debug builds !
// So in visual studio go to Project -> Properties -> C/C++ -> Code Generation -> Basic Runtime Checks and set to "Default"
#include <unordered_map>
#include <map>
#define GLM_FORCE_XYZW_ONLY
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.inl"
#include "glm/gtx/quaternion.hpp"

#include <xmmintrin.h>
#include <stdlib.h>

#include <thread>
#include <atomic>
#include <functional>
#include <random>

#define ANGLE_TORADIANS_COF glm::pi<float>() / 180.0f

//#define SINGLETON_PUBLIC_PART(CLASS_NAME)  \
//static CLASS_NAME& getInstance()           \
//{										   \
//	if (!_instance)                        \
//		_instance = new CLASS_NAME();      \
//	return *_instance;				       \
//}                                          \
//										   \
//~CLASS_NAME();
//
//#define SINGLETON_PRIVATE_PART(CLASS_NAME) \
//static CLASS_NAME* _instance;              \
//CLASS_NAME();                              \
//CLASS_NAME(const CLASS_NAME &);            \
//void operator= (const CLASS_NAME &);

#ifdef FE_DEBUG_ENABLED
	#define FE_GL_ERROR(glCall)            \
	{                                      \
		glCall;                            \
		GLenum error = glGetError();	   \
		if (error != 0)                    \
		{								   \
			assert("FE_GL_ERROR" && 0);	   \
		}                                  \
	}
#else
	#define FE_GL_ERROR(glCall)            \
	{                                      \
		glCall;                            \
	}
#endif // FE_GL_ERROR

#define FE_MAP_TO_STR_VECTOR(map)          \
std::vector<std::string> result;           \
auto iterator = map.begin();               \
while (iterator != map.end())              \
{                                          \
	result.push_back(iterator->first);     \
	iterator++;                            \
}                                          \
                                           \
return result;

#define FE_WIN_32