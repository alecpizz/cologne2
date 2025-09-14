//
// Created by alecpizz on 9/13/25.
//

#include "Navigation.h"

#include <DetourDebugDraw.h>
#include <DetourNavMesh.h>
#include <engine/scene/Scene.h>
#include <engine/util/FileUtil.h>

#include "NavmeshBuilder.h"
#include "NavmeshDebugDrawer.h"

namespace cologne
{
    bool  Navigation::_is_drawing = false;
    dtNavMesh * Navigation::_nav_mesh = nullptr;
    NavmeshDebugDrawer  Navigation::_debug_drawer = {};
    void Navigation::init_for_scene(Scene *scene)
    {
        auto path = ASSETS_PATH "navmesh/" + scene->get_scene_name() + ".navmesh";
        if (!FileUtil::file_exists(path))
        {
            LOG_WARN("NO NAVMESH EXISTS AT PATH! CREATING ONE");
            NavMeshBuilder::build_navmesh(scene);
        }

        _nav_mesh = NavMeshBuilder::load_navmesh(path.c_str());
    }

    void Navigation::cleanup()
    {
        if (_nav_mesh)
        {
            dtFreeNavMesh(_nav_mesh);
            _nav_mesh = nullptr;
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
}
