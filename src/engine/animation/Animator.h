//
// Created by alecpizz on 5/28/25.
//

#pragma once

namespace cologne
{
    struct Node;
    class Animation;
    class Animator
    {
    public:
        explicit Animator(Animation& anim);
        void update_animation(float dt);
        void play_animation(Animation& anim);
        void calculate_bone_transform( Node& node, glm::mat4 parent_transform);
        std::vector<glm::mat4>& get_bones();
    private:
        std::vector<glm::mat4> _bone_mats;
        Animation& _current_animation;
        float _current_time = 0.0f;
    };
}
