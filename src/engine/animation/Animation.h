#pragma once
#include "BoneAnimationData.h"
#include "../renderer/types/SkinnedModel.h"

struct aiScene;
struct aiNode;
struct aiAnimation;

namespace cologne
{
    struct Node
    {
        glm::mat4 transform;
        std::string name;
        std::vector<Node> children;
    };

    class SkinnedModel;
    struct BoneInfo;

    class Animation
    {
        friend class Animator;
    public:
        Animation(const std::string& path, SkinnedModel& model);
        Animation(aiAnimation* animation, const aiScene* scene, SkinnedModel& model);
        BoneAnimationData* find_bone(const std::string& name);
        float get_ticks_per_second() const;
        float get_duration() const;
        Node& get_root();
        static std::vector<Animation> get_animations(const std::string& path, SkinnedModel& model);
    private:
        void read_missing_bones(const aiAnimation* animation, SkinnedModel& model);
        void read_bone_hierarchy(Node& dest, const aiNode* src);
        std::unordered_map<std::string, BoneAnimationData> _bone_data_map;
        std::unordered_map<std::string, BoneInfo> _bone_info_map;
        float _duration;
        int _ticks_per_second;
        Node _root_node;
    };

}
