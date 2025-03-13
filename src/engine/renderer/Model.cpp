//
// Created by alecpizz on 3/1/2025.
//

#include "Model.h"
#include "gpch.h"
#include "Mesh.h"
#include "Texture.h"
#include "Vertex.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/Importer.hpp"


namespace goon
{
    struct Model::Impl
    {
        std::vector<Mesh> meshes = std::vector<Mesh>();
        std::string directory = "";
        std::unique_ptr<Texture> albedo = nullptr;
        std::unique_ptr<Texture> normal = nullptr;
        std::unique_ptr<Texture> ao = nullptr;
        std::unique_ptr<Texture> roughness = nullptr;
        std::unique_ptr<Texture> metallic = nullptr;

        void load_model()
        {
            Assimp::Importer importer;
            const aiScene *scene = importer.ReadFile(directory, aiProcess_Triangulate |
                                                                aiProcess_FlipUVs | aiProcess_GenSmoothNormals |
                                                                aiProcess_CalcTangentSpace);
            if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
            {
                LOG_ERROR("ASSIMP ERROR: %s", importer.GetErrorString());
                return;
            }
            process_node(scene->mRootNode, scene);
        }

        void process_node(const aiNode *node, const aiScene *scene)
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
                process_node(node->mChildren[i], scene);
            }
        }

        Mesh process_mesh(aiMesh *mesh)
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

            return Mesh(vertices.data(), static_cast<uint32_t>(vertices.size()), indices.data(),
                        static_cast<uint32_t>(indices.size()));
        }
    };


    Model::Model(const char *path)
    {
        _impl = new Impl();
        _transform = new Transform();
        auto path_str = std::string(path);
        // path_str = path_str.substr(0, path_str.find_last_of('/'));
        _impl->directory = path_str;
        _impl->load_model();
    }

    Model::~Model()
    {
        delete _impl;
    }

    Texture * Model::get_albedo() const
    {
        return _impl->albedo.get();
    }

    Texture * Model::get_normal() const
    {
        return _impl->normal.get();
    }

    Texture * Model::get_ao() const
    {
        return _impl->ao.get();
    }

    Texture * Model::get_roughness() const
    {
        return _impl->roughness.get();
    }

    Texture * Model::get_metallic() const
    {
        return _impl->metallic.get();
    }

    void Model::set_texture(const char *path, TextureType type)
    {
        switch (type)
        {
            case TextureType::ALBEDO:
                _impl->albedo = std::make_unique<Texture>(path);
                _impl->albedo.get()->set_texture_type(TextureType::ALBEDO);
                break;
            case TextureType::NORMAL:
                _impl->normal = std::make_unique<Texture>(path);
                _impl->normal.get()->set_texture_type(TextureType::NORMAL);
                break;
            case TextureType::AO:
                _impl->ao = std::make_unique<Texture>(path);
                break;
            case TextureType::ROUGHNESS:
                _impl->roughness = std::make_unique<Texture>(path);
                break;
            case TextureType::METALLIC:
                _impl->metallic = std::make_unique<Texture>(path);
                break;
            default: break;
        }
    }

    void Model::set_textures(const char *directory, const char *albedo, const char *normal, const char *ao,
        const char *roughness, const char *metallic)
    {
        std::string path = std::string(directory);
        set_texture(std::string(path + "/" + albedo).c_str(), TextureType::ALBEDO);
        set_texture(std::string(path + "/" + normal).c_str(), TextureType::NORMAL);
        set_texture(std::string(path + "/" + ao).c_str(), TextureType::AO);
        set_texture(std::string(path + "/" + roughness).c_str(), TextureType::ROUGHNESS);
        set_texture(std::string(path + "/" + metallic).c_str(), TextureType::METALLIC);
    }

    Transform * Model::get_transform() const
    {
        return _transform;
    }


    Mesh *Model::get_meshes()
    {
        return _impl->meshes.data();
    }

    uint64_t Model::get_num_meshes() const
    {
        return _impl->meshes.size();
    }
}
