#pragma once

#include <optional>
#include <cstring>

#include "serializer.hh"

extern "C" {
#include "ethercat.h"
}

/**
 * @brief スレーブのインデックスを表現する型
 * @note デイジーチェーン接続の場合、マスターから近い順に 1, 2, 3...
 */
using slave_index = int;

/**
 * @brief スレーブを表現し、スレーブからデータを取得するクラス
 * @tparam T 受信するデータの型
 */
template <typename T>
class ethercat_slave_receiver
{
    ec_slavet& soem_svale;

    slave_index index;

public:
    ethercat_slave_receiver(slave_index index)
        : soem_svale(ec_slave[index])
        , index(index)
    {
    }
    
    std::optional<T> get_data() const
    {
        // スレーブがバスに参加しているかチェック
        // todo: スレーブ数を基にチェックしているため、参加しているかのチェックになっていない
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
        return deserialize<T>({ soem_svale.inputs, sizeof(T) });
    }

};

/**
 * @brief スレーブを表現し、スレーブにデータを送信するクラス
 * @tparam T 送信するデータの型
 */
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
        const auto buffer = serialize<T>(data);
        std::copy(buffer.begin(), buffer.end(), soem_svale.outputs);
    }

};
