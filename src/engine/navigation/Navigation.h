//
// Created by alecpizz on 9/13/25.
//
#pragma once

class dtNavMeshQuery;
class dtNavMesh;

namespace cologne
{
    class Scene;
    class NavmeshDebugDrawer;
    class Navigation
    {
    public:
        static void init_for_scene(Scene * scene);
        static void cleanup();
        static void draw();
        static void set_drawing_visibility(bool visible);
        static std::vector<glm::vec3> find_path(glm::vec3 start, glm::vec3 end);
    private:
        static bool _is_drawing;
        static dtNavMesh* _nav_mesh;
        static dtNavMeshQuery* _nav_query;
        static NavmeshDebugDrawer _debug_drawer;
    };
}
