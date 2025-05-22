#pragma once

#include <optional>
#include <cstring>

#include "Serializer.hh"

extern "C" {
#include "ethercat.h"
}

/**
 * @brief スレーブのインデックスを表現する型
 * @note デイジーチェーン接続の場合、マスターから近い順に 1, 2, 3...
 */
using SlaveIndex = int;

/**
 * @brief スレーブを表現し、スレーブからデータを取得するクラス
 * @tparam T 受信するデータの型
 */
template <typename T>
class EthercatSlaveReceiver
{
    ec_slavet& Slave;

    SlaveIndex Index;

public:
    EthercatSlaveReceiver(SlaveIndex Index)
        : Slave(ec_slave[Index])
        , Index(Index)
    {
    }
    
    std::optional<T> GetData() const
    {
        // スレーブがバスに参加しているかチェック
        // todo: スレーブ数を基にチェックしているため、参加しているかのチェックになっていない
        if (Index > ec_slavecount)
        {
            return std::nullopt;
        }

        // バイト数チェック
        if (sizeof(T) > Slave.Ibytes)
        {
            return std::nullopt;
        }

        // データを取得
        return Deserialize<T>(Slave.inputs, sizeof(T));
    }

};

/**
 * @brief スレーブを表現し、スレーブにデータを送信するクラス
 * @tparam T 送信するデータの型
 */
template <typename T>
class EthercatSlaveSender
{
    ec_slavet& Slave;

    SlaveIndex Index;

public:
    EthercatSlaveSender(SlaveIndex Index)
        : Slave(ec_slave[Index])
        , Index(Index)
    {
    }

    void SetData(T data)
    {
        // スレーブがバスに参加しているかチェック
        if (Index > ec_slavecount)
        {
            return;
        }

        // バイト数チェック
        if (sizeof(T) > Slave.Obytes)
        {
            return;
        }

        // データをセット
        const auto Buffer = Serialize<T>(data);
        std::copy(Buffer.begin(), Buffer.end(), Slave.outputs);
    }

};
