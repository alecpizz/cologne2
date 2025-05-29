//
// Created by alecpizz on 5/25/2025.
//

#include "SkinnedModel.h"

#include <engine/Util.h>

#include "assimp/Importer.hpp"
#include "assimp/mesh.h"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

namespace cologne
{
    SkinnedMesh process_skinned_mesh(std::unordered_map<std::string, BoneInfo> &bone_map, int &bone_counter,
                                     aiMesh *mesh);

    void process_skinned_node(std::vector<SkinnedMesh> &meshes, std::unordered_map<std::string, BoneInfo> &bone_map,
                              int &bone_counter, const aiNode *node, const aiScene *scene);

    void reset_vertex(WeightedVertex &vertex);

    void set_vertex_data(WeightedVertex &vertex, int boneID, float weight);

    void extract_bone_weight_for_vertices(std::vector<WeightedVertex> &vertices,
                                          std::unordered_map<std::string, BoneInfo> &bone_map, int &bone_counter,
                                          const aiMesh *mesh);
    void load_materials(const aiScene* scene, std::vector<Material>& materials);

    SkinnedMesh process_skinned_mesh(std::unordered_map<std::string, BoneInfo> &bone_map, int &bone_counter,
                                     aiMesh *mesh)
    {
        std::vector<WeightedVertex> vertices;
        std::vector<uint32_t> indices;
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
            vertices.push_back(vertex);
        }

        extract_bone_weight_for_vertices(vertices, bone_map, bone_counter, mesh);

        for (size_t i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (size_t j = 0; j < face.mNumIndices; j++)
            {
                indices.push_back(face.mIndices[j]);
            }
        }
        return SkinnedMesh(vertices, indices, mesh->mMaterialIndex);
    }

    void process_skinned_node(std::vector<SkinnedMesh> &meshes, std::unordered_map<std::string, BoneInfo> &bone_map,
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

    SkinnedModel::SkinnedModel(const char *path, const char* name)
    {
        _name = name;
        Assimp::Importer importer;
        const aiScene *scene = importer.ReadFile(
            path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals |
                  aiProcess_RemoveRedundantMaterials);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            LOG_ERROR("ASSIMP ERROR: %s", importer.GetErrorString());
            return;
        }
        process_skinned_node(_meshes, _bone_info_map, _bone_count, scene->mRootNode, scene);
        load_materials(scene, _materials);
        importer.FreeScene();
        LOG_INFO("Loaded skinned model with %d meshes:", _meshes.size());
    }

    SkinnedModel::~SkinnedModel()
    {
    }

    Transform &SkinnedModel::get_transform()
    {
        return _transform;
    }

    AABB SkinnedModel::get_aabb() const
    {
        return _bounds;
    }

    Material *SkinnedModel::get_materials()
    {
        return _materials.data();
    }

    uint64_t SkinnedModel::get_num_materials() const
    {
        return _materials.size();
    }

    SkinnedMesh *SkinnedModel::get_meshes()
    {
        return _meshes.data();
    }

    uint64_t SkinnedModel::get_num_meshes() const
    {
        return _meshes.size();
    }

    void SkinnedModel::set_active(bool active)
    {
        _active = active;
    }

    void SkinnedModel::set_aabb(AABB aabb)
    {
        _bounds = aabb;
    }

    std::string & SkinnedModel::get_name()
    {
        return _name;
    }

    bool SkinnedModel::get_active() const
    {
        return _active;
    }

    void reset_vertex(WeightedVertex &vertex)
    {
        vertex.boneID = glm::ivec4(-1);
        vertex.weight = glm::vec4(0.0f);
    }

    void set_vertex_data(WeightedVertex &vertex, int boneID, float weight)
    {
        for (size_t i = 0; i < 4; i++)
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
            int bone_id;
            std::string bone_name = mesh->mBones[bone_idx]->mName.C_Str();
            if (!bone_map.contains(bone_name))
            {
                BoneInfo info;
                info.id = bone_counter;
                info.offset = Util::ai_mat4_to_glm_mat4(mesh->mBones[bone_idx]->mOffsetMatrix);
                bone_map[bone_name] = info;
                bone_id = bone_counter;
                bone_counter++;
            } else
            {
                bone_id = bone_map[bone_name].id;
            }
            auto weights = mesh->mBones[bone_idx]->mWeights;
            const int num_weights = mesh->mBones[bone_idx]->mNumWeights;
            for (size_t weight_idx = 0; weight_idx < num_weights; weight_idx++)
            {
                int vertex_id = weights[weight_idx].mVertexId;
                float weight = weights[weight_idx].mWeight;
                set_vertex_data(vertices[vertex_id], bone_id, weight);
            }
        }
    }

    void load_materials(const aiScene *scene, std::vector<Material>& materials)
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

                materials.push_back(mat);
            }
    }
}
