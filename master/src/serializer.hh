#pragma once

#include <vector>
#include <optional>
#include <cstdint>
#include <cstring>

template <typename T>
std::vector<uint8_t> serialize(const T& object)
{
    std::vector<uint8_t> buffer(sizeof(T));
    std::memcpy(buffer.data(), &object, sizeof(T));
    return buffer;
}

template <typename T>
std::optional<T> deserialize(uint8_t* data, size_t len)
{
    if (len != sizeof(T))
    {
        return std::nullopt;
    }

    T object;
    std::memcpy(&object, data, sizeof(T));
    return object;
}
