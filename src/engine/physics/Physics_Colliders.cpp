#include "Physics.h"
#include "PhysicsUtil.h"
#include "RaycastHitInfo.h"
#include <engine/renderer/Renderer.h>
#include <engine/renderer/types/Mesh.h>
#include <engine/scene/Components.h>
#include <engine/util/FileUtil.h>
#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Character/Character.h>
#include <Jolt/Physics/Character/CharacterVirtual.h>
#include <Jolt/Physics/Collision/Shape/PlaneShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Core/StreamWrapper.h>

#include <fstream>
#include <engine/asset_manager/AssetManager.h>

#include "Jolt/Physics/Ragdoll/Ragdoll.h"

//
// Created by alecpizz on 9/14/25.
//
namespace cologne
{
    using namespace JPH;
     uint32_t Physics::create_static_mesh_collider(Entity entity, TransformComponent transform,
                                         const Mesh &mesh)
    {
        JPH::Ref<Shape> mesh_shape;
        const std::string path = ASSETS_PATH "cache/colliders/" + mesh.get_name() + ".ccol";
        if (!FileUtil::file_exists(path))
        {
            FileUtil::create_directory_recursive(path);
            LOG_INFO("No cache collider for %s mesh! Generating one.", mesh.get_name().c_str());
            JPH::TriangleList triangle_list;
            for (int i = 0; i * 3 < mesh.get_indices_count(); i++)
            {
                Triangle triangle =
                {
                    PhysicsUtil::glm_vec3_to_float3(mesh.get_vertices()[mesh.get_indices()[3 * i]].position),
                    PhysicsUtil::glm_vec3_to_float3(mesh.get_vertices()[mesh.get_indices()[3 * i + 1]].position),
                    PhysicsUtil::glm_vec3_to_float3(mesh.get_vertices()[mesh.get_indices()[3 * i + 2]].position)
                };
                triangle_list.emplace_back(triangle);
            }
            JPH::MeshShapeSettings mesh_settings(triangle_list);
            mesh_settings.mMaxTrianglesPerLeaf = 4;
            mesh_settings.mBuildQuality = MeshShapeSettings::EBuildQuality::FavorRuntimePerformance;
            mesh_settings.SetEmbedded();
            auto result = mesh_settings.Create();
            mesh_shape = result.Get();
            //export now
            std::ofstream file(path, std::ios::binary);
            if (!file.is_open())
            {
                LOG_ERROR("Couldn't open file to export collision shape!");
                return -1;
            }
            JPH::StreamOutWrapper stream_out(file);
            JPH::Shape::ShapeToIDMap shape_to_id_map;
            JPH::Shape::MaterialToIDMap material_to_id_map;
            mesh_shape->SaveWithChildren(stream_out, shape_to_id_map, material_to_id_map);
            file.close();
        }
        else
        {
            // settings = something_else;
            std::ifstream file(path, std::ios::binary);
            if (!file.is_open())
            {
                LOG_ERROR("Couldn't open file to import collision shape!");
                return -1;
            }
            JPH::StreamInWrapper stream_in(file);
            JPH::Shape::IDToShapeMap id_to_shape_map;
            JPH::Shape::IDToMaterialMap id_to_material_map;
            JPH::Shape::ShapeResult result = JPH::Shape::sRestoreWithChildren(
                stream_in, id_to_shape_map, id_to_material_map);
            file.close();
            if (result.IsValid())
            {
                mesh_shape = result.Get();
            }
            else
            {
                LOG_ERROR("Couldn't open shape from file!");
                return -1;
            }
        }
        auto quat = PhysicsUtil::glm_quat_to_jph_quat(transform.rotation);
        if (!quat.IsNormalized())
        {
            LOG_INFO("Quat isn't normalized!");
            quat = quat.sIdentity();
        }
        auto settings = BodyCreationSettings(new ScaledShapeSettings(mesh_shape,
                                                                     PhysicsUtil::glm_vec3_to_jph_vec3(transform.scale)),
                                             PhysicsUtil::glm_vec3_to_jph_vec3(transform.position), quat, JPH::EMotionType::Static,
                                             cologne::Physics::NON_MOVING);

        auto &body_interface = _physics_system.GetBodyInterface();
        auto id = body_interface.CreateAndAddBody(
            settings, JPH::EActivation::DontActivate);
        _colliders_static.push_back(id);
        _physics_system.OptimizeBroadPhase();
        _entity_to_collider_map[id] = entity;
        return id.GetIndexAndSequenceNumber();
    }

    uint32_t Physics::create_infinite_ground_plane(glm::vec3 plane_normal, float constant)
    {
        auto id = _physics_system.GetBodyInterface().CreateAndAddBody(
            BodyCreationSettings(
                new PlaneShape(Plane(JPH::Vec3(plane_normal.x, plane_normal.y, plane_normal.z).Normalized(), constant),
                               nullptr, 100), RVec3(0, 0, 0), Quat::sIdentity(), EMotionType::Static,
                Layers::NON_MOVING), EActivation::DontActivate);
        _colliders_static.emplace_back(id);
        return id.GetIndexAndSequenceNumber();
    }

    uint32_t Physics::create_rigidbody(Entity entity)
     {
         JPH::Ref<Shape> result_shape;
         if (entity.has_component<ConvexMeshColliderComponent>())
         {
             auto mesh = AssetManager::get_mesh_by_name(entity.get_component<ConvexMeshColliderComponent>().mesh_name);
             if (!mesh)
             {
                 LOG_ERROR("No mesh found with name %s",
                           entity.get_component<ConvexMeshColliderComponent>().mesh_name.c_str());
                 return 0;
             }
             //TODO: BAKE ME BAKE ME BAKE ME
             auto &in_verts = mesh->get_vertices();
             std::vector<JPH::Vec3> convex_hull_verts;
             convex_hull_verts.reserve(in_verts.size());
             for (const auto &v: in_verts)
             {
                 convex_hull_verts.emplace_back(v.position.x, v.position.y, v.position.z);
             }

             JPH::ConvexHullShapeSettings hull_settings(convex_hull_verts.data(), convex_hull_verts.size());
             auto hull_result = hull_settings.Create();
             if (!hull_result.IsValid())
             {
                 LOG_ERROR("Error creating convex hull %s", hull_result.GetError().c_str());
                 return 0;
             }

             result_shape = hull_result.Get();
         }

         //TODO: scale me
         BodyCreationSettings settings(result_shape, PhysicsUtil::glm_vec3_to_jph_vec3(entity.get_transform().position),
                                       PhysicsUtil::glm_quat_to_jph_quat(entity.get_transform().rotation), EMotionType::Dynamic,
                                       Layers::MOVING);
         auto body = _physics_system.GetBodyInterface().CreateAndAddBody(settings, EActivation::Activate);
         _colliders_static.emplace_back(body);
         _entity_to_collider_map[body] = entity;
         return body.GetIndexAndSequenceNumber();
     }
}