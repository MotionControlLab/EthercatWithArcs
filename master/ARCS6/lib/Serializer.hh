#pragma once

#include <vector>
#include <optional>
#include <cstdint>
#include <cstring>

template <typename T>
std::vector<uint8_t> Serialize(const T& Object)
{
    std::vector<uint8_t> Buffer(sizeof(T));
    std::memcpy(Buffer.data(), &Object, sizeof(T));
    return Buffer;
}

template <typename T>
std::optional<T> Deserialize(uint8_t* Data, size_t Len)
{
    if (Len != sizeof(T))
    {
        return std::nullopt;
    }

    T Object;
    std::memcpy(&Object, Data, sizeof(T));
    return Object;
}
