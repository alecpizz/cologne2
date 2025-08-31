#pragma once
#include "Texture.h"

namespace cologne
{
    struct GPUMaterial
    {
        uint64_t albedo = 0;
        uint64_t normal = 0;
        uint64_t orm = 0;
        uint64_t emission = 0;
        float roughness_mod = 1.0f;
        float metallic_mod = 1.0f;
    };

    struct Material
    {
        Texture albedo;
        Texture normal;
        Texture orm;
        Texture emission;
        float roughness_override = 1.0f;
        float metallic_override = 1.0f;
#define ALBEDO_INDEX 0
#define ORM_INDEX 1
#define NORMAL_INDEX 2
#define EMISSION_INDEX 3

        void bind_all()
        {
            albedo.bind(ALBEDO_INDEX);
            orm.bind(ORM_INDEX);
            normal.bind(NORMAL_INDEX);
            emission.bind(EMISSION_INDEX);
        }

        void ensure_bindless()
        {
            albedo.make_resident();
            orm.make_resident();
            normal.make_resident();
            emission.make_resident();
        }

        void load_all()
        {
            albedo.load_compressed();
            albedo.load();
            orm.load_compressed();
            orm.load();
            normal.load_compressed();
            normal.load();
            emission.load_compressed();
            emission.load();
        }

        void cleanup_all()
        {
            albedo.cleanup();
            orm.cleanup();
            normal.cleanup();
            emission.cleanup();
        }

        GPUMaterial to_gpu_material() const
        {
            return {
                albedo.get_bindless_handle(),
                normal.get_bindless_handle(),
                orm.get_bindless_handle(),
                emission.get_bindless_handle(),
                roughness_override,
                metallic_override
            };
        }
    };
}
