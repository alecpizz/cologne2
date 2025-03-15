#pragma once
#include "gpch.h"
namespace goon::Input
{
    enum class Key
    {
        UnknownKey,
        A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
        One, Two, Three, Four, Five, Six, Seven, Eight, Nine, Zero,
        Return, Escape, Backspace, Tab, Space, Minus, Equals, LeftBracket,
        RightBracket,
        BackSlash, Semicolon, Apostrophe, Grave, Comma, Period, ForwardSlash,
        CapsLock,
        F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, PrintScreen, ScrollLock,
        Pause, Insert, Home, PageUp, Delete, End, PageDown, Right, Left, Down, Up,
        NumLock,
        KeypadDivide, KeypadMultiply, KeypadMinus, KeypadPlus, KeypadEnter,
        KeypadPeriod,
        Keypad1, Keypad2, Keypad3, Keypad4, Keypad5, Keypad6, Keypad7, Keypad8,
        Keypad9, Keypad0,
        LeftShift, RightShift, LeftCtrl, RightCtrl, LeftAlt, RightAlt, LeftGui,
        RightGui, AltGr
    } ;



    void init();
    void update();
    void disable_mouse();
    void enable_mouse();
    void update_mouse(float x, float y);
    void update_key_up(uint32_t scan_code);
    void update_key_down(uint32_t scan_code);
    bool key_pressed(Key key);
    bool key_down(Key key);
    glm::vec2 get_relative_mouse();

}