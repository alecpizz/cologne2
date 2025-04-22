//
// Created by alecpizz on 3/1/2025.
//

#include "Window.h"

#include "DebugUI.h"
#include "openglErrorReporting.h"

namespace cologne
{

    struct Window::Impl
    {
        SDL_Window *window = nullptr;
        SDL_GLContext context = nullptr;
        uint32_t width = 0;
        uint32_t height = 0;



        void init(uint32_t w, uint32_t h)
        {
            width = w;
            height = h;
            if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
            {
                LOG_ERROR("SDL_Init Error: %s", SDL_GetError());
            }

            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
            SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
            SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);
            window = SDL_CreateWindow("cologne 2", w, h, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
            if (window == nullptr)
            {
                SDL_Quit();
            }
            SDL_SetWindowBordered(window, false);
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


            enableReportGlErrors();
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
    }

    void Window::show_mouse() const
    {
        SDL_SetWindowRelativeMouseMode(_impl->window, false);
    }

    Window::Window(uint32_t width, uint32_t height)
    {
        _impl = new Impl();
        _impl->init(width, height);
        hide_mouse();
    }

    Window::~Window()
    {
        SDL_DestroyWindow(_impl->window);
        SDL_Quit();
        delete _impl;
    }
}
