#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif
#define TINYEXR_USE_MINIZ (0)
#define TINYEXR_USE_STB_ZLIB (1)
#define TINYEXR_IMPLEMENTATION
#include <tinyexr/tinyexr.h>
