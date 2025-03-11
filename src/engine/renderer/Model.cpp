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
    static TextureType translate_texture_type(aiTextureType type)
    {
        switch (type)
        {
            case aiTextureType_DIFFUSE: return TextureType::DIFFUSE;
            case aiTextureType_SPECULAR: return TextureType::SPECULAR;
            default: return TextureType::NONE;
        }
    }


    std::vector<Texture> load_material_textures(const aiMaterial *mat, const aiTextureType type,
                                                const std::string &directory)
    {
        std::vector<Texture> textures;
        for (uint32_t i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);
            bool skip = false;
            for (size_t j = 0; j < textures.size(); j++)
            {
                if (std::strcmp(textures[j].get_path(), str.C_Str()) == 0)
                {
                    textures.push_back(textures[j]);
                    skip = true;
                    break;
                }
            }
            if (!skip)
            {
                Texture texture(std::string(directory + '/' + str.C_Str()).c_str());
                texture.set_texture_type(translate_texture_type(type));
                textures.push_back(texture);
            }
        }
        return textures;
    }

    Mesh process_mesh(aiMesh *mesh, const aiScene *scene, const std::string &directory)
    {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        std::vector<Texture> textures;
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

        if (mesh->mMaterialIndex != 0)
        {
            aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
            std::vector<Texture> diffuse_maps = load_material_textures(material, aiTextureType_DIFFUSE, directory);
            std::vector<Texture> specular_maps = load_material_textures(material, aiTextureType_SPECULAR, directory);
            textures.insert(textures.end(), diffuse_maps.begin(), diffuse_maps.end());
            textures.insert(textures.end(), specular_maps.begin(), specular_maps.end());
        }

        if (indices.size() == 0)
        {
            LOG_INFO("No indicies found");
        }

        return {vertices.data(), vertices.size(),
                    indices.data(), indices.size(),
                    textures.data(), textures.size()};
    }


    void process_node(const aiNode *node, const aiScene *scene, std::vector<Mesh> &meshes, const std::string &directory)
    {
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            // the node object only contains indices to index the actual objects in the scene.
            // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
            Mesh m = process_mesh(mesh, scene, directory);
            meshes.push_back(m);
        }
        // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            process_node(node->mChildren[i], scene, meshes, directory);
        }
    }

    Model::Model(const char *path)
    {
        _directory = path;
        Assimp::Importer importer;
        const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate |
                                                       aiProcess_FlipUVs | aiProcess_GenSmoothNormals |
                                                       aiProcess_CalcTangentSpace);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            LOG_ERROR("ASSIMP ERROR: %s", importer.GetErrorString());
            return;
        }
        _directory = _directory.substr(0, _directory.find_last_of('/'));
        process_node(scene->mRootNode, scene, _meshes, _directory);
    }

    Model::~Model()
    {
    }


    Mesh *Model::get_meshes()
    {
        return _meshes.data();
    }

    uint64_t Model::get_num_meshes() const
    {
        return _meshes.size();
    }
}
