//
// Created by alecpizz on 9/12/25.
//
#pragma once

namespace cologne
{
    class Entity;
    class Scene;

    class Prefab
    {
    public:
        static void create(Entity entity, const std::string &path);

        static Entity instantiate(Scene *scene, const std::string &path);
    };
}
