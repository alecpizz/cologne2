#include <engine/core/Engine.h>

#ifdef _WIN32
#ifdef __cplusplus
extern "C" {
#endif

    __declspec(dllexport) uint32_t NvOptimusEnablement = 1;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;

#ifdef __cplusplus
}
#endif
#endif

#define DEFAULT_WIDTH 1600
#define DEFAULT_HEIGHT 900

int main()
{
    cologne::Engine engine;
    if (!engine.init(DEFAULT_WIDTH, DEFAULT_HEIGHT))
    {
        return -1;
    }
    engine.run();

    return 0;
}
