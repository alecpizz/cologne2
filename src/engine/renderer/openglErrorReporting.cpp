#include "openglErrorReporting.h"
#include <iostream>
//https://learnopengl.com/In-Practice/Debugging
void GLAPIENTRY glDebugOutput(GLenum source,
                              GLenum type,
                              unsigned int id,
                              GLenum severity,
                              GLsizei length,
                              const char *message,
                              const void *userParam)
{
    // ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204
        || id == 131222
    )
        return;
    if (type == GL_DEBUG_TYPE_PERFORMANCE) return;
    if (source == GL_DEBUG_SOURCE_THIRD_PARTY) return;
    std::string msg_string = std::string(message);
    std::string message_source;
    std::string message_type;
    std::string message_severity;
    switch (source)
    {
        case GL_DEBUG_SOURCE_API: message_source = "Source: API";
            break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM: message_source = "Source: Window System";
            break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: message_source = "Source: Shader Compiler";
            break;
        case GL_DEBUG_SOURCE_THIRD_PARTY: message_source = "Source: Third Party";
            break;
        case GL_DEBUG_SOURCE_APPLICATION: message_source = "Source: Application";
            break;
        case GL_DEBUG_SOURCE_OTHER: message_source = "Source: Other";
            break;
    }

    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR: message_type = "Type: Error";
            break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: message_type = "Type: Deprecated Behaviour";
            break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: message_type = "Type: Undefined Behaviour";
            break;
        case GL_DEBUG_TYPE_PORTABILITY: message_type = "Type: Portability";
            break;
        case GL_DEBUG_TYPE_PERFORMANCE: message_type = "Type: Performance";
            break;
        case GL_DEBUG_TYPE_MARKER: message_type = "Type: Marker";
            break;
        case GL_DEBUG_TYPE_PUSH_GROUP: message_type = "Type: Push Group";
            break;
        case GL_DEBUG_TYPE_POP_GROUP: message_type = "Type: Pop Group";
            break;
        case GL_DEBUG_TYPE_OTHER: message_type = "Type: Other";
            break;
    }
    std::cout << std::endl;

    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH: message_severity = "Severity: high";
            break;
        case GL_DEBUG_SEVERITY_MEDIUM: message_severity = "Severity: medium";
            break;
        case GL_DEBUG_SEVERITY_LOW: message_severity = "Severity: low";
            break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: message_severity = "Severity: notification";
            break;
    }
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:
        {
            LOG_ERROR("---------------");
            LOG_ERROR("Debug Message (%d)", id);
            LOG_ERROR("%s", message);
            LOG_ERROR("%s", message_source.c_str());
            LOG_ERROR("%s", message_type.c_str());
            LOG_ERROR("%s", message_severity.c_str());
            break;
        }
        case GL_DEBUG_SEVERITY_MEDIUM:
        {
            LOG_WARN("---------------");
            LOG_WARN("Debug Message (%d)", id);
            LOG_WARN("%s", message);
            LOG_WARN("%s", message_source.c_str());
            LOG_WARN("%s", message_type.c_str());
            LOG_WARN("%s", message_severity.c_str());
            break;
        }

        case GL_DEBUG_SEVERITY_LOW:
        {
            LOG_INFO("---------------");
            LOG_INFO("Debug Message (%d)", id);
            LOG_INFO("%s", message);
            LOG_INFO("%s", message_source.c_str());
            LOG_INFO("%s", message_type.c_str());
            LOG_INFO("%s", message_severity.c_str());
            break;
        }
        case GL_DEBUG_SEVERITY_NOTIFICATION:
        {
            LOG_DEBUG("---------------");
            LOG_DEBUG("Debug Message (%d)", id);
            LOG_DEBUG("%s", message);
            LOG_DEBUG("%s", message_source.c_str());
            LOG_DEBUG("%s", message_type.c_str());
            LOG_DEBUG("%s", message_severity.c_str());
            break;
        }
    }
}

void enableReportGlErrors()
{
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(glDebugOutput, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
}
