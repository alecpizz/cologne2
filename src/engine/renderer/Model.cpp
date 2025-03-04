//
// Created by alecpizz on 3/1/2025.
//

#include "Model.h"
#include "gpch.h"
#include "Mesh.h"
#include "Vertex.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/Importer.hpp"


namespace goon
{
    struct Model::Impl
    {
        std::vector<Mesh> meshes;
        std::string directory;
        std::vector<Texture> textures;
        void init(const std::string& path)
        {
            directory = path;
            Assimp::Importer importer;
            const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate |
                aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);
            if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
            {
                LOG_ERROR("ASSIMP ERROR: %s", importer.GetErrorString());
                return;
            }
            directory = path.substr(0, path.find_last_of('/'));
            process_node(scene->mRootNode, scene);
        }

        void process_node(aiNode *node, const aiScene *scene)
        {
            for (size_t i = 0; i < scene->mNumMeshes; i++)
            {
                aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
                meshes.push_back(process_mesh(mesh, scene));
            }
        }

        Mesh process_mesh(aiMesh *mesh, const aiScene *scene)
        {
            //TODO: process da meshy
            std::vector<Vertex> vertices;
            std::vector<uint32_t> indices;
            std::vector<Texture> textures;
            for (size_t i = 0; i < mesh->mNumVertices; i++)
            {
                Vertex vertex;
            }

            if (mesh->mMaterialIndex != 0)
            {

            }
            return Mesh(vertices)
        }
    };

    Model::Model(const char *path)
    {
        _impl = new Impl();
        _impl->init(path);
    }

    Model::~Model()
    {
        delete _impl;
    }

    Mesh * Model::get_meshes() const
    {
        return _impl->meshes.data();
    }

    uint32_t Model::get_num_meshes() const
    {
        return _impl->meshes.size();
    }
}
