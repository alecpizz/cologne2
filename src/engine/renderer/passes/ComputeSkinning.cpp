//
// Created by alecpizz on 7/7/25.
//
#include <engine/asset_manager/AssetManager.h>
#include <engine/core/Engine.h>
#include <engine/editor/Editor.h>
#include <engine/renderer/Renderer.h>
#include <engine/renderer/types/Shader.h>
#include <engine/renderer/types/SSBO.h>

namespace cologne
{
    void Renderer::compute_skinning_pass()
    {
        auto shader = get_shader_by_name("compute_skinning");
        auto transforms = get_ssbo_by_name("skinning_transforms");
        if (!shader)
        {
            return;
        }
        if (!transforms)
        {
            return;
        }
        int total_vertex_count = 0;
        static std::vector<glm::mat4> skinning_transforms(2048);
        skinning_transforms.clear();
        for (auto& skinned_render_item : _skinned_render_items)
        {
            auto model = AssetManager::get_skinned_model_by_index(skinned_render_item.id);
            if (!model)
            {
                continue;
            }

            const auto& transforms = skinned_render_item.bones;
            skinning_transforms.insert(skinning_transforms.end(), transforms.begin(), transforms.end());
        }
        transforms->update(skinning_transforms.size() * sizeof(glm::mat4), skinning_transforms.data());

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, get_skinned_vbo());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, get_skinned_bind_pose_vbo());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, transforms->get_handle());
        shader->bind();
        int j = 0;
        uint32_t base_output_vertex;
    }
}
