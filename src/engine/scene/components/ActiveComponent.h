//
// Created by alecpizz on 10/5/25.
//
#pragma once
namespace cologne
    {
    struct ActiveComponent
    {
        bool active = true;
        explicit operator bool() { return active; }
        explicit operator const bool() const { return active; }
    };
}