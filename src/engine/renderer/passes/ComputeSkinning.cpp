//
// Created by alecpizz on 7/7/25.
//
#include <engine/asset_manager/AssetManager.h>
#include <engine/core/Engine.h>
#include <engine/editor/Editor.h>
#include <engine/renderer/OpenGLDebugScope.h>
#include <engine/renderer/Renderer.h>
#include <engine/renderer/types/Shader.h>
#include <engine/renderer/types/SSBO.h>

namespace cologne
{
    void Renderer::compute_skinning_pass()
    {
        OpenGLDebugScope scope("Renderer::compute_skinning");
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
        for (auto& skinned_render_item : _skinned_render_items)
        {
            auto mesh = AssetManager::get_skinned_mesh_by_index(skinned_render_item.mesh_idx);
            if (!mesh)
            {
                continue;
            }
            total_vertex_count += mesh->get_vertices().size();
        }
        allocate_weighted_vertex_buffer(total_vertex_count);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, get_skinned_vbo());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, get_skinned_bind_pose_vbo());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, transforms->get_handle());
        shader->bind();
        int32_t base_output_vertex = 0;
        int32_t base_transform_index = 0;
        for (auto render_item : _skinned_render_items)
        {
            auto mesh = AssetManager::get_skinned_mesh_by_index(render_item.mesh_idx);
            if (!mesh)
            {
                continue;
            }
            shader->set_int("vertex_count", mesh->get_vertices().size());
            shader->set_int("base_input_vertex", mesh->get_base_vertex());
            shader->set_int("base_output_vertex", base_output_vertex);
            shader->set_int("base_transform_index", base_transform_index);

            uint32_t work_group_size = 128;
            uint32_t x = (mesh->get_vertices().size() + work_group_size - 1) / work_group_size;
            glDispatchCompute(x, 1, 1);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_UNIFORM_BARRIER_BIT);

            MultiDrawElementsCommand cmd{};
            cmd.index_count = mesh->get_indices().size();
            cmd.instance_count = 1;
            cmd.first_index = mesh->get_first_index();
            cmd.base_vertex = base_output_vertex;
            cmd.base_instance = render_item.entity_id;
            _skinned_render_cmds.emplace_back(cmd);


            base_output_vertex += mesh->get_vertices().size();
            base_transform_index += render_item.bones.size();
        }

        get_ssbo_by_name("skinned_draw_cmds")->update(sizeof(MultiDrawElementsCommand) * _skinned_render_cmds.size(), _skinned_render_cmds.data());
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_UNIFORM_BARRIER_BIT);
    }
}
