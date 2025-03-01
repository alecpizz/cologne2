#include "engine/Engine.h"


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
