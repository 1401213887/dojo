#pragma once

#include "dojo_config.h"

#include <cmath>
#include <cassert>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cfloat>
#include <climits>
#include <cstdint>

#include <array>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <functional>
#include <queue>
#include <atomic>
#include <thread>
#include <type_traits>
#include <utility>
#include <stdexcept>
#include <map>
#include <future>
#include <chrono>

//TODO move as many libraries as possible as inner dependencies, stop pushing them on users

#ifdef _MSC_VER
#pragma warning(push, 0)
#endif
#define GLM_FORCE_RADIANS //for sanity - why would it use degrees in places by default?
#include "glm/glm/glm.hpp"
#include "glm/glm/gtc/quaternion.hpp"
#include "glm/glm/gtc/type_ptr.hpp"
#include "glm/glm/gtc/matrix_transform.hpp"
#include "glm/glm/gtc/round.hpp"
#include "glm/glm/gtc/packing.hpp"
#include "glm/glm/gtx/bit.hpp"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "SmallSet.h"

#ifndef PLATFORM_WIN32
	#include <sys/time.h>
#endif

#if defined(PLATFORM_WIN32)
	//this cant be in config.h as it breaks successive system includes
	#ifdef _DEBUG

		#define _CRTDBG_MAP_ALLOC
		#include <crtdbg.h>

	#endif
#elif defined( PLATFORM_OSX )

#elif defined( PLATFORM_LINUX )
	#include <signal.h>
	#include <stdint.h>

#elif defined( PLATFORM_ANDROID )

	#include <stdint.h>
	#include "android/AndroidGLExtern.h"
	#include <android/log.h>
	#include <android/native_activity.h>
	#include <android_native_app_glue.h>

#else
	#error "No Platform defined!"

#endif

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H

#include "DebugUtils.h"

namespace Dojo {
	typedef int64_t RandomSeed;

	typedef std::function<void()> AsyncTask;
	typedef std::function<void()> AsyncCallback;

	using TimePoint = std::chrono::high_resolution_clock::time_point;
	using Duration = std::chrono::high_resolution_clock::duration;

	template<typename D>
	inline double durationToSeconds(const D& duration) {
		return std::chrono::duration_cast<std::chrono::duration<double>>(duration).count();
	}

}

using std::make_unique;
using std::make_shared;

#ifdef _MSC_VER
	//enable additional pragmas on MSVC
	#pragma warning(3:4062) //incomplete switch
	#pragma warning(3:4265) //'class': class has virtual functions, but destructor is not virtual
	#pragma warning(3:4296) //expression is always false
	#pragma warning(3:4701) //use of uninitialized variable
	#pragma warning(3:4702) //unreachable code
	#pragma warning(3:4189) //variable assigned but never used
	#pragma warning(4:4242)
	#pragma warning(default: 4254)

	#pragma warning(disable:4100) //unreferenced formal parameters are ok
	#pragma warning(disable:4458) //there's way too much shadowing here //TODO remove shadowing?
	#pragma warning(disable:4512) //what's this even?
	#pragma warning(disable:4503) //template name is too long and was truncated
#endif

#define UNUSED(X) ((void)(X))

#define self (*this)

//visual studio is non-standard and doesn't work with those
#ifdef _MSC_VER
#include <ciso646>
#endif

//TODO this stuff could be split off in another library?
#include "UTFString.h"
#include "dojostring.h"

#include "enum_cast.h"
#include "optional_ref.h"
#include "vec_view.h"
