//
// Created by alecpizz on 9/13/25.
//
#pragma once
#include <DebugDraw.h>

namespace cologne
{
    class NavmeshDebugDrawer : public duDebugDraw
    {
    public:
        NavmeshDebugDrawer();

        void depthMask(bool state) override;
        void texture(bool state) override;

        void begin(duDebugDrawPrimitives prim, float size) override;
        void vertex(const float *pos, unsigned int color) override;
        void vertex(const float x, const float y, const float z, unsigned int color) override;
        void vertex(const float *pos, unsigned color, const float *uv) override;
        void vertex(const float x, const float y, const float z, unsigned color, const float u, const float v) override;
        void end() override;

    private:
        static glm::vec4 du_to_glm_color(unsigned int color);
    private:
        duDebugDrawPrimitives _current_primitive;

    };
}
