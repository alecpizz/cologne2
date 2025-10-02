//
// Created by alecpizz on 9/13/25.
//

#include "Navigation.h"

#include <DetourCrowd.h>
#include <DetourDebugDraw.h>
#include <DetourNavMesh.h>
#include <engine/scene/Components/Components.h>
#include <engine/scene/Scene.h>
#include <engine/util/FileUtil.h>

#include "NavmeshBuilder.h"
#include "NavmeshDebugDrawer.h"

namespace cologne
{
    bool Navigation::_is_drawing = false;
    dtNavMesh *Navigation::_nav_mesh = nullptr;
    dtCrowd *Navigation::_nav_crowd = nullptr;
    NavmeshDebugDrawer Navigation::_debug_drawer = {};
    dtNavMeshQuery *Navigation::_nav_query = nullptr;

    void Navigation::init_navmesh(Scene *scene)
    {
        auto path = ASSETS_PATH "navmesh/" + scene->get_scene_name() + ".navmesh";
        if (!FileUtil::file_exists(path))
        {
            LOG_WARN("NO NAVMESH EXISTS AT PATH! CREATING ONE");
            NavMeshBuilder::build_navmesh(scene);
        }

        _nav_mesh = NavMeshBuilder::load_navmesh(path.c_str());
        if (_nav_mesh)
        {
            const dtNavMesh *mesh = _nav_mesh;
            int tile_count = 0;
            int poly_count = 0;
            for (int i = 0; i < mesh->getMaxTiles(); ++i)
            {
                const dtMeshTile *tile = mesh->getTile(i);
                if (!tile || !tile->header) continue;
                tile_count++;
                poly_count += tile->header->polyCount;
            }
            LOG_INFO("Navmesh loaded with %d tiles and %d polygons.", tile_count, poly_count);
        }
        _nav_query = dtAllocNavMeshQuery();
        auto status = _nav_query->init(_nav_mesh, 2048);
        if (dtStatusFailed(status))
        {
            LOG_ERROR("Navmesh query failed to initialize!");
        }

        _nav_crowd = dtAllocCrowd();
        status = _nav_crowd->init(100, 0.6f, _nav_mesh);

        dtObstacleAvoidanceParams avoidance_params{};
        avoidance_params.velBias = 0.4f;
        avoidance_params.weightDesVel = 2.0f;
        avoidance_params.weightCurVel = 0.75f;
        avoidance_params.weightSide = 0.75f;
        avoidance_params.weightToi = 2.5f;
        avoidance_params.horizTime = 2.5f;
        avoidance_params.gridSize = 33;
        avoidance_params.adaptiveDivs = 7;
        avoidance_params.adaptiveRings = 2;
        avoidance_params.adaptiveDepth = 3;

        _nav_crowd->setObstacleAvoidanceParams(0, &avoidance_params);
        if (dtStatusFailed(status))
        {
            LOG_ERROR("Nav crowd failed to initialize!");
        }
    }

    void Navigation::cleanup()
    {
        if (_nav_mesh)
        {
            dtFreeNavMesh(_nav_mesh);
            _nav_mesh = nullptr;
        }

        if (_nav_query)
        {
            dtFreeNavMeshQuery(_nav_query);
            _nav_query = nullptr;
        }

        if (_nav_crowd)
        {
            dtFreeCrowd(_nav_crowd);
            _nav_crowd = nullptr;
        }
    }

    void Navigation::draw()
    {
        if (_nav_mesh && _is_drawing)
        {
            duDebugDrawNavMesh(&_debug_drawer, *_nav_mesh, DU_DRAWNAVMESH_COLOR_TILES);
        }
    }

    void Navigation::set_drawing_visibility(bool visible)
    {
        _is_drawing = visible;
    }

    std::vector<glm::vec3> Navigation::find_path(glm::vec3 start, glm::vec3 end)
    {
        std::vector<glm::vec3> result;
        if (_nav_query == nullptr)
        {
            return result;
        }

        glm::vec3 search_extents{1.0f, 3.0f, 1.0f};

        dtQueryFilter query_filter;

        glm::vec3 poly_start_pos;
        dtPolyRef poly_start;

        dtStatus status;
        status = _nav_query->findNearestPoly(glm::value_ptr(start), glm::value_ptr(search_extents), &query_filter,
                                             &poly_start, glm::value_ptr(poly_start_pos));

        if (dtStatusFailed(status))
        {
            return result;
        }

        glm::vec3 poly_end_pos;
        dtPolyRef poly_end;

        status = _nav_query->findNearestPoly(glm::value_ptr(end), glm::value_ptr(search_extents), &query_filter,
                                             &poly_end, glm::value_ptr(poly_end_pos));

        if (dtStatusFailed(status))
        {
            return result;
        }

        dtPolyRef path_polys[32];
        int num_path_polys;
        status = _nav_query->findPath(poly_start, poly_end, glm::value_ptr(poly_start_pos),
                                      glm::value_ptr(poly_end_pos), &query_filter, path_polys, &num_path_polys, 32);

        if (dtStatusFailed(status) && !dtStatusDetail(status, DT_PARTIAL_RESULT))
        {
            return result;
        }

        glm::vec3 path_points[32];
        dtPolyRef straight_path_polys[32];
        uint8_t path_flags[32];
        int num_path_points;
        status = _nav_query->findStraightPath(glm::value_ptr(poly_start_pos), glm::value_ptr(poly_end_pos), path_polys,
                                              num_path_polys, glm::value_ptr(path_points[0]), path_flags,
                                              straight_path_polys, &num_path_points, 32, DT_STRAIGHTPATH_ALL_CROSSINGS);
        if (dtStatusFailed(status) && !dtStatusDetail(status, DT_PARTIAL_RESULT))
        {
            return result;
        }


        for (int i = 0; i < num_path_points; i++)
        {
            result.emplace_back(path_points[i]);
        }
        return result;
    }

    void Navigation::update_crowd(float dt)
    {
        if (!_nav_crowd)
        {
            return;
        }
        _nav_crowd->update(dt, nullptr);
    }

    void Navigation::set_agent_target(int agent_id, const glm::vec3 &target_pos)
    {
        if (!_nav_crowd)
        {
            return;
        }

        const dtQueryFilter* filter = _nav_crowd->getFilter(0);
        dtPolyRef target_poly;
        glm::vec3 nearest_point;
        glm::vec3 search_extents{1.0f, 3.0f, 1.0f};
        _nav_query->findNearestPoly(glm::value_ptr(target_pos), glm::value_ptr(search_extents),
            filter, &target_poly, glm::value_ptr(nearest_point));
        if (target_poly)
        {
            _nav_crowd->requestMoveTarget(agent_id, target_poly, glm::value_ptr(nearest_point));
        }
    }


    glm::vec3 Navigation::get_agent_position(int agent_id)
    {
       if (!_nav_crowd)
       {
           return {0.0f, 0.0f, 0.0f};
       }
        const dtCrowdAgent* agent = _nav_crowd->getAgent(agent_id);
        if (agent)
        {
            return {agent->npos[0], agent->npos[1], agent->npos[2]};
        }
        return glm::vec3(0.0f);
    }

    int Navigation::add_agent(glm::vec3 vec, const NPCCrowdMemberComponent& crowd_member)
    {
        if (!_nav_crowd)
        {
            return -1;
        }

        dtCrowdAgentParams agent_params = {};
        agent_params.radius = 0.2f;
        agent_params.height = 1.8f;
        agent_params.maxAcceleration = crowd_member.max_acceleration;
        agent_params.maxSpeed = crowd_member.max_speed;
        agent_params.updateFlags = DT_CROWD_ANTICIPATE_TURNS | DT_CROWD_OPTIMIZE_VIS | DT_CROWD_OPTIMIZE_TOPO
        | DT_CROWD_OBSTACLE_AVOIDANCE | DT_CROWD_SEPARATION;
        agent_params.obstacleAvoidanceType = 0;
        agent_params.separationWeight = 2.0f;
        agent_params.collisionQueryRange = agent_params.radius * 12.0f;
        agent_params.pathOptimizationRange = agent_params.radius * 30.0f;
        
        return _nav_crowd->addAgent(glm::value_ptr(vec), &agent_params);
    }

    void Navigation::remove_agent(int agent_id)
    {
        if (!_nav_crowd)
        {
            return;
        }
        _nav_crowd->removeAgent(agent_id);
    }
}
