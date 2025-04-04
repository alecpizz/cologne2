#include "engine/Engine.h"

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

int main()
{
    goon::Engine engine;
    if (!engine.init(1280, 720))
    {
        return -1;
    }
    engine.run();

    return 0;
}
