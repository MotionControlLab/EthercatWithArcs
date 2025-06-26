#pragma once

#include "EthercatBus.hh"

template <typename T>
class EthercatSender
{
    EthercatBus& bus;

    static_assert(sizeof(T) <= 32, "big sugi");

public:
    EthercatSender(EthercatBus& bus)
        : bus{ bus }
    {}

    void SetData(const T& data)
    {
        memcpy(bus.GetBufferInRef(), &data, sizeof data);
    }
};