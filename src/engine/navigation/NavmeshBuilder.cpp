//
// Created by alecpizz on 9/13/25.
//
#include "NavmeshBuilder.h"
#include <Recast.h>
#include <DetourNavMesh.h>
#include <DetourNavMeshBuilder.h>
#include <DetourNavMeshQuery.h>
#include <fstream>
#include <engine/asset_manager/AssetManager.h>
#include <engine/scene/Components/Components.h>
#include <engine/scene/Scene.h>
#include <engine/util/FileUtil.h>

namespace cologne
{
    class BuildContext : public rcContext
    {
    public:
        void doLog(const rcLogCategory category, const char *msg, const int len) override
        {
            switch (category)
            {
                case rcLogCategory::RC_LOG_ERROR:
                {
                    LOG_ERROR("Recast: %s", msg);
                }
                case rcLogCategory::RC_LOG_WARNING:
                {
                    LOG_WARN("Recast: %s", msg);
                }
                case rcLogCategory::RC_LOG_PROGRESS:
                {
                    LOG_INFO("Recast: %s", msg);
                }
            }
        }
    };

    static const int NAVMESHSET_MAGIC = 'M' << 24 | 'S' << 16 | 'E' << 8 | 'T'; //'MSET';
    static const int NAVMESHSET_VERSION = 1;

    struct NavMeshSetHeader
    {
        int magic;
        int version;
        int numTiles;
        dtNavMeshParams params;
    };

    struct NavMeshTileHeader
    {
        dtTileRef tileRef;
        int dataSize;
    };

    static bool save(const char *path, const dtNavMesh *mesh);

    void NavMeshBuilder::build_navmesh(Scene *scene)
    {
        auto &registry = scene->get_raw_registry();
        auto view = registry.view<StaticColliderComponent, WorldTransformComponent>();

        std::vector<glm::vec3> all_vertices;
        std::vector<int> all_triangles;
        for (auto entity: view)
        {
            auto &sc = registry.get<StaticColliderComponent>(entity);
            const auto transform = registry.get<WorldTransformComponent>(entity).transform;
            const auto &mesh_name = sc.mesh_name;
            const auto mesh = AssetManager::get_mesh_by_name(mesh_name);
            if (!mesh)
            {
                continue;
            }

            const auto verts = mesh->get_vertices();
            const int vert_offset = static_cast<int>(all_vertices.size());
            for (const auto &vertex: verts)
            {
                glm::vec4 world_pos = transform * glm::vec4(vertex.position, 1.0f);
                all_vertices.emplace_back(world_pos);
            }

            for (int i = 0; i < mesh->get_indices_count(); i += 3)
            {
                all_triangles.push_back(mesh->get_indices()[i + 0] + vert_offset);
                all_triangles.push_back(mesh->get_indices()[i + 1] + vert_offset);
                all_triangles.push_back(mesh->get_indices()[i + 2] + vert_offset);
            }
        }


        const float *verts = reinterpret_cast<const float *>(all_vertices.data());
        const int nverts = all_vertices.size();
        const int *tris = all_triangles.data();
        const int ntris = all_triangles.size() / 3;


        BuildContext context;

        rcConfig cfg = {};
        memset(&cfg, 0, sizeof(cfg));
        float cellSize = 0.1f;
        float cellHeight = 0.1f;
        float agentHeight = 1.8f;
        float agentRadius = 0.2f;
        float agentMaxClimb = 0.2f;
        float agentMaxSlope = 45.0f;
        float regionMinSize = 8;
        float regionMergeSize = 20;
        float edgeMaxLen = 12.0f;
        float edgeMaxError = 1.3f;
        float vertsPerPoly = 6.0f;
        float detailSampleDist = 6.0f;
        float detailSampleMaxError = 1.0f;
        // float partitionType = SAMPLE_PARTITION_WATERSHED;
        cfg.cs = cellSize;
        cfg.ch = cellHeight;
        cfg.walkableSlopeAngle = agentMaxSlope;
        cfg.walkableHeight = (int) ceilf(agentHeight / cfg.ch);
        cfg.walkableClimb = (int) floorf(agentMaxClimb / cfg.ch);
        cfg.walkableRadius = (int) ceilf(agentRadius / cfg.cs);
        cfg.maxEdgeLen = 20;
        cfg.maxSimplificationError = edgeMaxError;
        cfg.minRegionArea = (int) rcSqr(regionMinSize); // Note: area = size*size
        cfg.mergeRegionArea = (int) rcSqr(regionMergeSize); // Note: area = size*size
        cfg.maxVertsPerPoly = (int) vertsPerPoly;
        cfg.detailSampleDist = 1.0f;
        cfg.detailSampleMaxError = 0.2f;

        rcCalcBounds(verts, nverts, cfg.bmin, cfg.bmax);
        rcCalcGridSize(cfg.bmin, cfg.bmax, cfg.cs, &cfg.width, &cfg.height);


        auto height_field = rcAllocHeightfield();
        if (!height_field)
        {
            LOG_ERROR("COULDNT BUILD SOLID");
            return;
        }


        if (!rcCreateHeightfield(&context, *height_field,
                                 cfg.width, cfg.height,
                                 cfg.bmin, cfg.bmax, cfg.cs, cfg.ch))
        {
            LOG_ERROR("Couldn't create solid heightfield!");
            return;
        }

        auto tri_areas = new unsigned char[ntris];

        memset(tri_areas, 0, ntris * sizeof(unsigned char));
        rcMarkWalkableTriangles(&context, cfg.walkableSlopeAngle, verts, nverts, tris, ntris, tri_areas);
        if (!rcRasterizeTriangles(&context, verts, nverts, tris, tri_areas, ntris, *height_field, cfg.walkableClimb))
        {
            LOG_ERROR("COULDNT RASTERIZE TRIANGLES!");
            return;
        }

        delete[] tri_areas;
        tri_areas = nullptr;

        rcFilterLowHangingWalkableObstacles(&context, cfg.walkableClimb, *height_field);
        rcFilterLedgeSpans(&context, cfg.walkableHeight, cfg.walkableClimb, *height_field);
        rcFilterWalkableLowHeightSpans(&context, cfg.walkableHeight, *height_field);

        auto compact_height_field = rcAllocCompactHeightfield();
        if (!compact_height_field)
        {
            LOG_ERROR("COULDNT ALLOC COMPACT HEIGHT FIELD");
            return;
        }

        if (!rcBuildCompactHeightfield(&context, cfg.walkableHeight, cfg.walkableClimb, *height_field,
                                       *compact_height_field))
        {
            LOG_ERROR("COULDNT BUILD COMPACT DATA");
            return;
        }

        rcFreeHeightField(height_field);

        if (!rcErodeWalkableArea(&context, cfg.walkableRadius, *compact_height_field))
        {
            LOG_ERROR("COULDNT ERRODE NAVMESH");
            return;
        }

        //watershed partiioning
        if (!rcBuildDistanceField(&context, *compact_height_field))
        {
            LOG_ERROR("Could not build distance field");
            return;
        }

        if (!rcBuildRegions(&context, *compact_height_field, 0, cfg.minRegionArea, cfg.mergeRegionArea))
        {
            LOG_ERROR("Could not build regions");
            return;
        }

        auto contour_set = rcAllocContourSet();
        if (!contour_set)
        {
            LOG_ERROR("couldn't alloc contour set!");
            return;
        }

        if (!rcBuildContours(&context, *compact_height_field, cfg.maxSimplificationError, cfg.maxEdgeLen, *contour_set))
        {
            LOG_ERROR("Couldn't build contours!");
            return;
        }

        auto poly_mesh = rcAllocPolyMesh();
        if (!poly_mesh)
        {
            LOG_ERROR("Couldn't alloc polymesh");
            return;
        }

        if (!rcBuildPolyMesh(&context, *contour_set, cfg.maxVertsPerPoly, *poly_mesh))
        {
            LOG_ERROR("Couldn't build polymesh!");
            return;
        }

        auto detail_mesh = rcAllocPolyMeshDetail();
        if (!detail_mesh)
        {
            LOG_ERROR("Couldn't alloc detail mesh!");
            return;
        }

        if (!rcBuildPolyMeshDetail(&context, *poly_mesh, *compact_height_field, cfg.detailSampleDist,
                                   cfg.detailSampleMaxError,
                                   *detail_mesh))
        {
            LOG_ERROR("Couldn't build detail poly mesh");
            return;
        }

        for (int i = 0; i < poly_mesh->npolys; i++)
        {
            poly_mesh->flags[i] = 1;
        }

        rcFreeCompactHeightfield(compact_height_field);
        rcFreeContourSet(contour_set);


        //finally build detour data

        if (cfg.maxVertsPerPoly <= DT_VERTS_PER_POLYGON)
        {
            unsigned char *nav_data = 0;
            int nav_data_size = 0;

            dtNavMeshCreateParams params = {};
            params.verts = poly_mesh->verts;
            params.vertCount = poly_mesh->nverts;
            params.polys = poly_mesh->polys;
            params.polyAreas = poly_mesh->areas;
            params.polyFlags = poly_mesh->flags;
            params.polyCount = poly_mesh->npolys;
            params.nvp = poly_mesh->nvp;
            params.detailMeshes = detail_mesh->meshes;
            params.detailVerts = detail_mesh->verts;
            params.detailVertsCount = detail_mesh->nverts;
            params.detailTris = detail_mesh->tris;
            params.detailTriCount = detail_mesh->ntris;
            params.walkableHeight = agentHeight * cfg.ch;
            params.walkableRadius = agentRadius * cfg.cs;
            params.walkableClimb = agentMaxClimb * cfg.ch;
            params.offMeshConVerts = nullptr; // m_geom->getOffMeshConnectionVerts();
            params.offMeshConRad = nullptr; //m_geom->getOffMeshConnectionRads();
            params.offMeshConDir = nullptr; // m_geom->getOffMeshConnectionDirs();
            params.offMeshConAreas = nullptr; // m_geom->getOffMeshConnectionAreas();
            params.offMeshConFlags = nullptr; // m_geom->getOffMeshConnectionFlags();
            params.offMeshConUserID = nullptr; // m_geom->getOffMeshConnectionId();
            params.offMeshConCount = 0; // m_geom->getOffMeshConnectionCount();
            rcVcopy(params.bmin, poly_mesh->bmin);
            rcVcopy(params.bmax, poly_mesh->bmax);
            params.cs = cfg.cs;
            params.ch = cfg.ch;
            params.buildBvTree = true;

            if (!dtCreateNavMeshData(&params, &nav_data, &nav_data_size))
            {
                LOG_ERROR("COULDNT CREATE DETOUR NAVMESH");
                return;
            }

            dtNavMesh *navmesh = dtAllocNavMesh();
            if (!navmesh)
            {
                dtFree(nav_data);
                LOG_ERROR("COULDNT ALLOC NAVMESH");
                return;
            }

            dtStatus status;
            status = navmesh->init(nav_data, nav_data_size, DT_TILE_FREE_DATA);
            if (dtStatusFailed(status))
            {
                dtFree(nav_data);
                LOG_ERROR("Couldn't init detour navmesh!");
                return;
            }

            auto path = ASSETS_PATH "navmesh/" + scene->get_scene_name() + ".navmesh";
            if (!save(path.c_str(), navmesh))
            {
                LOG_ERROR("COULDNT SAVE NAVMESH");
                return;
            }
            dtFreeNavMesh(navmesh);
        }

        rcFreePolyMesh(poly_mesh);
        rcFreePolyMeshDetail(detail_mesh);
    }

    struct DtDeleter
    {
        void operator()(void *ptr) const
        {
            dtFree(ptr);
        }
    };

    struct DtNavMeshDeleter
    {
        void operator()(dtNavMesh *mesh) const
        {
            dtFreeNavMesh(mesh);
        }
    };

    using dtNavMeshUniquePtr = std::unique_ptr<dtNavMesh, DtNavMeshDeleter>;
    using dtDataUniquePtr = std::unique_ptr<unsigned char, DtDeleter>;

    dtNavMesh *NavMeshBuilder::load_navmesh(const char *path)
    {
        std::ifstream file(path, std::ios::binary);
        if (!file)
        {
            LOG_ERROR("Couldn't open file at path %s", path);
            return nullptr;
        }

        NavMeshSetHeader header;
        file.read(reinterpret_cast<char *>(&header), sizeof(NavMeshSetHeader));
        if (!file)
        {
            LOG_ERROR("Couldn't read header");
            return nullptr;
        }

        if (header.magic != NAVMESHSET_MAGIC || header.version != NAVMESHSET_VERSION)
        {
            LOG_ERROR("Invalid header magic or version");
            return nullptr;
        }

        dtNavMeshUniquePtr mesh(dtAllocNavMesh());
        if (!mesh)
        {
            LOG_ERROR("Couldn't allocate mesh");
            return nullptr;
        }

        if (dtStatusFailed(mesh->init(&header.params)))
        {
            LOG_ERROR("Couldn't initialize mesh");
            return nullptr;
        }

        // Read tiles.
        for (int i = 0; i < header.numTiles; ++i)
        {
            NavMeshTileHeader tileHeader;
            file.read(reinterpret_cast<char *>(&tileHeader), sizeof(tileHeader));
            if (!file)
            {
                LOG_ERROR("Couldn't read header for tile %d", i);
                return nullptr;
            }

            if (!tileHeader.tileRef || !tileHeader.dataSize)
            {
                break;
            }

            dtDataUniquePtr tileData(static_cast<unsigned char *>(dtAlloc(tileHeader.dataSize, DT_ALLOC_PERM)));
            if (!tileData)
            {
                LOG_ERROR("Couldn't allocate tile data");
                return nullptr;
            }

            file.read(reinterpret_cast<char *>(tileData.get()), tileHeader.dataSize);
            if (!file)
            {
                LOG_ERROR("Couldn't read tile data for tile %d", i);
                return nullptr;
            }

            if (dtStatusFailed(mesh->addTile(tileData.get(), tileHeader.dataSize, DT_TILE_FREE_DATA, tileHeader.tileRef,
                                             0)))
            {
                LOG_ERROR("Couldn't add tile to navmesh!");
                return nullptr;
            }
            tileData.release();
        }

        return mesh.release();
    }


    bool save(const char *path, const dtNavMesh *mesh)
    {
        if (!mesh) return false;

        std::ofstream file(path, std::ios::binary);
        if (!file)
        {
            LOG_ERROR("Couldn't open file for writing %s", path);
            return false;
        }

        NavMeshSetHeader header;
        header.magic = NAVMESHSET_MAGIC;
        header.version = NAVMESHSET_VERSION;
        header.numTiles = 0;
        for (int i = 0; i < mesh->getMaxTiles(); ++i)
        {
            const dtMeshTile *tile = mesh->getTile(i);
            if (tile && tile->header && tile->dataSize)
            {
                header.numTiles++;
            }
        }
        memcpy(&header.params, mesh->getParams(), sizeof(dtNavMeshParams));

        file.write(reinterpret_cast<const char *>(&header), sizeof(NavMeshSetHeader));
        if (!file)
        {
            LOG_ERROR("Couldn't write navmesh header");
            return false;
        }

        // Store tiles.
        for (int i = 0; i < mesh->getMaxTiles(); ++i)
        {
            const dtMeshTile *tile = mesh->getTile(i);
            if (!tile || !tile->header || !tile->dataSize) continue;

            NavMeshTileHeader tileHeader;
            tileHeader.tileRef = mesh->getTileRef(tile);
            tileHeader.dataSize = tile->dataSize;

            file.write(reinterpret_cast<const char *>(&tileHeader), sizeof(tileHeader));
            if (!file)
            {
                LOG_ERROR("Couldn't write tile header!");
                return false;
            }

            file.write(reinterpret_cast<const char *>(tile->data), tile->dataSize);
            if (!file)
            {
                LOG_ERROR("Couldn't write tile data!");
                return false;
            }
        }

        return true;
    }
}
