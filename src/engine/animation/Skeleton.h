#pragma once
//
// Created by alecpizz on 6/29/25.
//


namespace cologne
{
    struct Bone
    {
        std::string name;
        glm::mat4 inverse_bind_pose;
        glm::mat4 local_bind_transform;
        int parent_idx;
    };

    class Skeleton
    {
    public:
        Skeleton() = default;

        int find_bone_index(const std::string &name) const;

        const std::vector<Bone> &get_bones() const;

        size_t get_bone_count() const;

        std::vector<Bone> _bones; //TODO: private me
        std::unordered_map<std::string, int> _bone_name_to_index; //TODO: private me
    };
}
