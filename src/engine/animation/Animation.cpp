#include "Animation.h"

#include "BoneAnimationData.h"
#include "../util/Util.h"

#include "assimp/scene.h"
#include "../renderer/types/SkinnedModel.h"
//
// Created by alecpizz on 5/25/2025.
//
namespace cologne
{
    Animation::Animation(const std::string& base_name, const aiAnimation *animation, const aiScene *scene,
                         std::unordered_map<std::string, BoneInfo> &bone_map, int &bone_count)
    {
        _name = base_name + "_" + animation->mName.C_Str();
        _duration = static_cast<float>(animation->mDuration);
        _ticks_per_second = static_cast<int>(animation->mTicksPerSecond);
        read_bone_hierarchy(_root_node, scene->mRootNode);
        read_missing_bones(animation, bone_map, bone_count);
    }

    BoneAnimationData *Animation::find_bone(const std::string &name)
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

    Node &Animation::get_root()
    {
        return _root_node;
    }

    std::string Animation::get_name() const
    {
        return _name;
    }

    void Animation::read_missing_bones(const aiAnimation *animation,
                                       std::unordered_map<std::string, BoneInfo> &bone_map, int &bone_count)
    {
        int size = animation->mNumChannels;

        auto &model_bone_info = bone_map;
        auto &count = bone_count;

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
