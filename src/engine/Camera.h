#pragma once

namespace goon
{
    class Camera
    {
    public:
        Camera(glm::vec3 position, glm::vec3 forward, glm::vec3 up);

        Camera(Camera &&) = delete;

        Camera(const Camera &) = delete;

        Camera &operator=(Camera &&) = delete;

        Camera &operator=(const Camera &) = delete;

        glm::vec3 get_position() const;

        glm::vec3 get_forward() const;

        glm::vec3 get_up() const;

        glm::vec3 get_right() const;

        glm::mat4 get_view_matrix() const;

        glm::mat4 get_projection_matrix() const;

        void set_free_cam(bool on);

        bool is_free_cam() const;

        float get_fov() const;

        void update(float dt);

    private:
        bool _is_free_cam = false;
        float fov = glm::radians(45.0f);
        glm::vec3 _position = {0.0f, 0.0f, 0.0f};
        glm::vec3 _forward = {0.0f, 0.0f, 1.0f};
        glm::vec3 _up = {0.0f, 1.0f, 0.0f};
        bool _active = true;
        float _pitch = 0.0f;
        float _yaw = 0.0f;
    };
}
