//
// Created by alecpizz on 3/26/2025.
//

#include "Player.h"

namespace goon
{
    struct Player::Impl
    {
        glm::vec3 position;
    };

    Player::Player()
    {
        _impl = new Impl();
    }

    Player::~Player()
    {
        delete _impl;
    }


}