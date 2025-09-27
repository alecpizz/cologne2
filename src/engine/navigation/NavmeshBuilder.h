//
// Created by alecpizz on 9/13/25.
//

#pragma once
class dtNavMesh;

namespace cologne
{
    class Scene;

    class NavMeshBuilder
    {
    public:
        static void build_navmesh(Scene* scene);
        static dtNavMesh* load_navmesh(const char* path);
    };
}
