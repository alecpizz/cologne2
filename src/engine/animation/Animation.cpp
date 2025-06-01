#include "Animation.h"

#include "BoneAnimationData.h"
#include "../util/Util.h"

#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "../renderer/types/SkinnedModel.h"
#include "assimp/Importer.hpp"
//
// Created by alecpizz on 5/25/2025.
//
namespace cologne
{
    std::vector<Animation> Animation::get_animations(const std::string &path, SkinnedModel &model)
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, 0);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            LOG_ERROR("ASSIMP ERROR: %s", importer.GetErrorString());
            return std::vector<Animation>();
        }

        std::vector<Animation> result;
        for (size_t i = 0; i < scene->mNumAnimations; i++)
        {
            result.emplace_back(Animation(scene->mAnimations[i], scene, model));
        }
        return result;
    }

    Animation::Animation(const std::string &path, SkinnedModel &model)
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, 0);
        if (!scene || !scene->mRootNode)
        {
            LOG_ERROR("ASSIMP ERROR: %s", importer.GetErrorString());
            return;
        }

        auto animation = scene->mAnimations[0];
        _duration = animation->mDuration;
        _ticks_per_second = animation->mTicksPerSecond;
        read_bone_hierarchy(_root_node, scene->mRootNode);
        read_missing_bones(animation, model);
        LOG_INFO("Created an animation with %f duration %d tps", _duration, _ticks_per_second);
    }

    Animation::Animation(aiAnimation *animation, const aiScene* scene, SkinnedModel &model)
    {
        _duration = animation->mDuration;
        _ticks_per_second = animation->mTicksPerSecond;
        read_bone_hierarchy(_root_node, scene->mRootNode);
        read_missing_bones(animation, model);
    }

    BoneAnimationData * Animation::find_bone(const std::string &name)
    {
        if (_bone_data_map.contains(name))
        {
            return &_bone_data_map.find(name)->second;
        }
        return nullptr;
    }

    float Animation::get_ticks_per_second() const
    {
        return _ticks_per_second;
    }

    float Animation::get_duration() const
    {
        return _duration;
    }

    Node & Animation::get_root()
    {
        return _root_node;
    }

    void Animation::read_missing_bones(const aiAnimation *animation, SkinnedModel &model)
    {
        int size = animation->mNumChannels;

        auto& model_bone_info = model._bone_info_map;
        auto& count = model._bone_count;

        for (int i = 0; i < size; i++)
        {
            auto channel = animation->mChannels[i];
            std::string bone_name = channel->mNodeName.data;

            if (!model_bone_info.contains(bone_name))
            {
                LOG_INFO("MISSING BONE %s", channel->mNodeName.data);
                model_bone_info[bone_name].id = count;
                model_bone_info[bone_name].offset = glm::mat4(1.0f);
                count++;
            }
            _bone_data_map.insert(std::make_pair(bone_name,
                BoneAnimationData(bone_name, model_bone_info[bone_name].id, channel)));
        }

        _bone_info_map = model_bone_info;
    }

    void Animation::read_bone_hierarchy(Node &dest, const aiNode *src)
    {
        if (!src)
        {
            LOG_ERROR("NO BONE SOURCE!");
            dest.name = "empty_node_null";
            dest.transform = glm::mat4(1.0f);
            dest.children.clear();
            return;
        }
        dest.name = src->mName.data;
        dest.transform = Util::ai_mat4_to_glm_mat4(src->mTransformation);
        dest.children.clear();
        for (size_t i = 0; i < src->mNumChildren; i++)
        {
            Node node;
            read_bone_hierarchy(node, src->mChildren[i]);
            dest.children.push_back(node);
        }
    }


}
