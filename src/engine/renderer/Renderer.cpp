//
// Created by alecpizz on 3/1/2025.
//

#include "Renderer.h"

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

    Renderer::Renderer()
    {
        _impl = new Impl();
        _impl->init();
    }
}
