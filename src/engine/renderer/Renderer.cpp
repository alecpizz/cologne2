//
// Created by alecpizz on 3/1/2025.
//

#include "Renderer.h"

#include <engine/Engine.h>

#include "../Scene.h"
#include "Shader.h"

namespace goon
{
    struct Renderer::Impl
    {
        std::unique_ptr<Shader> lit_shader = nullptr;
        void init()
        {
            lit_shader = std::make_unique<Shader>(RESOURCES_PATH "shaders/lit.vert", RESOURCES_PATH "shaders/lit.frag");
            glEnable(GL_DEPTH_TEST);
        }
    };
    Renderer::~Renderer()
    {
        delete _impl;
    }

    void Renderer::render_scene(Scene &scene)
    {
        _impl->lit_shader->bind();

        //TEMP
        _impl->lit_shader->set_mat4("projection", &Engine::get_camera()->get_projection_matrix()[0][0]);
        _impl->lit_shader->set_mat4("view",  &Engine::get_camera()->get_view_matrix()[0][0]);
        _impl->lit_shader->set_int("texture_diffuse1", 0);
        for (size_t i = 0; i < scene.get_model_count(); i++)
        {
            auto& modelM = scene.get_models()[i];
            _impl->lit_shader->set_mat4("model", &modelM.get_transform()->get_model_matrix()[0][0]);
            modelM.get_albedo()->bind(0);
            for (size_t j = 0; j < modelM.get_num_meshes(); j++)
            {
                modelM.get_meshes()[j].draw();
            }
        }
    }

    Renderer::Renderer()
    {
        _impl = new Impl();
        _impl->init();
    }
}
