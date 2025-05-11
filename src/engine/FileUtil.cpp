//
// Created by alecp on 5/10/2025.
//

#include "FileUtil.h"
#include <assimp/Importer.hpp>
#include <assimp/Scene.h>
#include <assimp/PostProcess.h>
#include "renderer/Material.h"
#include "renderer/Texture.h"

namespace cologne::FileUtil
{
    void load_material(const aiScene *scene, const aiMaterial *material, Material &mat)
    {
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
    }

    ModelData import_model(const std::string &path)
    {
        ModelData model_data;
        Assimp::Importer importer;
        importer.SetPropertyBool(AI_CONFIG_PP_FD_REMOVE, true);
        const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate
                                                       | aiProcess_JoinIdenticalVertices |
                                                       aiProcess_ImproveCacheLocality
                                                       | aiProcess_RemoveRedundantMaterials | aiProcess_FlipUVs |
                                                       aiProcess_CalcTangentSpace);
        if (!scene)
        {
            LOG_ERROR("Failed to load model at %s ", path.c_str());
            LOG_ERROR("ASSIMP ERROR: %s", importer.GetErrorString());
            return model_data;
        }
        model_data.name = get_file_name(path);
        model_data.mesh_count = scene->mNumMeshes;
        model_data.material_count = scene->mNumMaterials;
        model_data.meshes.resize(model_data.mesh_count);
        model_data.materials.resize(model_data.material_count);

        for (size_t i = 0; i < model_data.meshes.size(); i++)
        {
            auto &meshData = model_data.meshes[i];
            meshData.vertex_count = scene->mMeshes[i]->mNumVertices;
            meshData.index_count = scene->mMeshes[i]->mNumFaces * 3;
            meshData.name = scene->mMeshes[i]->mName.C_Str();
            meshData.vertices.resize(meshData.vertex_count);
            meshData.indices.resize(meshData.index_count);
        }

        for (size_t i = 0; i < model_data.material_count; i++)
        {
            auto &material = model_data.materials[i];
            load_material(scene, scene->mMaterials[i], material);
        }

        for (size_t i = 0; i < model_data.meshes.size(); i++)
        {
            auto &meshData = model_data.meshes[i];
            const aiMesh *mesh = scene->mMeshes[i];

            for (uint32_t j = 0; j < meshData.vertex_count; j++)
            {
                meshData.vertices[j] = Vertex(
                    glm::vec3(mesh->mVertices[j].x, mesh->mVertices[j].y, mesh->mVertices[j].z),
                    glm::vec3(mesh->mNormals[j].x, mesh->mNormals[j].y, mesh->mNormals[j].z),
                    mesh->HasTextureCoords(0)
                        ? glm::vec2(mesh->mTextureCoords[0][j].x, mesh->mTextureCoords[0][j].y)
                        : glm::vec2(0.0f),
                    mesh->HasTangentsAndBitangents()
                        ? glm::vec3(mesh->mTangents[j].x, mesh->mTangents[j].y, mesh->mTangents[j].z)
                        : glm::vec3(0.0f));
                meshData.aabbMin = glm::min(meshData.vertices[j].position, meshData.aabbMin);
                meshData.aabbMin = glm::max(meshData.vertices[j].position, meshData.aabbMax);
            }

            for (uint32_t j = 0; j < mesh->mNumFaces; j++)
            {
                const aiFace &face = mesh->mFaces[j];
                uint32_t base_index = j * 3;
                meshData.indices[base_index] = face.mIndices[0];
                meshData.indices[base_index + 1] = face.mIndices[1];
                meshData.indices[base_index + 2] = face.mIndices[2];
            }

            for (Vertex &vert: meshData.vertices)
            {
                vert.normal = glm::normalize(vert.normal);
            }

            model_data.aabbMin = glm::min(model_data.aabbMin, meshData.aabbMin);
            model_data.aabbMax = glm::max(model_data.aabbMax, meshData.aabbMax);
        }
        importer.FreeScene();
        return model_data;
    }

    std::string get_file_name(const std::string &path)
    {
        size_t pos = path.find_last_of("/\\");
        std::string filename = (pos == std::string::npos) ? path : path.substr(pos + 1);
        pos = filename.find_last_of('.');
        return (pos != std::string::npos) ? filename.substr(0, pos) : filename;
    }
}
