//
// Created by alecpizz on 3/1/2025.
//

#include "Engine.h"
#include "engine/imguiThemes.h"
#include "gpch.h"


namespace goon
{
    struct Engine::Impl
    {
        std::unique_ptr<Window> window = nullptr;
        std::unique_ptr<Renderer> renderer = nullptr;
        std::unique_ptr<EventManager> event_manager = nullptr;
        bool running = true;
    };

    Engine::Engine()
    {
        _impl = new Impl();
        LOG_INFO("Starting up engine. the world is a shit place, and this is a shit engine. good luck!");
    }

    Engine::~Engine()
    {
        // ImGui_ImplOpenGL3_Shutdown();
        // ImGui_ImplSDL3_Shutdown();
        // ImGui::DestroyContext();
        delete _impl;
    }

    Renderer * Engine::get_renderer() const
    {
        return _impl->renderer.get();
    }

    Window * Engine::get_window() const
    {
        return _impl->window.get();
    }

    EventManager * Engine::get_event_manager() const
    {
        return _impl->event_manager.get();
    }

    bool Engine::init(uint32_t width, uint32_t height)
    {
        //TODO: Window class
        _impl->window = std::unique_ptr<Window>(new Window(width, height));
        _impl->renderer = std::unique_ptr<Renderer>(new Renderer());
        _impl->event_manager = std::unique_ptr<EventManager>(new EventManager());

        if (_impl->window == nullptr || _impl->renderer == nullptr)
        {
            LOG_ERROR("Failed to initialize window or renderer!");
            return false;
        }

        // ImGui::CreateContext();
        // imguiThemes::green();
        //
        // ImGuiIO &io = ImGui::GetIO();
        // (void) io;
        // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
        // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows
        //
        // ImGuiStyle &style = ImGui::GetStyle();
        // if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        // {
        //     //style.WindowRounding = 0.0f;
        //     style.Colors[ImGuiCol_WindowBg].w = 0.f;
        //     style.Colors[ImGuiCol_DockingEmptyBg].w = 0.f;
        // }
        //
        // ImGui_ImplSDL3_InitForOpenGL(_impl->window, _impl->context);
        // ImGui_ImplOpenGL3_Init("#version 460");

        return true;
    }

    void Engine::run()
    {
        // const ImGuiIO &io = ImGui::GetIO();
        // (void) io;
        _impl->running = true;
        while (!_impl->event_manager->should_quit())
        {
            _impl->window->resize();

            _impl->event_manager->poll_events();

            // ImGui_ImplOpenGL3_NewFrame();
            // ImGui_ImplSDL3_NewFrame();
            // ImGui::NewFrame();
            // // Create a docking space
            // ImGui::DockSpaceOverViewport(ImGui::GetMainViewport()->ID);
            //
            _impl->window->clear();
            // // Example ImGui window
            // ImGui::Begin("gooning window");
            //
            // ImGui::Text("i'm sdling my window!");
            //
            // ImGui::End();
            //
            //
            // // testing opengl
            //
            //
            // ImGui::Render();
            // ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            //
            // //view port stuff



            _impl->window->present();
        }
    }
} // goon
