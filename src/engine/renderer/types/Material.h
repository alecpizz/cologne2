#pragma once
#include "Texture.h"

namespace cologne
{
    struct GPUMaterial
    {
        uint64_t albedo = 0;
        uint64_t normal = 0;
        uint64_t metallic = 0;
        uint64_t roughness = 0;
        uint64_t ao = 0;
        uint64_t emission = 0;
        float roughness_mod = 1.0f;
        float metallic_mod = 1.0f;
    };

    struct Material
    {
        Texture albedo;
        Texture normal;
        Texture metallic;
        Texture roughness;
        Texture ao;
        Texture emission;
        float roughness_override = 1.0f;
        float metallic_override = 1.0f;
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

        void ensure_bindless()
        {
            albedo.make_resident();
            ao.make_resident();
            metallic.make_resident();
            roughness.make_resident();
            normal.make_resident();
            emission.make_resident();
        }

        void load_all()
        {
            albedo.load();
            ao.load();
            metallic.load();
            roughness.load();
            normal.load();
            emission.load();
        }

        GPUMaterial to_gpu_material() const
        {
            return {
                albedo.get_bindless_handle(),
                normal.get_bindless_handle(),
                metallic.get_bindless_handle(),
                roughness.get_bindless_handle(),
                ao.get_bindless_handle(),
                emission.get_bindless_handle(),
                roughness_override,
                metallic_override
            };
        }
    };
}
