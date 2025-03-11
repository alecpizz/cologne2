//
// Created by alecpizz on 3/1/2025.
//

#include "Renderer.h"
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
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 16.0f / 9.0f, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(glm::vec3(0.0), glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, 1.0, 0.0));
        _impl->lit_shader->set_mat4("projection", &projection[0][0]);
        _impl->lit_shader->set_mat4("view", &view[0][0]);

        // render the loaded model
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 10.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(1.0f));	// it's a bit too big for our scene, so scale it down
        static float angle = 0.0f;
        angle += 0.05f;
        model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        _impl->lit_shader->set_mat4("model", &model[0][0]);
        for (size_t i = 0; i < scene.get_model_count(); i++)
        {
            auto modelM = scene.get_models()[i];
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
