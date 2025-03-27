//
// Created by alecpizz on 3/26/2025.
//

#include "Player.h"
#include <Jolt/Jolt.h>

namespace goon
{
    struct Player::Impl
    {
        glm::vec3 position;
        void init()
        {

        }
    };

    Player::Player()
    {
        _impl = new Impl();
    }

    Player::~Player()
    {
        delete _impl;
    }

    void Player::update(float dt)
    {

    }
}
