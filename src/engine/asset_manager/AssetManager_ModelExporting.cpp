//
// Created by alecpizz on 7/16/25.
//
#include "AssetManager.h"
#include <filesystem>
#include <fstream>
#include <engine/util/DebugScope.h>
#include <engine/util/FileUtil.h>
#include <algorithm>
#include <string>
#include <vector>
#include <engine/core/Engine.h>
#include <engine/SerializationHeaders.h>
#include <engine/renderer/Renderer.h>

namespace cologne::AssetManager
{
#define NAME_BUFFER_SIZE 256

    void copy_name(char *buffer, const std::string &name, int size = NAME_BUFFER_SIZE)
    {
        std::memset(buffer, 0, size);
        std::memcpy(buffer, name.data(), std::min(static_cast<int>(name.length()), size - 1));
        buffer[std::min(static_cast<int>(name.length()), size - 1)] = '\0';
    }

    void process_material_header(std::ofstream& file, const std::vector<Material>& materials, const std::string& model_name)
    {
        int i = 0;
        for (auto &material: materials)
        {
            std::string path = "cache/textures/" + model_name + "/material" + std::to_string(i);
            MaterialCacheHeader material_header;
            if (material.albedo.contains_data())
            {
                copy_name(material_header.albedo_path, path + "/albedo.ctext", 512);
            }
            else
            {
                copy_name(material_header.albedo_path, "", 512);
            }
            if (material.roughness.contains_data())
            {
                copy_name(material_header.roughness_path, path + "/roughness.ctext", 512);
            }
            else
            {
                copy_name(material_header.roughness_path, "", 512);
            }
            if (material.ao.contains_data())
            {
                copy_name(material_header.ao_path, path + "/ao.ctext", 512);
            }
            else
            {
                copy_name(material_header.ao_path, "", 512);
            }
            if (material.emission.contains_data())
            {
                copy_name(material_header.emission_path, path + "/emission.ctext", 512);
            }
            else
            {
                copy_name(material_header.emission_path, "", 512);
            }
            if (material.metallic.contains_data())
            {
                copy_name(material_header.metallic_path, path + "/metallic.ctext", 512);
            }
            else
            {
                copy_name(material_header.metallic_path, "", 512);
            }
            if (material.normal.contains_data())
            {
                copy_name(material_header.normal_path, path + "/normal.ctext", 512);
            }
            else
            {
                copy_name(material_header.normal_path, "", 512);
            }
            material_header.metallic_override = material.metallic_override;
            material_header.roughness_override = material.roughness_override;
            file.write(reinterpret_cast<const char *>(&material_header), sizeof(MaterialCacheHeader));
            i++;
        }
    }

    void export_material_textures(const std::vector<Material>& materials, const std::string& model_name)
    {
        int i = 0;
        for (auto &material: materials)
        {
            std::string path = "cache/textures/" + model_name + "/material" + std::to_string(i);
            if (material.albedo.contains_data())
            {
                std::string albedo_path = path + "/albedo.ctext";
                material.albedo.export_to_compressed((RESOURCES_PATH + albedo_path).c_str());
            }
            if (material.roughness.contains_data())
            {
                std::string roughness_path = path + "/roughness.ctext";
                material.roughness.export_to_compressed((RESOURCES_PATH + roughness_path).c_str());
            }
            if (material.ao.contains_data())
            {
                std::string ao_path = path + "/ao.ctext";
                material.ao.export_to_compressed((RESOURCES_PATH + ao_path).c_str());
            }
            if (material.emission.contains_data())
            {
                std::string emission_path = path + "/emission.ctext";
                material.emission.export_to_compressed((RESOURCES_PATH + emission_path).c_str());
            }
            if (material.metallic.contains_data())
            {
                std::string metallic_path = path + "/metallic.ctext";
                material.metallic.export_to_compressed((RESOURCES_PATH + metallic_path).c_str());
            }
            if (material.normal.contains_data())
            {
                std::string normal_path = path + "/normal.ctext";
                material.normal.export_to_compressed((RESOURCES_PATH + normal_path).c_str());
            }
            i++;
        }

    }

    void export_model(const ModelData &model_data, uint64_t export_time)
    {
        const std::string output_path = RESOURCES_PATH "cache/models/" + model_data.name + ".cmdl";
        FileUtil::create_directory_recursive(output_path);
        std::ofstream file(output_path, std::ios::binary);
        if (!file.is_open())
        {
            LOG_ERROR("Couldn't open file for writing: %s", output_path);
            return;
        }
        //things to export:
        //the name of the model
        //aabb min
        //aabb max
        ModelCacheHeader header;
        header.mesh_count = model_data.meshes.size();
        header.material_count = model_data.materials.size();
        header.time_saved = export_time;
        header.aabb_max = model_data.aabb_max;
        header.aabb_min = model_data.aabb_min;
        copy_name(header.name, model_data.name);
        file.write(reinterpret_cast<const char *>(&header), sizeof(ModelCacheHeader));

        //the mesh datas
        for (const auto &mesh: model_data.meshes)
        {
            MeshCacheHeader mesh_header;
            copy_name(mesh_header.name, mesh.name);
            mesh_header.aabb_max = mesh.aabb_max;
            mesh_header.aabb_min = mesh.aabb_min;
            mesh_header.index_count = mesh.indices.size();
            mesh_header.vertex_count = mesh.vertices.size();
            mesh_header.material_index = mesh.material_index;
            mesh_header.inverse_bind_pose = mesh.inverse_bind_pose;

            file.write(reinterpret_cast<const char *>(&mesh_header), sizeof(MeshCacheHeader));

            file.write(reinterpret_cast<const char *>(mesh.vertices.data()), mesh.vertices.size() * sizeof(Vertex));
            file.write(reinterpret_cast<const char *>(mesh.indices.data()), mesh.indices.size() * sizeof(uint32_t));
        }
        //the materials
        process_material_header(file, model_data.materials, model_data.name);
        file.close();
        export_material_textures(model_data.materials, model_data.name);
        LOG_INFO("Exported model %s!", model_data.name.c_str());
    }

    void export_skinned_model(const SkinnedModelData &skinned_model_data, uint64_t export_time)
    {
        const std::string output_path = RESOURCES_PATH "cache/skinned_models/" + skinned_model_data.name + ".cskmdl";
        FileUtil::create_directory_recursive(output_path);
        std::ofstream file(output_path, std::ios::binary);
        if (!file.is_open())
        {
            LOG_ERROR("Couldn't open file for writing: %s", output_path);
            return;
        }

        //create the header giving high level info
        SkinnedModelCacheHeader header;
        header.mesh_count = skinned_model_data.meshes.size();
        header.material_count = skinned_model_data.materials.size();
        header.time_saved = export_time;
        header.aabb_max = skinned_model_data.aabb_max;
        header.aabb_min = skinned_model_data.aabb_min;
        copy_name(header.name, skinned_model_data.name);
        header.bone_count = skinned_model_data.skeleton.get_bone_count();
        header.animation_count = skinned_model_data.animations.size();

        //write meshes
        for (const auto& mesh : skinned_model_data.meshes)
        {
            MeshCacheHeader mesh_header;
            copy_name(mesh_header.name, mesh.name);
            mesh_header.aabb_max = mesh.aabb_max;
            mesh_header.aabb_min = mesh.aabb_min;
            mesh_header.index_count = mesh.indices.size();
            mesh_header.vertex_count = mesh.vertices.size();
            mesh_header.material_index=  mesh.material_index;
            mesh_header.inverse_bind_pose = glm::mat4(1.0f);

            file.write(reinterpret_cast<const char*>(&mesh_header), sizeof(MeshCacheHeader));

            file.write(reinterpret_cast<const char*>(mesh.vertices.data()), mesh.vertices.size() * sizeof(WeightedVertex));
            file.write(reinterpret_cast<const char *>(mesh.indices.data()), mesh.indices.size() * sizeof(uint32_t));
        }

        //need to export the following:
        //materials
        process_material_header(file, skinned_model_data.materials, skinned_model_data.name);
        //skeleton bones
        for (const auto& bone : skinned_model_data.skeleton._bones)
        {
            BoneHeader bone_header;
            copy_name(bone_header.name, bone.name, 512);
            bone_header.inverse_bind_pose = bone.inverse_bind_pose;
            bone_header.local_bind_transform = bone.local_bind_transform;
            bone_header.parent_idx = bone.parent_idx;

            file.write(reinterpret_cast<const char*>(&bone_header), sizeof(bone_header));
        }

        //skeleton hiearchy
        for (const auto&[bone_name, bone_idx] : skinned_model_data.skeleton._bone_name_to_index)
        {
            BoneMappingHeader bone_mapping_header;
            copy_name(bone_mapping_header.name, bone_name, 512);
            bone_mapping_header.index = bone_idx;

            file.write(reinterpret_cast<const char*>(&bone_mapping_header), sizeof(BoneMappingHeader));
        }

        //animations
        for (auto& anim : skinned_model_data.animations)
        {
            auto key_frames = anim.get_data();
            AnimationClipCacheHeader clip_header;
            clip_header.channel_count = key_frames.size();
            clip_header.duration = anim.get_duration();
            clip_header.ticks_per_second = anim.get_ticks_per_second();
            copy_name(clip_header.name, anim.get_name(), 512);
            file.write(reinterpret_cast<const char*>(&clip_header), sizeof(AnimationClipCacheHeader));

            //write each key frame fuckkkkkk
            for (auto& key_frame : key_frames)
            {
                KeyframeCacheHeader key_frame_header;
                copy_name(key_frame_header.name, key_frame.first);
                key_frame_header.position_count = key_frame.second.get_positions().size();
                key_frame_header.rotation_count = key_frame.second.get_rotations().size();
                key_frame_header.scale_count = key_frame.second.get_scales().size();
                file.write(reinterpret_cast<const char*>(&key_frame_header), sizeof(KeyframeCacheHeader));

                file.write(reinterpret_cast<const char*>(&key_frame.second.get_positions()), sizeof(AnimationKeyPosition) * key_frame.second.get_positions().size());
                file.write(reinterpret_cast<const char*>(&key_frame.second.get_rotations()), sizeof(AnimationKeyRotation) * key_frame.second.get_rotations().size());
                file.write(reinterpret_cast<const char*>(&key_frame.second.get_scales()), sizeof(AnimationKeyScale) * key_frame.second.get_scales().size());
            }
        }

        file.close();
        export_material_textures(skinned_model_data.materials, skinned_model_data.name);
    }

    ModelData import_model(const char *path)
    {
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open())
        {
            LOG_ERROR("error importing model at %s", path);
            return {};
        }

        ModelCacheHeader model_header;
        file.read(reinterpret_cast<char *>(&model_header), sizeof(model_header));

        ModelData result_data;
        result_data.name = model_header.name;
        result_data.aabb_max = model_header.aabb_max;
        result_data.aabb_min = model_header.aabb_min;
        result_data.meshes.resize(model_header.mesh_count);
        result_data.materials.resize(model_header.material_count);

        for (uint32_t i = 0; i < model_header.mesh_count; i++)
        {
            MeshData &data = result_data.meshes[i];

            MeshCacheHeader mesh_header;
            file.read(reinterpret_cast<char *>(&mesh_header), sizeof(MeshCacheHeader));

            data.name = mesh_header.name;
            data.aabb_max = mesh_header.aabb_max;
            data.aabb_min = mesh_header.aabb_min;
            data.vertices.resize(mesh_header.vertex_count);
            data.indices.resize(mesh_header.index_count);
            data.material_index = mesh_header.material_index;
            data.inverse_bind_pose = mesh_header.inverse_bind_pose;
            file.read(reinterpret_cast<char *>(data.vertices.data()), mesh_header.vertex_count * sizeof(Vertex));
            file.read(reinterpret_cast<char *>(data.indices.data()), mesh_header.index_count * sizeof(uint32_t));
        }

        for (uint32_t i = 0; i < model_header.material_count; i++)
        {
            Material &material = result_data.materials[i];

            MaterialCacheHeader material_header;
            file.read(reinterpret_cast<char *>(&material_header), sizeof(MaterialCacheHeader));

            material.albedo.set_path(std::string(RESOURCES_PATH) + material_header.albedo_path);
            material.ao.set_path(std::string(RESOURCES_PATH) + material_header.ao_path);
            material.emission.set_path(std::string(RESOURCES_PATH) + material_header.emission_path);
            material.metallic.set_path(std::string(RESOURCES_PATH) + material_header.metallic_path);
            material.roughness.set_path(std::string(RESOURCES_PATH) + material_header.roughness_path);
            material.normal.set_path(std::string(RESOURCES_PATH)  + material_header.normal_path);
            material.roughness_override = material_header.roughness_override;
            material.metallic_override = material_header.metallic_override;
        }
        file.close();

        return result_data;
    }

    SkinnedModelData import_skinned_model(const char *path)
    {
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open())
        {
            LOG_ERROR("error importing model at %s", path);
            return {};
        }

        //read header
        SkinnedModelCacheHeader skinned_model_header;
        file.read(reinterpret_cast<char*>(&skinned_model_header), sizeof(skinned_model_header));

        SkinnedModelData result_data;
        result_data.name = skinned_model_header.name;
        result_data.aabb_max = skinned_model_header.aabb_max;
        result_data.aabb_min = skinned_model_header.aabb_min;
        result_data.meshes.resize(skinned_model_header.mesh_count);
        result_data.materials.resize(skinned_model_header.material_count);
        result_data.animations.resize(skinned_model_header.animation_count);
        //pre-allocate bone vector
        std::vector<Bone> bones (skinned_model_header.bone_count);
        std::unordered_map<std::string, int> bone_mapping (skinned_model_header.bone_count);

        //read meshes
        for (size_t i = 0; i < skinned_model_header.mesh_count; i++)
        {
            MeshCacheHeader mesh_header;
            file.read(reinterpret_cast<char*>(&mesh_header), sizeof(mesh_header));

            auto& mesh = result_data.meshes[i];
            mesh.aabb_max = mesh_header.aabb_max;
            mesh.aabb_min = mesh_header.aabb_min;
            mesh.material_index = mesh_header.material_index;
            mesh.name = mesh_header.name;
            mesh.indices.resize(mesh_header.index_count);
            mesh.vertices.resize(mesh_header.vertex_count);

            file.read(reinterpret_cast<char*>(mesh.vertices.data()), mesh_header.vertex_count * sizeof(WeightedVertex));
            file.read(reinterpret_cast<char*>(mesh.indices.data()), mesh_header.index_count * sizeof(uint32_t));
        }

        //read materials
        for (size_t i = 0; i < skinned_model_header.material_count; i++)
        {
            Material &material = result_data.materials[i];

            MaterialCacheHeader material_header;
            file.read(reinterpret_cast<char *>(&material_header), sizeof(MaterialCacheHeader));

            material.albedo.set_path(std::string(RESOURCES_PATH) + material_header.albedo_path);
            material.ao.set_path(std::string(RESOURCES_PATH) + material_header.ao_path);
            material.emission.set_path(std::string(RESOURCES_PATH) + material_header.emission_path);
            material.metallic.set_path(std::string(RESOURCES_PATH) + material_header.metallic_path);
            material.roughness.set_path(std::string(RESOURCES_PATH) + material_header.roughness_path);
            material.normal.set_path(std::string(RESOURCES_PATH)  + material_header.normal_path);
            material.roughness_override = material_header.roughness_override;
            material.metallic_override = material_header.metallic_override;
        }


        //read skeleton bones
        for (size_t i = 0; i < skinned_model_header.bone_count; i++)
        {
            BoneHeader bone_header;
            file.read(reinterpret_cast<char*>(&bone_header), sizeof(BoneHeader));

            auto& bone = bones[i];
            bone.inverse_bind_pose = bone_header.inverse_bind_pose;
            bone.local_bind_transform = bone_header.local_bind_transform;
            bone.name = bone_header.name;
            bone.parent_idx = bone_header.parent_idx;
        }

        //read skeleton hiearchy
        for (size_t i = 0; i < skinned_model_header.bone_count; i++)
        {
            BoneMappingHeader bone_mapping_header;
            file.read(reinterpret_cast<char*>(&bone_mapping_header), sizeof(BoneMappingHeader));
            bone_mapping.insert(std::make_pair(bone_mapping_header.name, bone_mapping_header.index));
        }

        //assign skeleton
        result_data.skeleton = Skeleton(bones, bone_mapping);


        //read animations
        for (size_t i = 0; i < skinned_model_header.animation_count; i++)
        {
            //read in the clip info
            AnimationClipCacheHeader clip_header;
            file.read(reinterpret_cast<char*>(&clip_header), sizeof(AnimationClipCacheHeader));

            std::unordered_map<std::string, BoneAnimationData> key_frames;

            //read in each key-frame
            for (size_t j = 0; j < clip_header.channel_count; j++)
            {
                KeyframeCacheHeader key_frame_header;
                file.read(reinterpret_cast<char*>(&key_frame_header), sizeof(key_frame_header));
                std::string name = key_frame_header.name;
                std::vector<AnimationKeyPosition> positions(key_frame_header.position_count);
                std::vector<AnimationKeyRotation> rotations(key_frame_header.rotation_count);
                std::vector<AnimationKeyScale> scales(key_frame_header.scale_count);

                file.read(reinterpret_cast<char*>(positions.data()), sizeof(AnimationKeyPosition) * key_frame_header.position_count);
                file.read(reinterpret_cast<char*>(rotations.data()), sizeof(AnimationKeyRotation) * key_frame_header.rotation_count);
                file.read(reinterpret_cast<char*>(scales.data()), sizeof(AnimationKeyScale) * key_frame_header.scale_count);

                BoneAnimationData bone_animation_data(name, positions, rotations, scales);
                key_frames.insert(std::make_pair(name, bone_animation_data));
            }

            result_data.animations[i] = AnimationClip(clip_header.name, key_frames, clip_header.duration, clip_header.ticks_per_second);
        }
        file.close();
        return result_data;
    }
}
