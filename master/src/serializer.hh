#pragma once

#include <span>
#include <vector>
#include <optional>

template <typename T>
std::vector<uint8_t> serialize(const T& object);

template <typename T>
std::optional<T> deserialize(std::span<uint8_t> data);
