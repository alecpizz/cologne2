#pragma once

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
    class BoneAnimationData;
    struct BoneInfo;

    class Animation
    {
    public:
        Animation(const std::string& path, SkinnedModel& model);
        BoneAnimationData* find_bone(const std::string& name);
        float get_ticks_per_second() const;
        float get_duration() const;
        Node& get_root();
    private:
        void read_missing_bones(const aiAnimation* animation, SkinnedModel& model);
        void read_bone_hierarchy(Node& dest, const aiNode* src);
        std::unordered_map<std::string, BoneAnimationData> _bone_data_map;
        std::unordered_map<std::string, BoneInfo> _bone_info_map;
        float _duration;
        int _ticks_per_second;
        Node _root_node;
    };

    std::vector<Animation> get_animations(const std::string& path, SkinnedModel& model);
}
