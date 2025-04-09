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
#define ALBEDO_INDEX 0
#define AO_INDEX 1
#define METALLIC_INDEX 2
#define ROUGHNESS_INDEX 3
#define NORMAL_INDEX 4
#define EMISSION_INDEX 5

        void bind_all()
        {
            albedo.bind(ALBEDO_INDEX);
            ao.bind(AO_INDEX);
            metallic.bind(METALLIC_INDEX);
            roughness.bind(ROUGHNESS_INDEX);
            normal.bind(NORMAL_INDEX);
            emission.bind(EMISSION_INDEX);
        }
    };
}
