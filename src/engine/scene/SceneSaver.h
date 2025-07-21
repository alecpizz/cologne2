//
// Created by alecpizz on 7/19/25.
//
#pragma once
namespace cologne
{
    class Scene;
}

namespace cologne
    {
    class SceneSaver
    {
    public:
        SceneSaver(Scene* scene);
        void serialize(const std::string& path);
        void deserialize(const std::string& path);
        void serialize_runtime(const std::string& path);
        void deserialize_runtime(const std::string& path);
    private:
        Scene* _scene;
    };
}
