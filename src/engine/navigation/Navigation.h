//
// Created by alecpizz on 9/13/25.
//
#pragma once

class dtCrowd;
class dtNavMeshQuery;
class dtNavMesh;

namespace cologne
{
    struct NPCCrowdMemberComponent;
    class Scene;
    class NavmeshDebugDrawer;
    class Navigation
    {
    public:
        static void init_navmesh(Scene * scene);
        static void cleanup();
        static void draw();
        static void set_drawing_visibility(bool visible);
        static std::vector<glm::vec3> find_path(glm::vec3 start, glm::vec3 end);
        static void update_crowd(float dt);
        static void set_agent_target(int agent_id, const glm::vec3& target_pos);
        static glm::vec3 get_agent_position(int agent_id);
        static int add_agent(glm::vec3 vec, const NPCCrowdMemberComponent& crowd_member);

    private:
        static bool _is_drawing;
        static dtCrowd* _nav_crowd;
        static dtNavMesh* _nav_mesh;
        static dtNavMeshQuery* _nav_query;
        static NavmeshDebugDrawer _debug_drawer;
    };
}
