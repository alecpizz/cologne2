//
// Created by alecp on 5/10/2025.
//

#pragma once
#include "assimp/quaternion.h"
#include "assimp/vector3.h"
#include "assimp/vector2.h"


namespace cologne::Util
{
    glm::vec3 ai_vec3_to_glm_vec3(aiVector3t<ai_real> vector);
    glm::vec2 ai_vec2_to_glm_vec2(aiVector2t<ai_real> vector);
    glm::mat4 ai_mat4_to_glm_mat4(const aiMatrix4x4t<ai_real> &matrix);
    glm::quat ai_quat_to_glm_quat(aiQuaterniont<ai_real> quat);
    void decompose_mat4(const glm::mat4& matrix, glm::vec3& position, glm::quat& rotation, glm::vec3& scale);
    float inverse_lerp(float a, float b, float v);
    float lerp(float a, float b, float t);
    float remap(float in_min, float in_max, float out_min, float out_max, float v);
    void get_screen_to_world_ray(glm::vec2 position, glm::mat4 view, glm::mat4 proj, glm::vec3& origin, glm::vec3& dir);
}
