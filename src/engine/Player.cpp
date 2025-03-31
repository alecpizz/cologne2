//
// Created by alecpizz on 3/26/2025.
//

#include "Player.h"
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Character/Character.h>
#include <Jolt/Physics/Character/CharacterVirtual.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>

#include "Audio.h"
#include "Engine.h"
#include "Input.h"
#include "Physics.h"

namespace goon
{
    struct Player::Impl
    {
        float height_standing = 1.35f;
        float radius_standing = 0.3f;
        float height_crouching = 0.8f;
        float radius_crouching = 0.3f;
        float inner_friction = 0.9f;
        float character_speed = 3.5f;
        float jump_speed = 4.0f;
        glm::vec3 position;

        JPH::RefConst<JPH::Shape> standing_shape;
        JPH::RefConst<JPH::Shape> crouching_shape;
        JPH::RefConst<JPH::Shape> inner_standing_shape;
        JPH::RefConst<JPH::Shape> inner_crouching_shape;

        JPH::Ref<JPH::CharacterVirtual> character;

        bool jump = false;
        bool was_jump = false;
        bool switch_stance = false;
        bool was_switch_stance = false;
        bool allow_sliding = false;
        bool grounded = true;
        bool grounded_last_frame = true;
        bool footstep_played = false;
        float bob_time = 0.0f;
        float bob_offset = 0.0f;
        JPH::Vec3 input = JPH::Vec3::sZero();
        JPH::Vec3 desired_velocity = JPH::Vec3::sZero();
        std::vector<std::string> footstep_sounds;

        void init()
        {
            standing_shape = JPH::RotatedTranslatedShapeSettings(
                JPH::Vec3(0, 0.5f * height_standing + radius_standing, 0), JPH::Quat::sIdentity(),
                new JPH::CapsuleShape(0.5f * height_standing, radius_standing)).Create().Get();
            inner_standing_shape = JPH::RotatedTranslatedShapeSettings(
                JPH::Vec3(0, 0.5f * height_standing + radius_standing, 0), JPH::Quat::sIdentity(),
                new JPH::CapsuleShape(0.5f * inner_friction * height_standing,
                                      inner_friction * radius_standing)).Create().Get();

            JPH::Ref<JPH::CharacterVirtualSettings> settings = new JPH::CharacterVirtualSettings();
            settings->mMaxSlopeAngle = glm::radians(45.0f);
            settings->mMaxStrength = 100.0f;
            settings->mShape = standing_shape;
            settings->mBackFaceMode = JPH::EBackFaceMode::CollideWithBackFaces;
            settings->mCharacterPadding = 0.02f;
            settings->mPenetrationRecoverySpeed = 1.0f;
            settings->mPredictiveContactDistance = 0.1f;
            settings->mSupportingVolume = JPH::Plane(JPH::Vec3::sAxisY(), -radius_standing);
            settings->mEnhancedInternalEdgeRemoval = false;
            settings->mInnerBodyShape = inner_standing_shape;
            settings->mInnerBodyLayer = goon::physics::NON_MOVING;

            character = new JPH::CharacterVirtual(settings, JPH::RVec3::sZero(), JPH::Quat::sIdentity(), 0,
                                                  goon::physics::get_physics_system());
            footstep_sounds.emplace_back(RESOURCES_PATH "sounds/player_step_1.wav");
            footstep_sounds.emplace_back(RESOURCES_PATH "sounds/player_step_2.wav");
            footstep_sounds.emplace_back(RESOURCES_PATH "sounds/player_step_3.wav");
            footstep_sounds.emplace_back(RESOURCES_PATH "sounds/player_step_4.wav");
            for (const auto& footstep_sound : footstep_sounds)
            {
                Audio::add_sound(footstep_sound.c_str());
            }
        }

        void handle_input(float dt)
        {
            float x = 0.0f;
            float y = 0.0f;
            if (goon::Input::key_down(goon::Input::Key::W))
            {
                x += 1.0f;
            }
            if (goon::Input::key_down(goon::Input::Key::S))
            {
                x -= 1.0f;
            }
            if (goon::Input::key_down(goon::Input::Key::A))
            {
                y -= 1.0f;
            }
            if (goon::Input::key_down(goon::Input::Key::D))
            {
                y += 1.0f;
            }
            bool jump = goon::Input::key_pressed(goon::Input::Key::Space);
            bool crouch = goon::Input::key_pressed(goon::Input::Key::LeftCtrl);
            JPH::Vec3 movement = JPH::Vec3(x, 0.0f, y);
            if (!movement.IsNearZero())
            {
                movement = movement.Normalized();
            }
            glm::vec3 cam_fwd = Engine::get_camera()->get_forward();
            cam_fwd.y = 0.0f;
            glm::normalize(cam_fwd);
            JPH::Quat rotation = JPH::Quat::sFromTo(JPH::Vec3::sAxisX(), JPH::Vec3(cam_fwd.x, cam_fwd.y, cam_fwd.z));
            movement = rotation * movement;


            desired_velocity = movement * character_speed;
            if (character->IsSupported())
            {
                allow_sliding = !movement.IsNearZero();
            } else
            {
                allow_sliding = false;
            }
            JPH::Quat char_up_rotation = JPH::Quat::sEulerAngles(JPH::Vec3(0.0f, 0.0f, 0.0f));
            character->SetUp(char_up_rotation.RotateAxisY());
            character->SetRotation(char_up_rotation);

            character->UpdateGroundVelocity();

            JPH::Vec3 current_vertical_velocity =
                    character->GetLinearVelocity().Dot(character->GetUp()) * character->GetUp();
            JPH::Vec3 ground_velocity = character->GetGroundVelocity();
            JPH::Vec3 new_velocity;

            bool moving_towards_ground = (current_vertical_velocity.GetY() - ground_velocity.GetY()) < 0.1f;
            grounded_last_frame = grounded;
            grounded = character->GetGroundState() == JPH::CharacterVirtual::EGroundState::OnGround;
            if (grounded && !character->
                IsSlopeTooSteep(character->GetGroundNormal()))
            {
                new_velocity = ground_velocity;
                if (jump && moving_towards_ground)
                {
                    new_velocity += jump_speed * character->GetUp();
                }
            } else
            {
                new_velocity = current_vertical_velocity;
            }

            new_velocity += (char_up_rotation * physics::get_physics_system()->GetGravity() * dt);
            new_velocity += (char_up_rotation * desired_velocity);

            character->SetLinearVelocity(new_velocity);

            // if (crouch)
            // {
            //     bool is_standing = character->GetShape() == standing_shape;
            //     auto shape = is_standing ? crouching_shape : standing_shape;
            //     if (character->SetShape(shape, 1.5f * physics::get_physics_system()->GetPhysicsSettings().mPenetrationSlop, physics::get_physics_system()->GetDefaultLayerFilter(physics::MOVING), ))
            // }
            play_footsteps(dt);
        }

        void play_footsteps(float dt)
        {
            auto vel = character->GetLinearVelocity();
            vel.SetY(0.0f);
            if (vel.IsNearZero())
            {
                return;
            }

            bob_time += dt;
            bob_offset = glm::sin(bob_time * 4.5f * character_speed) * 0.05f;
            if (bob_offset < -0.04f && !footstep_played && grounded)
            {
                auto idx = rand() % 4;
                Audio::play_sound(footstep_sounds[idx].c_str(), 30);
                footstep_played = true;
            }

            if (bob_offset > 0.0f)
            {
                footstep_played = false;
            }

            if (grounded && !grounded_last_frame)
            {
                auto idx = rand() % 4;
                Audio::play_sound(footstep_sounds[idx].c_str(), 30);
                bob_time = 0.0f;
            }
        }
    };

    Player::Player()
    {
        _impl = new Impl();
        _impl->init();

    }

    Player::~Player()
    {
        delete _impl;
    }

    glm::vec3 Player::get_camera_position()
    {
        return _impl->position + glm::vec3(0.0f, 1.45f + _impl->bob_offset, 0.0f);
    }

    void Player::update(float dt)
    {
        if (Engine::get_camera()->is_free_cam())
        {
            return;
        }
        JPH::CharacterVirtual::ExtendedUpdateSettings update_settings;
        update_settings.mStickToFloorStepDown = -_impl->character->GetUp() * update_settings.mStickToFloorStepDown.
                                                Length();
        update_settings.mWalkStairsStepUp = _impl->character->GetUp() * update_settings.mWalkStairsStepUp.Length();
        _impl->character->ExtendedUpdate(
            dt, _impl->character->GetUp() * physics::get_physics_system()->GetGravity().Length(), update_settings,
            physics::get_physics_system()->GetDefaultBroadPhaseLayerFilter(1),
            physics::get_physics_system()->GetDefaultLayerFilter(1),
            {},
            {},
            *physics::get_temp_allocator());

        _impl->position = glm::vec3(_impl->character->GetPosition().GetX(), _impl->character->GetPosition().GetY(),
                                    _impl->character->GetPosition().GetZ());
        _impl->handle_input(dt);
    }
}
