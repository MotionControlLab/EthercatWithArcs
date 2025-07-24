#pragma once

#include <EasyCAT.h>

class EthercatBus
{
    EasyCAT EasyCat;

public:
    EthercatBus() = default;
    
    EthercatBus(EasyCAT&& EasyCat)
        : EasyCat{ std::move(EasyCat) }
    {}

    bool Init()
    {
        return EasyCat.Init();
    }

    void Update()
    {
        EasyCat.MainTask();
    }

    auto& GetBufferInRef()
    {
        return EasyCat.BufferIn.Byte;
    }

    auto& GetBufferOutRef()
    {
        return EasyCat.BufferOut.Byte;
    }
};
