#include "Input.h"
#include <SDL3/SDL.h>
#include <unordered_map>

namespace goon::Input
{
    glm::vec2 _mouse_motion = {0.0f, 0.0f};
    std::unordered_map<Key, bool> _key_down_map;
    std::unordered_map<Key, bool> _key_pressed_map;
    std::unordered_map<Key, bool> _key_pressed_last_frame_map;

    Key translate_key(uint32_t scan_code)
    {
        switch (scan_code)
        {
            case SDL_SCANCODE_A: return Key::A;
            case SDL_SCANCODE_B: return Key::B;
            case SDL_SCANCODE_C: return Key::C;
            case SDL_SCANCODE_D: return Key::D;
            case SDL_SCANCODE_E: return Key::E;
            case SDL_SCANCODE_F: return Key::F;
            case SDL_SCANCODE_G: return Key::G;
            case SDL_SCANCODE_H: return Key::H;
            case SDL_SCANCODE_I: return Key::I;
            case SDL_SCANCODE_J: return Key::J;
            case SDL_SCANCODE_K: return Key::K;
            case SDL_SCANCODE_L: return Key::L;
            case SDL_SCANCODE_M: return Key::M;
            case SDL_SCANCODE_N: return Key::N;
            case SDL_SCANCODE_O: return Key::O;
            case SDL_SCANCODE_P: return Key::P;
            case SDL_SCANCODE_Q: return Key::Q;
            case SDL_SCANCODE_R: return Key::R;
            case SDL_SCANCODE_S: return Key::S;
            case SDL_SCANCODE_T: return Key::T;
            case SDL_SCANCODE_U: return Key::U;
            case SDL_SCANCODE_V: return Key::V;
            case SDL_SCANCODE_W: return Key::W;
            case SDL_SCANCODE_X: return Key::X;
            case SDL_SCANCODE_Y: return Key::Y;
            case SDL_SCANCODE_Z: return Key::Z;
            case SDL_SCANCODE_SLASH: return Key::ForwardSlash;
            case SDL_SCANCODE_BACKSLASH: return Key::BackSlash;
            case SDL_SCANCODE_NUMLOCKCLEAR: return Key::NumLock;
            case SDL_SCANCODE_KP_DIVIDE: return Key::KeypadDivide;
            case SDL_SCANCODE_KP_MULTIPLY: return Key::KeypadMultiply;
            case SDL_SCANCODE_KP_MINUS: return Key::KeypadMinus;
            case SDL_SCANCODE_KP_PLUS: return Key::KeypadPlus;
            case SDL_SCANCODE_KP_ENTER: return Key::KeypadEnter;
            case SDL_SCANCODE_KP_1: return Key::Keypad1;
            case SDL_SCANCODE_KP_2: return Key::Keypad2;
            case SDL_SCANCODE_KP_3: return Key::Keypad3;
            case SDL_SCANCODE_KP_4: return Key::Keypad4;
            case SDL_SCANCODE_KP_5: return Key::Keypad5;
            case SDL_SCANCODE_KP_6: return Key::Keypad6;
            case SDL_SCANCODE_KP_7: return Key::Keypad7;
            case SDL_SCANCODE_KP_8: return Key::Keypad8;
            case SDL_SCANCODE_KP_9: return Key::Keypad9;
            case SDL_SCANCODE_KP_0: return Key::Keypad0;
            case SDL_SCANCODE_KP_PERIOD: return Key::KeypadPeriod;
            case SDL_SCANCODE_LCTRL: return Key::LeftCtrl;
            case SDL_SCANCODE_LSHIFT: return Key::LeftShift;
            case SDL_SCANCODE_LALT: return Key::LeftAlt;
            case SDL_SCANCODE_RCTRL: return Key::RightCtrl;
            case SDL_SCANCODE_RSHIFT: return Key::RightShift;
            case SDL_SCANCODE_RALT: return Key::RightAlt;
            case SDL_SCANCODE_GRAVE: return Key::Grave;
            case SDL_SCANCODE_SPACE: return Key::Space;
            case SDL_SCANCODE_CAPSLOCK: return Key::CapsLock;
            case SDL_SCANCODE_F1: return Key::F1;
            case SDL_SCANCODE_F2: return Key::F2;
            case SDL_SCANCODE_F3: return Key::F3;
            case SDL_SCANCODE_F4: return Key::F4;
            case SDL_SCANCODE_F5: return Key::F5;
            case SDL_SCANCODE_F6: return Key::F6;
            case SDL_SCANCODE_F7: return Key::F7;
            case SDL_SCANCODE_F8: return Key::F8;
            case SDL_SCANCODE_F9: return Key::F9;
            case SDL_SCANCODE_F10: return Key::F10;
            case SDL_SCANCODE_F11: return Key::F11;
            case SDL_SCANCODE_F12: return Key::F12;
            case SDL_SCANCODE_PRINTSCREEN: return Key::PrintScreen;
            case SDL_SCANCODE_SCROLLLOCK: return Key::ScrollLock;
            case SDL_SCANCODE_PAUSE: return Key::Pause;
            case SDL_SCANCODE_INSERT: return Key::Insert;
            case SDL_SCANCODE_DELETE: return Key::Delete;
            case SDL_SCANCODE_HOME: return Key::Home;
            case SDL_SCANCODE_PAGEUP: return Key::PageUp;
            case SDL_SCANCODE_PAGEDOWN: return Key::PageDown;
            case SDL_SCANCODE_END: return Key::End;
            case SDL_SCANCODE_MODE: return Key::AltGr;
            case SDL_SCANCODE_0: return Key::Zero;
            case SDL_SCANCODE_1: return Key::One;
            case SDL_SCANCODE_2: return Key::Two;
            case SDL_SCANCODE_3: return Key::Three;
            case SDL_SCANCODE_4: return Key::Four;
            case SDL_SCANCODE_5: return Key::Five;
            case SDL_SCANCODE_6: return Key::Six;
            case SDL_SCANCODE_7: return Key::Seven;
            case SDL_SCANCODE_8: return Key::Eight;
            case SDL_SCANCODE_9: return Key::Nine;
            case SDL_SCANCODE_RETURN: return Key::Return;
            case SDL_SCANCODE_ESCAPE: return Key::Escape;
            case SDL_SCANCODE_BACKSPACE: return Key::Backspace;
            case SDL_SCANCODE_TAB: return Key::Tab;
            case SDL_SCANCODE_MINUS: return Key::Minus;
            case SDL_SCANCODE_EQUALS: return Key::Equals;
            case SDL_SCANCODE_LEFTBRACKET: return Key::LeftBracket;
            case SDL_SCANCODE_RIGHTBRACKET: return Key::RightBracket;
            case SDL_SCANCODE_SEMICOLON: return Key::Semicolon;
            case SDL_SCANCODE_APOSTROPHE: return Key::Apostrophe;
            case SDL_SCANCODE_COMMA: return Key::Comma;
            case SDL_SCANCODE_PERIOD: return Key::Period;
            case SDL_SCANCODE_RIGHT: return Key::Right;
            case SDL_SCANCODE_LEFT: return Key::Left;
            case SDL_SCANCODE_DOWN: return Key::Down;
            case SDL_SCANCODE_UP: return Key::Up;
            case SDL_SCANCODE_LGUI: return Key::LeftGui;
            case SDL_SCANCODE_RGUI: return Key::RightGui;
            default: return Key::UnknownKey;
        }
    }

    void update()
    {
        _mouse_motion.x = 0.0f;
        _mouse_motion.y = 0.0f;
        for (auto key_down_map: _key_pressed_map)
        {
            _key_pressed_map[key_down_map.first] = false;
        }
    }

    void update_mouse(float x, float y)
    {
        _mouse_motion.x = x;
        _mouse_motion.y = y;
    }

    void update_key_up(uint32_t scan_code)
    {
        Key key = translate_key(scan_code);
        _key_down_map[key] = false;

        if (_key_pressed_last_frame_map[key])
        {
            _key_pressed_last_frame_map[key] = false;
        }
        // if (!_key_pressed_last_frame_map[key])
        //     _key_pressed_map[key] = false;
        // else
        //     _key_pressed_map[key] = true;
        // _key_pressed_last_frame_map[key] = _key_down_map[key];
        // _key_pressed_last_frame_map[key] = _key_down_map[key];
    }

    void update_key_down(uint32_t scan_code)
    {
        Key key = translate_key(scan_code);
        _key_down_map[key] = true;
        if (!_key_pressed_last_frame_map[key])
        {
            _key_pressed_map[key] = true;
            _key_pressed_last_frame_map[key] = true;
        }
    }

    glm::vec2 get_relative_mouse()
    {
        return _mouse_motion;
    }

    bool key_pressed(Key key)
    {
        return _key_pressed_map[key];
    }

    bool key_down(Key key)
    {
        return _key_down_map.contains(key) && _key_down_map[key];
    }
}
