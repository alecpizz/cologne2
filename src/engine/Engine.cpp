//
// Created by alecpizz on 3/1/2025.
//

#include "Engine.h"
#include <iostream>
#include <SDL3/SDL.h>
#include <glad/glad.h>
#include "openglErrorReporting.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_opengl3.h"
#include "engine/imguiThemes.h"
#include <assimp/Importer.hpp>

namespace goon
{
    struct Engine::Impl
    {
        SDL_Window *window = nullptr;
        SDL_GLContext context = nullptr;
        bool running = true;
    };

    Engine::Engine()
    {
        _impl = new Impl();
    }

    Engine::~Engine()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
        SDL_DestroyWindow(_impl->window);
        SDL_Quit();
        delete _impl;
    }

    bool Engine::init(uint32_t width, uint32_t height)
    {
        if (!SDL_Init(SDL_INIT_VIDEO))
        {
            std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
            return false;
        }

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

#ifdef __APPLE__
        // apple moment...
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
#endif

        _impl->window = SDL_CreateWindow("gooner josh", 1280, 720, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
        if (_impl->window == nullptr)
        {
            std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
            SDL_Quit();
            return false;
        }

        _impl->context = SDL_GL_CreateContext(_impl->window);
        if (_impl->context == nullptr)
        {
            std::cerr << "SDL_GL_CreateContext Error: " << SDL_GetError() << std::endl;
            SDL_DestroyWindow(_impl->window);
            SDL_Quit();
            return false;
        }

        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress)))
        {
            std::cerr << "Failed to initialize GLAD" << std::endl;
            SDL_GL_DestroyContext(_impl->context);
            SDL_DestroyWindow(_impl->window);
            SDL_Quit();
            return false;
        }

        enableReportGlErrors();

        ImGui::CreateContext();
        imguiThemes::green();

        ImGuiIO &io = ImGui::GetIO();
        (void) io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows

        ImGuiStyle &style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            //style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 0.f;
            style.Colors[ImGuiCol_DockingEmptyBg].w = 0.f;
        }

        ImGui_ImplSDL3_InitForOpenGL(_impl->window, _impl->context);
        ImGui_ImplOpenGL3_Init("#version 460");

        return true;
    }

    void Engine::run()
    {
        const ImGuiIO &io = ImGui::GetIO();
        (void) io;
        _impl->running = true;
        while (_impl->running)
        {
            int w = 0, h = 0;
            SDL_GetWindowSize(_impl->window, &w, &h);

            glViewport(0, 0, w, h);

            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                ImGui_ImplSDL3_ProcessEvent(&event);
                if (event.type == SDL_EVENT_QUIT)
                {
                    _impl->running = false;
                }

                if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
                {
                    if (event.window.windowID == SDL_GetWindowID(_impl->window))
                    {
                        _impl->running = false;
                    }
                }
            }

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplSDL3_NewFrame();
            ImGui::NewFrame();
            // Create a docking space
            ImGui::DockSpaceOverViewport(ImGui::GetMainViewport()->ID);

            glClear(GL_COLOR_BUFFER_BIT);
            glClearColor(0.0f, 0.0f, 0.2f, 1.0f);

            // Example ImGui window
            ImGui::Begin("gooning window");

            ImGui::Text("i'm sdling my window!");

            ImGui::End();


            // testing opengl


            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            //view port stuff
            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            {
                SDL_Window *backup_current_window = SDL_GL_GetCurrentWindow();
                SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
                SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
            }


            SDL_GL_SwapWindow(_impl->window);
        }
    }
} // goon
