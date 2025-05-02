#pragma once


/**
 * @brief EtherCATバス
 */
class EthercatBus
{

    char IOmap[4096];    ///< IOデータバッファ

    int ExpectedWKC;    ///< Working Counter Error

public:

    EthercatBus() = default;
    
    ~EthercatBus();

    // 二重クローズを防ぐためコピー禁止
    EthercatBus(const EthercatBus&) = delete;

    EthercatBus& operator=(const EthercatBus&) = delete;

    enum class InitState
    {
        ALL_SLAVES_OP_STATE,    ///< 全てのスレーブがOP状態に (成功)
        PORT_OPEN_FAILED,       ///< ポートが見つからない
        SLAVES_NOT_FOUND,       ///< スレーブが見つからない
        NOT_ALL_OP_STATE        ///< OP状態にならないスレーブがある
    };

    InitState Init(const char* InterfaceName);

    void Close();

    bool Update();
};