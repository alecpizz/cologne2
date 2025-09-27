#include "Physics.h"
#include <engine/core/Engine.h>
#include <engine/core/Input.h>
#include "RaycastHitInfo.h"
#include <engine/renderer/Renderer.h>
#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Renderer/DebugRendererSimple.h>

//
// Created by alecpizz on 9/14/25.
//
namespace cologne
{
    using namespace JPH;

    class PhysDebugRenderer : public JPH::DebugRendererSimple
    {
    public:
        void DrawLine(RVec3Arg inFrom, RVec3Arg inTo, ColorArg inColor) override
        {
            Engine::get_renderer()->draw_line(glm::vec3(inFrom.GetX(), inFrom.GetY(), inFrom.GetZ()),
                                              glm::vec3(inTo.GetX(), inTo.GetY(), inTo.GetZ()),
                                              glm::vec3(inColor.r, inColor.g, inColor.b));
        }

        void DrawTriangle(RVec3Arg inV1, RVec3Arg inV2, RVec3Arg inV3, ColorArg inColor,
                          ECastShadow inCastShadow) override
        {
            Engine::get_renderer()->draw_triangle(glm::vec3(inV1.GetX(), inV1.GetY(), inV1.GetZ()),
                                                  glm::vec3(inV2.GetX(), inV2.GetY(), inV2.GetZ()),
                                                  glm::vec3(inV3.GetX(), inV3.GetY(), inV3.GetZ()),
                                                  glm::vec3(inColor.r, inColor.g, inColor.b));
        }

        void DrawText3D(RVec3Arg inPosition, const string_view &inString, ColorArg inColor, float inHeight)
        {
            //TODO!
        }
    };

    bool Physics::_drawing = false;

    void Physics::draw()
    {
        if (key_pressed(Input::Key::P))
        {
            _drawing = !_drawing;
        }
        if (_drawing)
        {
            static PhysDebugRenderer debug_renderer;
            BodyManager::DrawSettings draw_settings;
            draw_settings.mDrawShape = true;
            draw_settings.mDrawShapeWireframe = true;
            _physics_system.DrawBodies(draw_settings, &debug_renderer);
        }
    }
}
