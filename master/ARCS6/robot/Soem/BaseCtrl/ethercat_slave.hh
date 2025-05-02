#pragma once

#include <optional>
#include <cstring>

extern "C" {
#include "ethercat.h"
}


using slave_index = int;

template <typename T>
class ethercat_slave_reader
{
    ec_slavet& soem_svale;

    slave_index index;

public:
    ethercat_slave_reader(slave_index index)
        : soem_svale(ec_slave[index])
        , index(index)
    {
    }
    
    std::optional<T> get_data()
    {
        // スレーブがバスに参加しているかチェック
        if (index > ec_slavecount)
        {
            return std::nullopt;
        }

        // バイト数チェック
        if (sizeof(T) > soem_svale.Ibytes)
        {
            return std::nullopt;
        }

        // データを取得
        T data;
        std::memcpy(&data, soem_svale.inputs, sizeof(T));
        return data;
    }

};

template <typename T>
class ethercat_slave_sender
{
    ec_slavet& soem_svale;

    slave_index index;

public:
    ethercat_slave_sender(slave_index index)
        : soem_svale(ec_slave[index])
        , index(index)
    {
    }

    void set_data(T data)
    {
        // スレーブがバスに参加しているかチェック
        if (index > ec_slavecount)
        {
            return;
        }

        // バイト数チェック
        if (sizeof(T) > soem_svale.Obytes)
        {
            return;
        }

        // データをセット
        std::memcpy(soem_svale.outputs, &data, sizeof(T));
    }

};
