//
// Created by alecp on 5/10/2025.
//

#include "Util.h"

#include "assimp/matrix4x4.h"

namespace cologne::Util
{
    glm::vec3 ai_vec3_to_glm_vec3(const aiVector3t<ai_real> vector)
    {
        glm::vec3 result;
        result.x = vector.x;
        result.y = vector.y;
        result.z = vector.z;
        return result;
    }

    glm::vec2 ai_vec2_to_glm_vec2(const aiVector2t<ai_real> vector)
    {
        glm::vec2 result;
        result.x = vector.x;
        result.y = vector.y;
        return result;
    }

    glm::mat4 ai_mat4_to_glm_mat4(const aiMatrix4x4t<ai_real> &matrix)
    {
        glm::mat4 to;
        //the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
        to[0][0] = matrix.a1;
        to[1][0] = matrix.a2;
        to[2][0] = matrix.a3;
        to[3][0] = matrix.a4;
        to[0][1] = matrix.b1;
        to[1][1] = matrix.b2;
        to[2][1] = matrix.b3;
        to[3][1] = matrix.b4;
        to[0][2] = matrix.c1;
        to[1][2] = matrix.c2;
        to[2][2] = matrix.c3;
        to[3][2] = matrix.c4;
        to[0][3] = matrix.d1;
        to[1][3] = matrix.d2;
        to[2][3] = matrix.d3;
        to[3][3] = matrix.d4;
        return to;
    }

    glm::quat ai_quat_to_glm_quat(aiQuaterniont<ai_real> quat)
    {
        return glm::quat(quat.w, quat.x, quat.y, quat.z);
    }
}
