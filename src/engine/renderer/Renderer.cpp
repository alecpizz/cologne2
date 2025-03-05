//
// Created by alecpizz on 3/1/2025.
//

#include "Renderer.h"
#include "../Scene.h"
#include "VertexAttribute.h"

namespace goon
{
    struct Renderer::Impl
    {
        void init()
        {

        }
    };
    Renderer::~Renderer()
    {
        delete _impl;
    }

    void Renderer::render_scene(Scene &scene)
    {

    }

    Renderer::Renderer()
    {
        _impl = new Impl();
        _impl->init();
    }
}
