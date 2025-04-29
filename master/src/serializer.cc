//
//    シリアライザ
//

//    一旦memcpyで

#include "serializer.hh"

template <typename T>
std::vector<uint8_t> serialize(const T& object)
{
    std::vector<uint8_t> buffer(sizeof(T));
    std::memcpy(buffer.data(), &object, sizeof(T));
    return buffer;
}

template <typename T>
std::optional<T> deserialize(std::span<uint8_t> data)
{
    if (data.size() != sizeof(T))
    {
        return std::nullopt;
    }

    T object;
    std::memcpy(&object, data.data(), sizeof(T));
    return object;
}
