//
// Created by alecpizz on 10/5/25.
//
#pragma once
namespace cologne
    {
    struct EnemyComponent
    {
        float health = 100.0f;
        bool dead = false;
        std::string hurt_sound = ASSETS_PATH "sounds/enemy_hurt.mp3";
    };
}