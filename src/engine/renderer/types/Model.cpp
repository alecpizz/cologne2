//
// Created by alecpizz on 3/1/2025.
//

#include "Model.h"

#include <engine/physics/Physics.h>

#include "Mesh.h"
#include "Texture.h"
#include "Vertex.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/Importer.hpp"


namespace cologne
{
    Model::Model(const char *path, bool flip_textures)
    {
        _path = std::string(path);
        load_model();
    }


    Model::~Model()
    {
        _meshes.clear();
        LOG_INFO("YOU NEED TO CLEAN UP YOUR MESHES");
        _materials.clear();
    }

    void Model::load_model()
    {
        Assimp::Importer importer;
        const aiScene *scene = importer.ReadFile(_path, aiProcess_Triangulate |
                                                        aiProcess_FlipUVs | aiProcess_GenSmoothNormals |
                                                        aiProcess_CalcTangentSpace |
                                                        aiProcess_RemoveRedundantMaterials |
                                                        aiProcess_GenBoundingBoxes);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            LOG_ERROR("ASSIMP ERROR: %s", importer.GetErrorString());
            return;
        }
        process_node(scene->mRootNode, scene);
        load_materials(scene);
        importer.FreeScene();
        LOG_INFO("Loaded model with %d meshes", _meshes.size());
    }

    void Model::load_materials(const aiScene *scene)
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

            _materials.push_back(mat);
        }
    }

    void Model::process_node(const aiNode *node, const aiScene *scene)
    {
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            // the node object only contains indices to index the actual objects in the scene.
            // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];

            _meshes.push_back(process_mesh(mesh));
        }
        // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            process_node(node->mChildren[i], scene);
        }
    }

    Mesh Model::process_mesh(aiMesh *mesh)
    {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
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
            vertices.push_back(vertex);
        }

        for (size_t i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (size_t j = 0; j < face.mNumIndices; j++)
            {
                indices.push_back(face.mIndices[j]);
            }
        }

        _bounds.union_aabb(AABB(glm::vec3(mesh->mAABB.mMin.x, mesh->mAABB.mMin.y, mesh->mAABB.mMin.z),
                                       glm::vec3(mesh->mAABB.mMax.x, mesh->mAABB.mMax.y, mesh->mAABB.mMax.z)));
        return Mesh(vertices, indices, mesh->mMaterialIndex);
    }

    Transform &Model::get_transform()
    {
        return _transform;
    }

    AABB Model::get_aabb() const
    {
        return _bounds;
    }

    const char *Model::get_path() const
    {
        return _path.c_str();
    }

    Material *Model::get_materials()
    {
        return _materials.data();
    }


    Mesh *Model::get_meshes()
    {
        return _meshes.data();
    }

    uint64_t Model::get_num_meshes() const
    {
        return _meshes.size();
    }


    void Model::set_active(bool active)
    {
        _active = active;
    }

    void Model::set_aabb(AABB aabb)
    {
        _bounds = aabb;
    }

    bool Model::get_active() const
    {
        return _active;
    }

    bool Model::get_gi_only() const
    {
        return _gi_only;
    }

    bool Model::get_cast_shadows() const
    {
        return _cast_shadows;
    }

    void Model::set_cast_shadows(bool b)
    {
        _cast_shadows = b;
    }

    void Model::set_gi_only(bool b)
    {
        _gi_only = b;
    }
}
