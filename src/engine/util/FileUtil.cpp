//
// Created by alecp on 5/10/2025.
//

#include "FileUtil.h"

#include <engine/renderer/types/SkinnedModel.h>

#include "Util.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/Importer.hpp"
#include <filesystem>

namespace cologne::FileUtil
{
    void process_node(std::vector<MeshData> &meshes, const aiNode *node, const aiScene *scene);

    void process_skinned_node(std::vector<SkinnedMeshData> &meshes, std::unordered_map<std::string, BoneInfo> &bone_map,
                              int &bone_counter, const aiNode *node, const aiScene *scene);

    SkinnedMeshData process_skinned_mesh(std::unordered_map<std::string, BoneInfo> &bone_map, int &bone_counter,
                                         aiMesh *mesh);

    MeshData process_mesh(aiMesh *mesh);

    void reset_vertex(WeightedVertex &vertex);

    void set_vertex_data(WeightedVertex &vertex, int boneID, float weight);

    void extract_bone_weight_for_vertices(std::vector<WeightedVertex> &vertices,
                                          std::unordered_map<std::string, BoneInfo> &bone_map, int &bone_counter,
                                          const aiMesh *mesh);

    Animation import_animation(SkinnedModel &model);

    void process_materials(std::vector<Material> &mats, const aiScene *scene);

    void process_node(std::vector<MeshData> &meshes, const aiNode *node, const aiScene *scene)
    {
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            // the node object only contains indices to index the actual objects in the scene.
            // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];

            meshes.push_back(process_mesh(mesh));
        }
        // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            process_node(meshes, node->mChildren[i], scene);
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
            } else
            {
                vertex.uv = glm::vec2(0.0f, 0.0f);
            }
            if (mesh->mTangents)
            {
                vector.x = mesh->mTangents[i].x;
                vector.y = mesh->mTangents[i].y;
                vector.z = mesh->mTangents[i].z;
                vertex.tangent = vector;
            } else
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

    void extract_bone_weight_for_vertices(std::vector<WeightedVertex> &vertices,
                                          std::unordered_map<std::string, BoneInfo> &bone_map, int &bone_counter,
                                          const aiMesh *mesh)
    {
        for (size_t bone_idx = 0; bone_idx < mesh->mNumBones; bone_idx++)
        {
            int bone_id = -1;
            std::string bone_name = mesh->mBones[bone_idx]->mName.C_Str();
            if (!bone_map.contains(bone_name))
            {
                BoneInfo info{};
                info.id = bone_counter;
                info.offset = Util::ai_mat4_to_glm_mat4(mesh->mBones[bone_idx]->mOffsetMatrix);
                bone_map.insert(std::make_pair(bone_name, info));
                bone_id = bone_counter;
                bone_counter++;
            } else
            {
                bone_id = bone_map[bone_name].id;
            }
            assert(bone_id != -1);
            const auto weights = mesh->mBones[bone_idx]->mWeights;
            const int num_weights = mesh->mBones[bone_idx]->mNumWeights;

            for (size_t weight_idx = 0; weight_idx < num_weights; weight_idx++)
            {
                int vertex_id = weights[weight_idx].mVertexId;
                float weight = weights[weight_idx].mWeight;
                if (weight > 1.0f)
                {
                    LOG_INFO("FAT %f", weight);
                }
                set_vertex_data(vertices[vertex_id], bone_id, weight);
            }
        }
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
                    mat.albedo = Texture(reinterpret_cast<unsigned char *>(texture->pcData), texture->mWidth,
                                         texture->mHeight);
                }
            }

            aiString normalPath;
            if (material->GetTexture(aiTextureType_NORMAL_CAMERA, 0, &normalPath) == aiReturn_SUCCESS || material->
                GetTexture(aiTextureType_NORMALS, 0, &normalPath) == aiReturn_SUCCESS)
            {
                if (auto texture = scene->GetEmbeddedTexture(normalPath.C_Str()); texture != nullptr)
                {
                    mat.normal = Texture(reinterpret_cast<unsigned char *>(texture->pcData), texture->mWidth,
                                         texture->mHeight);
                }
            }

            aiString ambient_path;
            if (material->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &ambient_path) == aiReturn_SUCCESS ||
                material->GetTexture(aiTextureType_AMBIENT, 0, &ambient_path) == aiReturn_SUCCESS)
            {
                if (auto texture = scene->GetEmbeddedTexture(ambient_path.C_Str()); texture != nullptr)
                {
                    mat.ao = Texture(reinterpret_cast<unsigned char *>(texture->pcData), texture->mWidth,
                                     texture->mHeight);
                }
            }

            aiString roughness_path;
            if (material->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &roughness_path) == aiReturn_SUCCESS)
            {
                if (auto texture = scene->GetEmbeddedTexture(roughness_path.C_Str()); texture != nullptr)
                {
                    mat.roughness = Texture(reinterpret_cast<unsigned char *>(texture->pcData), texture->mWidth,
                                            texture->mHeight);
                }
            }

            aiString metallic_path;
            if (material->GetTexture(aiTextureType_METALNESS, 0, &metallic_path) == aiReturn_SUCCESS)
            {
                if (auto texture = scene->GetEmbeddedTexture(metallic_path.C_Str()); texture != nullptr)
                {
                    mat.metallic = Texture(reinterpret_cast<unsigned char *>(texture->pcData), texture->mWidth,
                                           texture->mHeight);
                }
            }

            aiString emission_path;
            if (material->GetTexture(aiTextureType_EMISSION_COLOR, 0, &emission_path) == aiReturn_SUCCESS ||
                material->GetTexture(aiTextureType_EMISSIVE, 0, &emission_path) == aiReturn_SUCCESS)
            {
                if (auto texture = scene->GetEmbeddedTexture(emission_path.C_Str()); texture != nullptr)
                {
                    mat.emission = Texture(reinterpret_cast<unsigned char *>(texture->pcData), texture->mWidth,
                                           texture->mHeight);
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
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            LOG_ERROR("ASSIMP ERROR: %s", importer.GetErrorString());
            return {};
        }
        ModelData result_data;
        process_node(result_data.meshes, scene->mRootNode, scene);
        process_materials(result_data.materials, scene);
        for (auto &mesh: result_data.meshes)
        {
            result_data.aabb_max = glm::max(result_data.aabb_max, mesh.aabb_max);
            result_data.aabb_min = glm::min(result_data.aabb_min, mesh.aabb_min);
        }
        result_data.name = get_file_name(path);
        return result_data;
    }


    SkinnedModelData import_skinned_model(const std::string &path)
    {
        SkinnedModelData result_data;
        Assimp::Importer importer;
        const aiScene *scene = importer.ReadFile(
            path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals |
                  aiProcess_RemoveRedundantMaterials | aiProcess_LimitBoneWeights);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            LOG_ERROR("ASSIMP ERROR: %s", importer.GetErrorString());
            return result_data;
        }
        process_skinned_node(result_data.meshes, result_data.bone_map, result_data.bone_count, scene->mRootNode, scene);
        process_materials(result_data.materials, scene);
        result_data.name = get_file_name(path);
        for (auto &mesh: result_data.meshes)
        {
            result_data.aabb_max = glm::max(result_data.aabb_max, mesh.aabb_max);
            result_data.aabb_min = glm::min(result_data.aabb_min, mesh.aabb_min);
        }

        for (size_t i = 0; i < scene->mNumAnimations; i++)
        {
            result_data.animations.emplace_back(scene->mAnimations[i], scene,
                result_data.bone_map, result_data.bone_count);
        }
        importer.FreeScene();
        LOG_INFO("Loaded skinned model with %d meshes:  %d bones", result_data.meshes.size(), result_data.bone_count);
        return result_data;
    }

    std::string get_file_name(const std::string &path)
    {
        size_t pos = path.find_last_of("/\\");
        std::string filename = (pos == std::string::npos) ? path : path.substr(pos + 1);
        pos = filename.find_last_of('.');
        return (pos != std::string::npos) ? filename.substr(0, pos) : filename;
    }

    std::string get_full_path(const std::filesystem::directory_entry &entry)
    {
        return entry.path().string();
    }

    std::string get_file_extension(const std::filesystem::directory_entry &entry)
    {
        return entry.path().extension().string().substr(1);
    }

    std::string get_file_name_without_extension(const std::filesystem::directory_entry &entry)
    {
        return entry.path().stem().string();
    }

    std::vector<FileInfo> iterate_directory(const std::string &directory, const std::vector<std::string> &extensions)
    {
        std::vector<FileInfo> result;
        if (!std::filesystem::exists(directory))
        {
            return result;
        }

        auto iter = std::filesystem::directory_iterator(directory);
        for (const auto &entry: iter)
        {
            if (!std::filesystem::is_regular_file(entry)) continue;

            FileInfo info = {
                get_full_path(entry), get_file_name_without_extension(entry), get_file_extension(entry), directory
            };

            if (extensions.empty() || std::find(extensions.begin(), extensions.end(), info.ext) != extensions.end())
            {
                result.push_back(info);
            }
        }
        return result;
    }

    void process_skinned_node(std::vector<SkinnedMeshData> &meshes, std::unordered_map<std::string, BoneInfo> &bone_map,
                              int &bone_counter, const aiNode *node, const aiScene *scene)
    {
        for (size_t i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(process_skinned_mesh(bone_map, bone_counter, mesh));
        }
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            process_skinned_node(meshes, bone_map, bone_counter, node->mChildren[i], scene);
        }
    }

    SkinnedMeshData process_skinned_mesh(std::unordered_map<std::string, BoneInfo> &bone_map, int &bone_counter,
                                         aiMesh *mesh)
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
            } else
            {
                vertex.uv = glm::vec2(0.0f);
            }
            if (mesh->mTangents)
            {
                vertex.tangent = Util::ai_vec3_to_glm_vec3(mesh->mTangents[i]);
            } else
            {
                vertex.tangent = glm::vec3(0.0f);
            }
            result_data.vertices.push_back(vertex);
        }

        for (size_t i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (size_t j = 0; j < face.mNumIndices; j++)
            {
                result_data.indices.push_back(face.mIndices[j]);
            }
        }

        extract_bone_weight_for_vertices(result_data.vertices, bone_map, bone_counter, mesh);
        result_data.name = mesh->mName.C_Str();
        result_data.material_index = mesh->mMaterialIndex;
        result_data.aabb_max = glm::max(result_data.aabb_max, Util::ai_vec3_to_glm_vec3(mesh->mAABB.mMax));
        result_data.aabb_min = glm::min(result_data.aabb_min, Util::ai_vec3_to_glm_vec3(mesh->mAABB.mMin));
        return result_data;
    }
}
