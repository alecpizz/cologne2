//
// Created by alecpizz on 7/21/25.
//

#include "UUID.h"

#include <random>

namespace cologne
{
    static std::random_device random_device;
    static std::mt19937_64 random_engine(random_device());
    static std::uniform_int_distribution<uint64_t> uniform_int_distribution;

    UUID::UUID() : _uuid(uniform_int_distribution(random_engine))
    {

    }

    UUID::UUID(uint64_t id) : _uuid(id)
    {
    }

}
