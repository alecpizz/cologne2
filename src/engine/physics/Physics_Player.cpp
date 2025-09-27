#include "Physics.h"
#include "PhysicsUtil.h"

#include "RaycastHitInfo.h"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Character/CharacterVirtual.h>
#include <Jolt/Physics/Collision/Shape/PlaneShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>

#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <engine/asset_manager/AssetManager.h>

#include "Jolt/Math/Vec3.h"
#include "Jolt/Physics/Ragdoll/Ragdoll.h"

//
// Created by alecpizz on 9/14/25.
//
namespace cologne
{
    using namespace JPH;

    struct PhysicsPlayer
    {
        JPH::RefConst<JPH::Shape> standing_shape;
        JPH::RefConst<JPH::Shape> crouching_shape;
        JPH::RefConst<JPH::Shape> inner_standing_shape;
        JPH::RefConst<JPH::Shape> inner_crouching_shape;
        JPH::Ref<JPH::CharacterVirtual> character;
        glm::vec3 character_position;
    };

    std::unordered_map<uint32_t, PhysicsPlayer> Physics::_physics_players;

    void Physics::cleanup_players()
    {
        _physics_players.clear();
    }

    uint32_t Physics::create_player(PlayerCreateInfo &info)
    {
        PhysicsPlayer player;
        player.standing_shape = JPH::RotatedTranslatedShapeSettings(
            Vec3(0, 0.5f * info.height_standing + info.radius_standing, 0), JPH::Quat::sIdentity(),
            new JPH::CapsuleShape(0.5f * info.height_standing, info.radius_standing)).Create().Get();
        player.inner_standing_shape = JPH::RotatedTranslatedShapeSettings(
            JPH::Vec3(0, 0.5f * info.height_standing + info.radius_standing, 0), JPH::Quat::sIdentity(),
            new JPH::CapsuleShape(0.5f * info.inner_friction * info.height_standing,
                                  info.inner_friction * info.radius_standing)).Create().Get();

        JPH::Ref<JPH::CharacterVirtualSettings> settings = new JPH::CharacterVirtualSettings();
        settings->mMaxSlopeAngle = glm::radians(45.0f);
        settings->mMaxStrength = 100.0f;
        settings->mShape = player.standing_shape;
        settings->mBackFaceMode = JPH::EBackFaceMode::CollideWithBackFaces;
        settings->mCharacterPadding = 0.02f;
        settings->mPenetrationRecoverySpeed = 1.0f;
        settings->mPredictiveContactDistance = 0.1f;
        settings->mSupportingVolume = JPH::Plane(JPH::Vec3::sAxisY(), -info.radius_standing);
        settings->mEnhancedInternalEdgeRemoval = false;
        settings->mInnerBodyShape = player.inner_standing_shape;
        settings->mInnerBodyLayer = cologne::Physics::PLAYER;


        player.character = new JPH::CharacterVirtual(settings, PhysicsUtil::glm_vec3_to_vec3(info.position),
                                                     JPH::Quat::sIdentity(), 0, &_physics_system);
        uint32_t id = player.character->GetID().GetValue();
        _physics_players[id] = player;
        return id;
    }

    void Physics::update_players(float dt)
    {
        for (auto &physics_player: _physics_players)
        {
            auto &p = physics_player.second;
            auto &character = p.character;
            JPH::CharacterVirtual::ExtendedUpdateSettings update_settings;
            update_settings.mStickToFloorStepDown = -character->GetUp() * update_settings.mStickToFloorStepDown.
                                                    Length();
            update_settings.mWalkStairsStepUp = character->GetUp() * update_settings.mWalkStairsStepUp.Length();
            character->ExtendedUpdate(
                dt, character->GetUp() * _physics_system.GetGravity().Length(), update_settings,
                _physics_system.GetDefaultBroadPhaseLayerFilter(Layers::PLAYER),
                _physics_system.GetDefaultLayerFilter(Layers::PLAYER),
                {},
                {},
                *_temp_allocator);

            p.character_position = glm::vec3(character->GetPosition().GetX(), character->GetPosition().GetY(),
                                             character->GetPosition().GetZ());
        }
    }


    glm::vec3 Physics::get_player_position(uint32_t id)
    {
        if (!_physics_players.contains(id))
        {
            LOG_ERROR("NO PLAYER WITH id %d", id);
            return glm::vec3(0.0f);
        }
        return _physics_players[id].character_position;
    }

    void Physics::move_player(uint32_t id, PlayerMovementCommand cmd)
    {
        if (!_physics_players.contains(id))
        {
            LOG_ERROR("NO PLAYER WITH id %d", id);
            return;
        }
        auto &character = _physics_players[id].character;
        character->SetUp(PhysicsUtil::glm_vec3_to_vec3(cmd.up));
        character->SetRotation(PhysicsUtil::glm_quat_to_jph_quat(cmd.rotation));
        character->SetLinearVelocity(PhysicsUtil::glm_vec3_to_vec3(cmd.movement));
    }

    void Physics::teleport_player(uint32_t id, glm::vec3 position)
    {
        if (!_physics_players.contains(id))
        {
            return;
        }

        auto &player = _physics_players[id];
        auto &character = player.character;
        character->SetPosition(PhysicsUtil::glm_vec3_to_vec3(position));
    }

    bool Physics::player_is_grounded(uint32_t id)
    {
        if (!_physics_players.contains(id))
        {
            LOG_ERROR("NO PLAYER WITH id %d", id);
            return false;
        }
        return _physics_players[id].character->GetGroundState() == JPH::CharacterVirtual::EGroundState::OnGround;
    }

    bool Physics::player_is_supported(uint32_t id)
    {
        if (!_physics_players.contains(id))
        {
            LOG_ERROR("NO PLAYER WITH id %d", id);
            return false;
        }
        return _physics_players[id].character->IsSupported();
    }

    bool Physics::slope_to_steep_for_player(uint32_t id)
    {
        if (!_physics_players.contains(id))
        {
            LOG_ERROR("NO PLAYER WITH id %d", id);
            return false;
        }
        return _physics_players[id].character->IsSlopeTooSteep(_physics_players[id].character->GetGroundNormal());
    }

    glm::vec3 Physics::get_gravity()
    {
        return PhysicsUtil::jph_vec3_to_glm_vec3(_physics_system.GetGravity());
    }


    glm::vec3 Physics::get_player_velocity(uint32_t id)
    {
        if (!_physics_players.contains(id))
        {
            LOG_ERROR("NO PLAYER WITH id %d", id);
            return glm::vec3(0.0F);
        }
        return PhysicsUtil::jph_vec3_to_glm_vec3(_physics_players[id].character->GetLinearVelocity());
    }

    glm::vec3 Physics::get_player_ground_velocity(uint32_t id)
    {
        if (!_physics_players.contains(id))
        {
            LOG_ERROR("NO PLAYER WITH id %d", id);
            return glm::vec3(0.0F);
        }
        return PhysicsUtil::jph_vec3_to_glm_vec3(_physics_players[id].character->GetGroundVelocity());
    }
}
