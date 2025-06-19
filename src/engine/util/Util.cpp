//
// Created by alecp on 5/10/2025.
//

#include "Util.h"

#include "assimp/matrix4x4.h"
#include "engine/core/Engine.h"

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

    glm::mat4 ai_mat4_to_glm_mat4(const aiMatrix4x4t<ai_real>& matrix)
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

    void decompose_mat4(const glm::mat4& matrix, glm::vec3 &position, glm::quat &rotation, glm::vec3 &scale)
    {
        using namespace glm;
        using T = float;

        mat4 LocalMatrix(matrix);

        // Normalize the matrix.
        if (epsilonEqual(LocalMatrix[3][3], static_cast<float>(0), epsilon<T>()))
            return;

        // First, isolate perspective.  This is the messiest.
        if (
            epsilonNotEqual(LocalMatrix[0][3], static_cast<T>(0), epsilon<T>()) ||
            epsilonNotEqual(LocalMatrix[1][3], static_cast<T>(0), epsilon<T>()) ||
            epsilonNotEqual(LocalMatrix[2][3], static_cast<T>(0), epsilon<T>()))
        {
            // Clear the perspective partition
            LocalMatrix[0][3] = LocalMatrix[1][3] = LocalMatrix[2][3] = static_cast<T>(0);
            LocalMatrix[3][3] = static_cast<T>(1);
        }

        // Next take care of translation (easy).
        position = vec3(LocalMatrix[3]);
        LocalMatrix[3] = vec4(0, 0, 0, LocalMatrix[3].w);

        vec3 Row[3], Pdum3;

        // Now get scale and shear.
        for (length_t i = 0; i < 3; ++i)
            for (length_t j = 0; j < 3; ++j)
                Row[i][j] = LocalMatrix[i][j];

        // Compute X scale factor and normalize first row.
        scale.x = length(Row[0]);
        Row[0] = detail::scale(Row[0], static_cast<T>(1));
        scale.y = length(Row[1]);
        Row[1] = detail::scale(Row[1], static_cast<T>(1));
        scale.z = length(Row[2]);
        Row[2] = detail::scale(Row[2], static_cast<T>(1));

        // At this point, the matrix (in rows[]) is orthonormal.
        // Check for a coordinate system flip.  If the determinant
        // is -1, then negate the matrix and the scaling factors.
#if 0
        Pdum3 = cross(Row[1], Row[2]); // v3Cross(row[1], row[2], Pdum3);
        if (dot(Row[0], Pdum3) < 0)
        {
            for (length_t i = 0; i < 3; i++)
            {
                scale[i] *= static_cast<T>(-1);
                Row[i] *= static_cast<T>(-1);
            }
        }
#endif

        rotation.y = asin(-Row[0][2]);
        if (cos(rotation.y) != 0) {
            rotation.x = atan2(Row[1][2], Row[2][2]);
            rotation.z = atan2(Row[0][1], Row[0][0]);
        }
        else {
            rotation.x = atan2(-Row[2][0], Row[1][1]);
            rotation.z = 0;
        }


    }

    void get_screen_to_world_ray(glm::vec2 position, glm::mat4 view, glm::mat4 proj, glm::vec3& origin, glm::vec3& dir)
    {
        int width = Engine::get_window()->get_width();
        int height = Engine::get_window()->get_height();

        float x_ndc = (2.0f * position.x) / width - 1.0f;
        float y_ndc = 1.0f - (2.0f * position.y) / height;
        float z_ndc = -1.0f; // Point on the near clipping plane
        glm::vec4 ray_clip = glm::vec4(x_ndc, y_ndc, z_ndc, 1.0f);

        glm::vec4 ray_eye = glm::inverse(proj) * ray_clip;

        ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0f, 0.0f);

        glm::vec4 ray_world_4d = glm::inverse(view) * ray_eye;
        glm::vec3 ray_direction = glm::normalize(glm::vec3(ray_world_4d));

        glm::vec3 ray_origin = glm::vec3(glm::inverse(view)[3]);
        origin = ray_origin;
        dir = ray_direction;
    }
}
