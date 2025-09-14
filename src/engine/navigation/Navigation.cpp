//
// Created by alecpizz on 9/13/25.
//

#include "Navigation.h"

#include <DetourDebugDraw.h>
#include <DetourNavMesh.h>
#include <engine/core/Engine.h>
#include <engine/scene/Scene.h>
#include <engine/util/FileUtil.h>

#include "NavmeshBuilder.h"
#include "NavmeshDebugDrawer.h"

namespace cologne
{
    bool Navigation::_is_drawing = false;
    dtNavMesh *Navigation::_nav_mesh = nullptr;
    NavmeshDebugDrawer Navigation::_debug_drawer = {};
    dtNavMeshQuery *Navigation::_nav_query = nullptr;
    static const int MAX_POLYS = 256;
    constexpr float POLY_PICK_EXTENTS[3] = {2.0f, 50.0f, 2.0f};

    void Navigation::init_for_scene(Scene *scene)
    {
        auto path = ASSETS_PATH "navmesh/" + scene->get_scene_name() + ".navmesh";
        if (true)
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

        glm::vec3 search_extents {1.0f, 3.0f, 1.0f};

        dtQueryFilter query_filter;

        glm::vec3 poly_start_pos;
        dtPolyRef poly_start;

        dtStatus status;
        status = _nav_query->findNearestPoly(glm::value_ptr(start), glm::value_ptr(search_extents ), &query_filter,
                                             &poly_start, glm::value_ptr(poly_start_pos));

        if (dtStatusFailed(status))
        {
            return result;
        }

        glm::vec3 poly_end_pos;
        dtPolyRef poly_end;

        status = _nav_query->findNearestPoly(glm::value_ptr(end), glm::value_ptr(search_extents ), &query_filter,
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
}
