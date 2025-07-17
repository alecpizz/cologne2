#pragma once

struct aiNodeAnim;

namespace cologne
{
    struct AnimationKeyPosition
    {
        glm::vec3 position;
        float time_stamp;
    };

    struct AnimationKeyRotation
    {
        glm::quat rotation;
        float time_stamp;
    };

    struct AnimationKeyScale
    {
        glm::vec3 scale;
        float time_stamp;
    };

    class BoneAnimationData
    {
    public:
        BoneAnimationData() = default;
        BoneAnimationData(const std::string& name, const aiNodeAnim* channel);
        BoneAnimationData(const std::string& name, const std::vector<AnimationKeyPosition>& positions, const std::vector<AnimationKeyRotation>& rotations, const std::vector<AnimationKeyScale>& scales);
        void update(float animation_time);
        glm::mat4 get_transform() const;
        std::string get_name() const;
        int get_id() const;
        int get_position_idx(float time);
        int get_rotation_idx(float time);
        int get_scale_idx(float time);
        const std::vector<AnimationKeyPosition>& get_positions() const;
        const std::vector<AnimationKeyRotation>& get_rotations() const;
        const std::vector<AnimationKeyScale>& get_scales() const;
    private:
        float get_scale_factor(float last_time_stamp, float next_time_stamp, float animation_time);
        glm::mat4 interpolate_position(float animation_time);
        glm::mat4 interpolate_rotation(float animation_time);
        glm::mat4 interpolate_scale(float animation_time);

        std::vector<AnimationKeyPosition> _positions;
        std::vector<AnimationKeyRotation> _rotations;
        std::vector<AnimationKeyScale> _scales;
        glm::mat4 _transform;
        std::string _name;
    };
}
