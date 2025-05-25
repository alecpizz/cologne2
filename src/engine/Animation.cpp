#include "Animation.h"

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
//
// Created by alecpizz on 5/25/2025.
//
namespace cologne::Animation
{
    void process_node(aiNode *node, const aiScene *scene, SkinnedModelData &data);

    SkinnedMeshData process_mesh(aiMesh *mesh);


    SkinnedModelData load_model(const std::string &path)
    {
        SkinnedModelData result;
        Assimp::Importer importer;
        const aiScene *scene = importer.ReadFile(path.c_str(),
                                                 aiProcess_Triangulate | aiProcess_FlipUVs |
                                                 aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace |
                                                 aiProcess_RemoveRedundantMaterials);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            LOG_ERROR("ASSIMPF FAILED TO IMPORT FILE %s", path.c_str());
            LOG_ERROR("ASSIMP ERROR: %s", importer.GetErrorString());
            return result;
        }

        result.global_inverse_transform = assimp_to_glm_matrix(scene->mRootNode->mTransformation.Inverse());
        process_node(scene->mRootNode, scene, result);

        return result;
    }

    void process_node(aiNode *node, const aiScene *scene, SkinnedModelData &data)
    {
        for (size_t i = 0; i < node->mNumMeshes; i++)
        {
            const auto mesh = scene->mMeshes[node->mMeshes[i]];
            data.meshes.push_back(process_mesh(mesh));
        }

        for (size_t i = 0; i < node->mNumChildren; i++)
        {
            process_node(node->mChildren[i], scene, data);
        }
    }

    SkinnedMeshData process_mesh(aiMesh *mesh)
    {
        SkinnedMeshData result;
        result.vertices.resize(mesh->mNumVertices);
        for (size_t i = 0; i < mesh->mNumVertices; i++)
        {
            WeightedVertex vertex;
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
            vertex.weight = glm::vec4(0.0f);
            vertex.boneID = glm::ivec4(0);
            result.vertices.push_back(vertex);
        }

        for (size_t i = 0; i < mesh->mNumBones; i++)
        {

        }
    }


    glm::mat4 assimp_to_glm_matrix(const aiMatrix4x4 &from)
    {
        glm::mat4 to;
        to[0][0] = from.a1;
        to[1][0] = from.a2;
        to[2][0] = from.a3;
        to[3][0] = from.a4;
        to[0][1] = from.b1;
        to[1][1] = from.b2;
        to[2][1] = from.b3;
        to[3][1] = from.b4;
        to[0][2] = from.c1;
        to[1][2] = from.c2;
        to[2][2] = from.c3;
        to[3][2] = from.c4;
        to[0][3] = from.d1;
        to[1][3] = from.d2;
        to[2][3] = from.d3;
        to[3][3] = from.d4;
        return to;
    }
}
