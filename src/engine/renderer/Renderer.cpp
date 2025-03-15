//
// Created by alecpizz on 3/1/2025.
//

#include "Renderer.h"

#include <engine/Engine.h>
#include <engine/Input.h>

#include "Light.h"
#include "../Scene.h"
#include "Shader.h"

namespace goon
{
    struct Renderer::Impl
    {
        std::unique_ptr<Shader> lit_shader = nullptr;
        std::vector<Light> lights;

        void init()
        {
            glEnable(GL_DEPTH_TEST);
        }

        void init_lit_shader()
        {
            lit_shader = std::make_unique<Shader>(RESOURCES_PATH "shaders/lit.vert",
                                                  RESOURCES_PATH "shaders/lit.frag");
            lit_shader->bind();
            lit_shader->set_int("texture_albedo", 0);
            lit_shader->set_int("texture_normal", 1);
            lit_shader->set_int("texture_roughness", 2);
            lit_shader->set_int("texture_metallic", 3);
            lit_shader->set_int("texture_ao", 4);
        }

        void add_light(Light light)
        {
            lights.emplace_back(light);
            size_t light_idx = lights.size() - 1;
            lit_shader->bind();
            lit_shader->set_vec3(std::string("lights[" + std::to_string(light_idx) + "].position").c_str(),
                                     glm::value_ptr(lights[light_idx].position));
            lit_shader->set_vec3(std::string("lights[" + std::to_string(light_idx) + "].color").c_str(),
                                 glm::value_ptr(lights[light_idx].color));
            lit_shader->set_float(std::string("lights[" + std::to_string(light_idx) + "].radius").c_str(),
                                 lights[light_idx].radius);
            lit_shader->set_int(std::string("lights[" + std::to_string(light_idx) + "].type").c_str(),
                                 lights[light_idx].type);
            lit_shader->set_float(std::string("lights[" + std::to_string(light_idx) + "].strength").c_str(),
                                 lights[light_idx].strength);
            lit_shader->set_vec3(std::string("lights[" + std::to_string(light_idx) + "].direction").c_str(),
                                    glm::value_ptr(lights[light_idx].direction));
            lit_shader->set_int("num_lights", lights.size());
        }

        void bind_lights(const Shader& shader)
        {
            shader.bind();
            for (size_t i = 0; i < lights.size(); i++)
            {
                lit_shader->set_vec3(std::string("lights[" + std::to_string(i) + "].position").c_str(),
                                     glm::value_ptr(lights[i].position));
                lit_shader->set_vec3(std::string("lights[" + std::to_string(i) + "].color").c_str(),
                                     glm::value_ptr(lights[i].color));
                lit_shader->set_float(std::string("lights[" + std::to_string(i) + "].radius").c_str(),
                                     lights[i].radius);
                lit_shader->set_int(std::string("lights[" + std::to_string(i) + "].type").c_str(),
                                     lights[i].type);
                lit_shader->set_float(std::string("lights[" + std::to_string(i) + "].strength").c_str(),
                                     lights[i].strength);
                lit_shader->set_vec3(std::string("lights[" + std::to_string(i) + "].direction").c_str(),
                                        glm::value_ptr(lights[i].direction));
            }
            lit_shader->set_int("num_lights", lights.size());
        }


    };

    Renderer::~Renderer()
    {
        delete _impl;
    }

    void Renderer::render_scene(Scene &scene)
    {
        //indirect pass
        //shadow maps
        //geometry pass
        //culling pass
        //lighting pass
        //skybox pass
        //post processing pass
        //any debug visuals pass

        if (goon::Input::key_pressed(Input::Key::H))
        {
            //hot reload shaders
           reload_shaders();
        }

        _impl->lit_shader->bind();
        _impl->lit_shader->set_vec3("camera_pos", glm::value_ptr(Engine::get_camera()->get_position()));
        //TEMP
        _impl->lit_shader->set_mat4("projection", &Engine::get_camera()->get_projection_matrix()[0][0]);
        _impl->lit_shader->set_mat4("view", &Engine::get_camera()->get_view_matrix()[0][0]);
        for (size_t i = 0; i < scene.get_model_count(); i++)
        {
            auto &modelM = scene.get_models()[i];
            _impl->lit_shader->set_mat4("model", &modelM.get_transform()->get_model_matrix()[0][0]);
            modelM.get_albedo()->bind(0);
            modelM.get_normal()->bind(1);
            modelM.get_roughness()->bind(2);
            modelM.get_metallic()->bind(3);
            modelM.get_ao()->bind(4);
            for (size_t j = 0; j < modelM.get_num_meshes(); j++)
            {
                modelM.get_meshes()[j].draw();
            }
        }
    }

    void Renderer::reload_shaders()
    {
        _impl->lit_shader.release();
        _impl->init_lit_shader();
        _impl->bind_lights(*_impl->lit_shader);
        LOG_INFO("RELOADED SHADERS");
    }

    Renderer::Renderer()
    {
        _impl = new Impl();
        _impl->init();
        _impl->init_lit_shader();
        _impl->add_light(Light(glm::vec3(-10.0f, 10.0f, 10.0f), glm::vec3(0.0f),
                               glm::vec3(300.0f, 300.0f, 300.0f), 6.0f, 1.0f,
                               LightType::Point));
        _impl->add_light(Light(glm::vec3(10.0f, 10.0f, 10.0f), glm::vec3(0.0f),
                               glm::vec3(300.0f, 300.0f, 300.0f), 6.0f, 1.0f,
                               LightType::Point));
        _impl->add_light(Light(glm::vec3(-10.0f, -10.0f, 10.0f), glm::vec3(0.0f),
                               glm::vec3(300.0f, 300.0f, 300.0f), 6.0f, 1.0f,
                               LightType::Point));
        _impl->add_light(Light(glm::vec3(10.0f, -10.0f, 10.0f), glm::vec3(0.0f),
                               glm::vec3(300.0f, 300.0f, 300.0f), 6.0f, 1.0f,
                               LightType::Point));
    }
}
