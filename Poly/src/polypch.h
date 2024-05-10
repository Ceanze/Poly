#pragma once

#define NOMINMAX

#pragma warning(disable : 26812) // Disable unscoped enum warning

// Standard libraries
#include <string>
#include <set>
#include <algorithm>
#include <chrono>
#include <vector>

// Libs
#pragma warning(push)
#pragma warning(disable : 26495) // Disable uninitalized warning from GLM (it's a lot)
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtc/quaternion.hpp"
#pragma warning(pop)

// Custom files
#include "Poly/Core/Logger.h"
#include "Poly/Core/PolyUtils.h"
#include "Poly/Core/Core.h"
#include "Poly/Core/PolyID.h"
