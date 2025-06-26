#pragma once

#include "EthercatBus.hh"

template <typename T>
class EthercatReceiver
{
    EthercatBus& bus;

    static_assert(sizeof(T) <= 32, "big sugi");

public:
    EthercatReceiver(EthercatBus& bus)
        : bus{ bus }
    {}

    T GetData() const
    {
        T data;
        memcpy(&data, bus.GetBufferOutRef(), sizeof data);
        return data;
    }
};