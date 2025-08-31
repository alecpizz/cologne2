//
// Created by alecpizz on 3/1/2025.
//

#include "Window.h"
#include <engine/util/DebugScope.h>
#include <SDL3/SDL.h>

namespace cologne
{
    struct Window::Impl
    {
        SDL_Window *window = nullptr;
        SDL_GLContext context = nullptr;
        uint32_t width = 0;
        uint32_t height = 0;
        bool mouse_visible = false;


        void init(uint32_t w, uint32_t h)
        {
            width = w;
            height = h;
            if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
            {
                LOG_ERROR("SDL_Init Error: %s", SDL_GetError());
                return;
            }

            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
            SDL_SetCurrentThreadPriority(SDL_THREAD_PRIORITY_HIGH);

            //no point in doing msaa when there's deferred rendering
            // SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
            // SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);
            window = SDL_CreateWindow("cologne 2", w, h, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
            if (window == nullptr)
            {
                LOG_ERROR("SDL_CreateWindow Error: %s", SDL_GetError());
                SDL_Quit();
                return;
            }
            // SDL_SetWindowBordered(window, false);
            SDL_RaiseWindow(window);
            context = SDL_GL_CreateContext(window);

            if (context == nullptr)
            {
                LOG_ERROR("SDL_GL_CreateContext Error: %s", SDL_GetError());
                SDL_DestroyWindow(window);
                SDL_Quit();
            }

            if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress)))
            {
                LOG_ERROR("GLAD_GL_GetProcAddress Error: %s", SDL_GetError());
                SDL_GL_DestroyContext(context);
                SDL_DestroyWindow(window);
                SDL_Quit();
            }

            ImGui_ImplSDL3_InitForOpenGL(window, context);
            ImGui_ImplOpenGL3_Init("#version 460");
            const GLubyte *vendor = glGetString(GL_VENDOR); // Returns the vendor
            const GLubyte *renderer = glGetString(GL_RENDERER); // Returns a hint to the model
            LOG_INFO("Created window for GPU: %s %s", vendor, renderer);
        }
    };

    uint32_t Window::get_width() const
    {
        return _impl->width;
    }

    uint32_t Window::get_height() const
    {
        return _impl->height;
    }

    glm::vec2 Window::get_dimensions() const
    {
        return glm::vec2(get_height(), get_width());
    }

    void Window::clear() const
    {
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void Window::resize() const
    {
        int w = 0, h = 0;
        SDL_GetWindowSize(_impl->window, &w, &h);
        _impl->width = w;
        _impl->height = h;
    }

    void Window::maximize() const
    {
        static bool maximized = false;
        maximized = !maximized;
        if (maximized)
        {
            SDL_MaximizeWindow(_impl->window);
        }
        else
        {
            SDL_RestoreWindow(_impl->window);
        }
    }

    void Window::present() const
    {
        SDL_GL_SwapWindow(_impl->window);
    }

    void Window::minimize()
    {
        SDL_MinimizeWindow(_impl->window);
    }

    void Window::hide_mouse() const
    {
        SDL_SetWindowRelativeMouseMode(_impl->window, true);
        _impl->mouse_visible = false;
    }

    void Window::show_mouse() const
    {
        SDL_SetWindowRelativeMouseMode(_impl->window, false);
        _impl->mouse_visible = true;
    }

    bool Window::mouse_visible() const
    {
        return _impl->mouse_visible;
    }

    void Window::set_cursor_pos(float x, float y) const
    {
        SDL_WarpMouseInWindow(_impl->window, x, y);
    }

    void Window::show_file_dialogue_window(const std::unordered_map<std::string, std::string> &filters,
                                           const std::string &path, void *callback)
    {
        std::vector<SDL_DialogFileFilter> dialog_file_filters;
        for (auto &filter: filters)
        {
            dialog_file_filters.emplace_back(filter.first.c_str(), filter.second.c_str());
        }

        SDL_ShowOpenFileDialog(reinterpret_cast<SDL_DialogFileCallback>(callback), nullptr, _impl->window, dialog_file_filters.data(),
                               dialog_file_filters.size(), path.c_str(), false);
    }

    Window::Window(uint32_t width, uint32_t height)
    {
        DebugScope scope(__PRETTY_FUNCTION__);
        _impl = new Impl();
        _impl->init(width, height);
        //hide_mouse();
    }

    Window::~Window()
    {
        SDL_DestroyWindow(_impl->window);
        SDL_Quit();
        delete _impl;
    }
}
