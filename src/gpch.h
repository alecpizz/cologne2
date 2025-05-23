﻿#pragma once

#include <any>
#include <map>
#include <set>
#include <array>
#include <mutex>
#include <atomic>
#include <memory>
#include <string>
#include <vector>
#include <cstdint>
#include <sstream>
#include <utility>
#include <iostream>
#include <functional>
#include <algorithm>
#include <typeindex>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <glad/glad.h>

#include "engine/Log.h"


#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>



#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_opengl3.h"


#define GLM_ENABLE_EXPERIMENTAL
#include "../vendor/glm/glm/glm.hpp"
#include "../vendor/glm/glm/glm.hpp"
#include "../vendor/glm/glm/gtc/type_ptr.hpp"
#include "../vendor/glm/glm/gtx/matrix_decompose.hpp"
#include "../vendor/glm/glm/gtc/matrix_transform.hpp"
#include "../vendor/glm/glm/gtx/rotate_vector.hpp"
#include "../vendor/glm/glm/gtx/quaternion.hpp"
#include "../vendor/glm/glm/gtx/euler_angles.hpp"
#include <stb_image/stb_image.h>
