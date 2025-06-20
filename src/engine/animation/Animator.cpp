//
// Created by alecpizz on 5/28/25.
//

#include "Animator.h"

#include <engine/asset_manager/AssetManager.h>

#include "Animation.h"
#include "BoneAnimationData.h"

namespace cologne
{
    AnimatorComponent::AnimatorComponent(Animation &anim) : _current_animation(anim)
    {
        _current_time = 0.0f;
        _bone_mats.reserve(200);
        for (int i = 0; i < 200; i++)
        {
            _bone_mats.emplace_back(1.0f);
        }
    }

    AnimatorComponent::AnimatorComponent(size_t idx) : _current_animation(*AssetManager::get_animation_by_index(idx))
    {
        _bone_mats.reserve(200);
        for (int i = 0; i < 200; i++)
        {
            _bone_mats.emplace_back(1.0f);
        }
    }

    void AnimatorComponent::update_animation(float dt)
    {
        if (&_current_animation)
        {
            _current_time += _current_animation.get_ticks_per_second() * dt;
            _current_time = std::fmod(_current_time, _current_animation.get_duration());
            calculate_bone_transform(_current_animation.get_root(), glm::mat4(1.0f));
        }
    }

    void AnimatorComponent::play_animation(Animation &anim)
    {
        _current_animation = anim;
        _current_time = 0.0f;
    }

    void AnimatorComponent::play_animation(size_t idx)
    {
        _current_animation = *AssetManager::get_animation_by_index(idx);
        _current_time = 0.0f;
    }

    void AnimatorComponent::calculate_bone_transform(Node &node, glm::mat4 parent_transform)
    {
        std::string node_name = node.name;
        glm::mat4 node_transform = node.transform;
        auto bone = _current_animation.find_bone(node_name);
        if (bone != nullptr)
        {
            bone->update(_current_time);
            node_transform = bone->get_transform();
        }

        glm::mat4 global_transform = parent_transform * node_transform;

        auto& bone_map = _current_animation._bone_info_map;
        if (bone_map.contains(node_name))
        {
            int idx = bone_map[node_name].id;
            glm::mat4 offset = bone_map[node_name].offset;
            _bone_mats[idx] = global_transform * offset;
        }

        for (auto & child_node : node.children)
        {
            calculate_bone_transform(child_node, global_transform);
        }
    }

    std::vector<glm::mat4> &AnimatorComponent::get_bones()
    {
        return _bone_mats;
    }

    Animation & AnimatorComponent::get_current_animation() const
    {
        return _current_animation;
    }

    float AnimatorComponent::get_current_time() const
    {
        return _current_time;
    }
}
