//
// Created by alecpizz on 6/29/25.
//
#include "FileUtil.h"
#include <engine/renderer/types/SkinnedModel.h>

#include "Util.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/Importer.hpp"
#include <engine/animation/AnimationClip.h>


namespace cologne::FileUtil
{
    void process_node(std::vector<MeshData> &meshes, const aiNode *node, const aiScene *scene,
                      const aiMatrix4x4 &parentTransform);


    MeshData process_mesh(aiMesh *mesh);

    void reset_vertex(WeightedVertex &vertex);

    void set_vertex_data(WeightedVertex &vertex, int boneID, float weight);

    Animation import_animation(SkinnedModel &model);

    void process_materials(std::vector<Material> &mats, const aiScene *scene);

    void process_all_skinned_meshes(const aiScene *scene, std::vector<SkinnedMeshData> &out_meshes,
                                    const Skeleton &skeleton);

    SkinnedMeshData process_single_skinned_mesh(const aiMesh *mesh, const Skeleton &skeleton);

    void extract_bone_weights_for_mesh(std::vector<WeightedVertex> &vertices, const aiMesh *mesh,
                                       const Skeleton &skeleton);

    void process_node(std::vector<MeshData> &meshes, const aiNode *node, const aiScene *scene,
                      const aiMatrix4x4 &parentTransform)
    {
        auto world_transform = parentTransform * node->mTransformation;
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            // the node object only contains indices to index the actual objects in the scene.
            // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
            auto m = process_mesh(mesh);
            m.inverse_bind_pose = Util::ai_mat4_to_glm_mat4(world_transform);
            meshes.push_back(m);
        }
        // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            process_node(meshes, node->mChildren[i], scene, world_transform);
        }
    }

    MeshData process_mesh(aiMesh *mesh)
    {
        MeshData result;
        for (size_t i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            glm::vec3 vector;
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.position = vector;
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.normal = vector;
            if (mesh->mTextureCoords[0])
            {
                glm::vec2 vec;
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.uv = vec;
            }
            else
            {
                vertex.uv = glm::vec2(0.0f, 0.0f);
            }
            if (mesh->mTangents)
            {
                vector.x = mesh->mTangents[i].x;
                vector.y = mesh->mTangents[i].y;
                vector.z = mesh->mTangents[i].z;
                vertex.tangent = vector;
            }
            else
            {
                vertex.tangent = glm::vec3(0.0f);
            }
            result.vertices.push_back(vertex);
        }

        for (size_t i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (size_t j = 0; j < face.mNumIndices; j++)
            {
                result.indices.push_back(face.mIndices[j]);
            }
        }
        result.aabb_min = glm::min(result.aabb_min, Util::ai_vec3_to_glm_vec3(mesh->mAABB.mMin));
        result.aabb_max = glm::max(result.aabb_max, Util::ai_vec3_to_glm_vec3(mesh->mAABB.mMax));
        result.name = mesh->mName.C_Str();
        result.material_index = mesh->mMaterialIndex;
        return result;
    }

    void reset_vertex(WeightedVertex &vertex)
    {
        for (int i = 0; i < 4; i++)
        {
            vertex.boneID[i] = -1;
            vertex.weight[i] = 0.0f;
        }
    }

    void set_vertex_data(WeightedVertex &vertex, int boneID, float weight)
    {
        for (int i = 0; i < 4; i++)
        {
            if (vertex.boneID[i] < 0)
            {
                vertex.weight[i] = weight;
                vertex.boneID[i] = boneID;
                break;
            }
        }
    }


    Texture create_orm_texture(const Texture &occlusion, const Texture &roughness, const Texture &metallic)
    {
        if (!roughness.contains_data())
        {
            LOG_ERROR("roughness!");
            return {};
        }
        uint32_t width = roughness.get_width();
        uint32_t height = roughness.get_height();

        bool has_ao = occlusion.contains_data();
        bool has_metal = metallic.contains_data();
        const auto *occ_data = occlusion.get_raw_data().data();
        const auto *rough_data = roughness.get_raw_data().data();
        const auto *metal_data = metallic.get_raw_data().data();

        const auto occ_channels = has_ao ? occlusion.get_channels() : 1;
        const auto rough_channels = roughness.get_channels();
        const auto metal_channels = has_metal ? metallic.get_channels() : 1;

        const uint32_t dest_channels = 4;
        size_t total_pixels = width * height;
        std::vector<uint8_t> packed_data(total_pixels * dest_channels);

        for (size_t i = 0; i < total_pixels; i++)
        {
            const size_t occ_idx = i * occ_channels;
            const size_t rough_idx = i * rough_channels;
            const size_t metal_idx = i * metal_channels;

            const size_t dest_idx = i * dest_channels;

            packed_data[dest_idx + 0] = has_ao ? occ_data[occ_idx] : 255;
            packed_data[dest_idx + 1] = rough_data[rough_idx + (rough_channels > 1 ? 1 : 0)];
            if (has_metal)
            {
                packed_data[dest_idx + 2] = metal_data[metal_idx + (metal_channels > 2 ? 2 : 0)];
            }
            else
            {
                packed_data[dest_idx + 2] = 0;
            }

            packed_data[dest_idx + 3] = 255;
        }
        return Texture(packed_data.data(), width, height, dest_channels);
    }

    void process_materials(std::vector<Material> &mats, const aiScene *scene)
    {
        for (size_t i = 0; i < scene->mNumMaterials; i++)
        {
            Material mat;
            auto material = scene->mMaterials[i];

            aiString diffuse_path;
            if (material->GetTexture(aiTextureType_BASE_COLOR, 0, &diffuse_path) == aiReturn_SUCCESS)
            {
                if (auto texture = scene->GetEmbeddedTexture(diffuse_path.C_Str()); texture != nullptr)
                {
                    stbi_set_flip_vertically_on_load(false);
                    int new_width = 0, new_height = 0, new_channels = 0;
                    int size = texture->mHeight == 0 ? texture->mWidth : texture->mHeight * texture->mWidth;
                    stbi_uc *img_data =
                            stbi_load_from_memory(reinterpret_cast<unsigned char *>(texture->pcData),
                                                  size, &new_width, &new_height, &new_channels, 4);
                    mat.albedo = Texture(img_data, new_width, new_height, 4);
                    stbi_image_free(img_data);
                }
            }

            aiString normalPath;
            if (material->GetTexture(aiTextureType_NORMAL_CAMERA, 0, &normalPath) == aiReturn_SUCCESS || material->
                GetTexture(aiTextureType_NORMALS, 0, &normalPath) == aiReturn_SUCCESS)
            {
                if (auto texture = scene->GetEmbeddedTexture(normalPath.C_Str()); texture != nullptr)
                {
                    stbi_set_flip_vertically_on_load(false);
                    int new_width = 0, new_height = 0, new_channels = 0;
                    int size = texture->mHeight == 0 ? texture->mWidth : texture->mHeight * texture->mWidth;
                    stbi_uc *img_data =
                            stbi_load_from_memory(reinterpret_cast<unsigned char *>(texture->pcData),
                                                  size, &new_width, &new_height, &new_channels, 4);
                    mat.normal = Texture(img_data, new_width, new_height, 4);
                    stbi_image_free(img_data);
                }
            }

            aiString emission_path;
            if (material->GetTexture(aiTextureType_EMISSION_COLOR, 0, &emission_path) == aiReturn_SUCCESS ||
                material->GetTexture(aiTextureType_EMISSIVE, 0, &emission_path) == aiReturn_SUCCESS)
            {
                if (auto texture = scene->GetEmbeddedTexture(emission_path.C_Str()); texture != nullptr)
                {
                    stbi_set_flip_vertically_on_load(false);
                    int new_width = 0, new_height = 0, new_channels = 0;
                    int size = texture->mHeight == 0 ? texture->mWidth : texture->mHeight * texture->mWidth;
                    stbi_uc *img_data =
                            stbi_load_from_memory(reinterpret_cast<unsigned char *>(texture->pcData),
                                                  size, &new_width, &new_height, &new_channels, 4);
                    mat.emission = Texture(img_data, new_width, new_height, 4);
                    stbi_image_free(img_data);
                }
            }

            aiString misc_path;
            if (material->GetTexture(aiTextureType_UNKNOWN, 0, &misc_path) == aiReturn_SUCCESS)
            {
                if (auto texture = scene->GetEmbeddedTexture(misc_path.C_Str()); texture != nullptr)
                {
                    LOG_INFO("FOUND MISC TEXTURE AT: %s", texture->mFilename.C_Str());
                }
            }

            Texture occlusion;
            Texture metallic;
            Texture roughness;

            aiString roughness_path;
            if (material->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &roughness_path) == aiReturn_SUCCESS)
            {
                if (auto texture = scene->GetEmbeddedTexture(roughness_path.C_Str()); texture != nullptr)
                {
                    stbi_set_flip_vertically_on_load(false);
                    int new_width = 0, new_height = 0, new_channels = 0;
                    int size = texture->mHeight == 0 ? texture->mWidth : texture->mHeight * texture->mWidth;
                    stbi_uc *img_data =
                            stbi_load_from_memory(reinterpret_cast<unsigned char *>(texture->pcData),
                                                  size, &new_width, &new_height, &new_channels, 4);
                    roughness = Texture(img_data, new_width, new_height, 4);
                    stbi_image_free(img_data);
                }
            }

            aiString occlusion_path;
            if (material->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &occlusion_path) == aiReturn_SUCCESS ||
                material->GetTexture(aiTextureType_AMBIENT, 0, &occlusion_path) == aiReturn_SUCCESS)
            {
                if (strcmp(occlusion_path.C_Str(), roughness_path.C_Str()) == 0)
                {
                    occlusion = roughness;
                }
                else
                {
                    if (auto texture = scene->GetEmbeddedTexture(occlusion_path.C_Str()); texture != nullptr)
                    {
                        stbi_set_flip_vertically_on_load(false);
                        int new_width = 0, new_height = 0, new_channels = 0;
                        int size = texture->mHeight == 0 ? texture->mWidth : texture->mHeight * texture->mWidth;
                        stbi_uc *img_data =
                                stbi_load_from_memory(reinterpret_cast<unsigned char *>(texture->pcData),
                                                      size, &new_width, &new_height, &new_channels, 1);
                        occlusion = Texture(img_data, new_width, new_height, 1);
                        stbi_image_free(img_data);
                    }
                }
            }


            aiString metallic_path;
            if (material->GetTexture(aiTextureType_METALNESS, 0, &metallic_path) == aiReturn_SUCCESS)
            {
                if (strcmp(metallic_path.C_Str(), roughness_path.C_Str()) == 0)
                {
                    metallic = roughness;
                }
                else
                {
                    if (auto texture = scene->GetEmbeddedTexture(metallic_path.C_Str()); texture != nullptr)
                    {
                        stbi_set_flip_vertically_on_load(false);
                        int new_width = 0, new_height = 0, new_channels = 0;
                        int size = texture->mHeight == 0 ? texture->mWidth : texture->mHeight * texture->mWidth;
                        stbi_uc *img_data =
                                stbi_load_from_memory(reinterpret_cast<unsigned char *>(texture->pcData),
                                                      size, &new_width, &new_height, &new_channels, 4);
                        metallic = Texture(img_data, new_width, new_height, 4);
                        stbi_image_free(img_data);
                    }
                }
            }

            mat.orm = create_orm_texture(occlusion, roughness, metallic);


            ai_real roughness_factor;
            if (material->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness_factor) == aiReturn_SUCCESS)
            {
                mat.roughness_override = roughness_factor;
            }

            ai_real metallic_factor;
            if (material->Get(AI_MATKEY_METALLIC_FACTOR, metallic_factor) == aiReturn_SUCCESS)
            {
                mat.metallic_override = metallic_factor;
            }

            mats.push_back(mat);
        }
    }

    ModelData import_model(const std::string &path)
    {
        Assimp::Importer importer;
        const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate |
                                                       aiProcess_FlipUVs | aiProcess_GenSmoothNormals |
                                                       aiProcess_CalcTangentSpace |
                                                       aiProcess_RemoveRedundantMaterials |
                                                       aiProcess_GenBoundingBoxes);
        if (!scene || !scene->mRootNode)
        {
            LOG_ERROR("ASSIMP ERROR: %s", importer.GetErrorString());
            return {};
        }
        ModelData result_data;
        process_node(result_data.meshes, scene->mRootNode, scene, {});
        process_materials(result_data.materials, scene);
        result_data.name = get_file_name(path);
        for (auto &mesh: result_data.meshes)
        {
            glm::vec3 pos;
            glm::quat rot;
            glm::vec3 scale;
            Util::decompose_mat4(mesh.inverse_bind_pose, pos, rot, scale);
            result_data.aabb_max = glm::max(result_data.aabb_max, mesh.aabb_max);
            result_data.aabb_min = glm::min(result_data.aabb_min, mesh.aabb_min);
            mesh.name = result_data.name + "_" + mesh.name;
        }
        return result_data;
    }


    void build_skeleton_recursive(const aiNode *node, int parent_idx, Skeleton &out_skeleton,
                                  const std::unordered_map<std::string, const aiBone *> &allBones)
    {
        const std::string node_name = node->mName.C_Str();

        Bone node_rep;
        node_rep.name = node_name;
        node_rep.parent_idx = parent_idx;
        node_rep.local_bind_transform = Util::ai_mat4_to_glm_mat4(node->mTransformation);
        node_rep.inverse_bind_pose = glm::mat4(1.0f);

        if (const auto it = allBones.find(node_name); it != allBones.end())
        {
            const aiBone *boneData = it->second;
            node_rep.inverse_bind_pose = Util::ai_mat4_to_glm_mat4(boneData->mOffsetMatrix);
        }

        int new_idx = out_skeleton._bones.size();
        out_skeleton._bones.push_back(node_rep);
        out_skeleton._bone_name_to_index[node_name] = new_idx;

        for (unsigned int i = 0; i < node->mNumChildren; ++i)
        {
            build_skeleton_recursive(node->mChildren[i], new_idx, out_skeleton, allBones);
        }
    }

    void build_skeleton(const aiScene *scene, Skeleton &out_skeleton)
    {
        std::unordered_map<std::string, const aiBone *> allBones;
        for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
        {
            const aiMesh *mesh = scene->mMeshes[i];
            for (unsigned int j = 0; j < mesh->mNumBones; ++j)
            {
                aiBone *bone = mesh->mBones[j];
                if (!allBones.contains(bone->mName.C_Str()))
                {
                    allBones[bone->mName.C_Str()] = bone;
                }
            }
        }

        build_skeleton_recursive(scene->mRootNode, -1, out_skeleton, allBones);
    }


    SkinnedModelData import_skinned_model(const std::string &path)
    {
        SkinnedModelData result_data;
        Assimp::Importer importer;
        const aiScene *scene = importer.ReadFile(
            path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals |
                  aiProcess_RemoveRedundantMaterials | aiProcess_LimitBoneWeights | aiProcess_GenBoundingBoxes);
        if (!scene || !scene->mRootNode)
        {
            LOG_ERROR("ASSIMP ERROR: %s", importer.GetErrorString());
            return result_data;
        }

        build_skeleton(scene, result_data.skeleton);
        process_all_skinned_meshes(scene, result_data.meshes, result_data.skeleton);
        process_materials(result_data.materials, scene);
        result_data.name = get_file_name(path);
        for (auto &mesh: result_data.meshes)
        {
            result_data.aabb_max = glm::max(result_data.aabb_max, mesh.aabb_max);
            result_data.aabb_min = glm::min(result_data.aabb_min, mesh.aabb_min);
        }

        for (size_t i = 0; i < scene->mNumAnimations; i++)
        {
            result_data.animations.emplace_back(result_data.name, scene->mAnimations[i], scene);
        }
        importer.FreeScene();
        LOG_INFO("Loaded skinned model with %d meshes:  %d bones", result_data.meshes.size(),
                 result_data.skeleton.get_bone_count());
        return result_data;
    }

    void process_all_skinned_meshes(const aiScene *scene, std::vector<SkinnedMeshData> &out_meshes,
                                    const Skeleton &skeleton)
    {
        for (size_t i = 0; i < scene->mNumMeshes; i++)
        {
            const auto mesh = scene->mMeshes[i];
            out_meshes.emplace_back(process_single_skinned_mesh(mesh, skeleton));
        }
    }

    SkinnedMeshData process_single_skinned_mesh(const aiMesh *mesh, const Skeleton &skeleton)
    {
        SkinnedMeshData result_data;
        for (size_t i = 0; i < mesh->mNumVertices; i++)
        {
            WeightedVertex vertex;
            reset_vertex(vertex);
            vertex.position = Util::ai_vec3_to_glm_vec3(mesh->mVertices[i]);
            vertex.normal = Util::ai_vec3_to_glm_vec3(mesh->mNormals[i]);
            if (mesh->mTextureCoords[0])
            {
                vertex.uv = Util::ai_vec3_to_glm_vec3(mesh->mTextureCoords[0][i]);
            }
            vertex.tangent = Util::ai_vec3_to_glm_vec3(mesh->mTangents[i]);
            result_data.vertices.emplace_back(vertex);
        }

        for (size_t i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (size_t j = 0; j < face.mNumIndices; j++)
            {
                result_data.indices.emplace_back(face.mIndices[j]);
            }
        }

        extract_bone_weights_for_mesh(result_data.vertices, mesh, skeleton);

        result_data.name = mesh->mName.C_Str();
        result_data.material_index = mesh->mMaterialIndex;
        result_data.aabb_max = glm::max(result_data.aabb_max, Util::ai_vec3_to_glm_vec3(mesh->mAABB.mMax));
        result_data.aabb_min = glm::min(result_data.aabb_min, Util::ai_vec3_to_glm_vec3(mesh->mAABB.mMin));
        return result_data;
    }

    void extract_bone_weights_for_mesh(std::vector<WeightedVertex> &vertices, const aiMesh *mesh,
                                       const Skeleton &skeleton)
    {
        for (size_t bone_idx = 0; bone_idx < mesh->mNumBones; bone_idx++)
        {
            std::string bone_name = mesh->mBones[bone_idx]->mName.C_Str();

            const int bone_id = skeleton.find_bone_index(bone_name);

            if (bone_id == -1)
            {
                continue;
            }

            const auto weights = mesh->mBones[bone_idx]->mWeights;
            const int num_weights = mesh->mBones[bone_idx]->mNumWeights;

            for (size_t weight_idx = 0; weight_idx < num_weights; weight_idx++)
            {
                const int vertex_id = weights[weight_idx].mVertexId;
                const float weight = weights[weight_idx].mWeight;
                set_vertex_data(vertices[vertex_id], bone_id, weight);
            }
        }
    }
}
