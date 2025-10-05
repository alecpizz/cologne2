//
// Created by alecpizz on 10/5/25.
//
#pragma once
namespace cologne
    {
    struct MeshComponent
    {
        MeshComponent() = default;

        MeshComponent(const std::string &name);

        MeshComponent(int idx);

        std::string mesh_name;
    };
}