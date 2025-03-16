#pragma once

namespace goon
{
    struct Material
    {
        Texture albedo;
        Texture normal;
        Texture metallic;
        Texture roughness;
        Texture ao;
        Texture emission;
    };
}
