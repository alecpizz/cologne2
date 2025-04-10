#pragma once

namespace cologne
{
    class Player
    {
    public:
        Player();

        ~Player();

        Player(Player &&) = delete;

        Player(const Player &) = delete;

        Player &operator=(Player &&) = delete;

        Player &operator=(const Player &) = delete;

        glm::vec3 get_position();
        glm::vec3 get_camera_position();

        void update(float dt);

    private:
        struct Impl;
        Impl *_impl;
    };
}
