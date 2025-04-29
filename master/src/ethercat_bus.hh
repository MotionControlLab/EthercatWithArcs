#pragma once


/**
 * @brief EtherCATバス
 */
class ethercat_bus
{

    char IOmap[4096];    ///< IOデータバッファ

    int expectedWKC;    ///< Working Counter Error

public:

    ethercat_bus() = default;
    
    ~ethercat_bus();

    // 二重クローズを防ぐためコピー禁止
    ethercat_bus(const ethercat_bus&) = delete;

    ethercat_bus& operator=(const ethercat_bus&) = delete;

    enum class init_state
    {
        ALL_SLAVES_OP_STATE,    ///< 全てのスレーブがOP状態に (成功)
        PORT_OPEN_FAILED,       ///< ポートが見つからない
        SLAVES_NOT_FOUND,       ///< スレーブが見つからない
        NOT_ALL_OP_STATE        ///< OP状態にならないスレーブがある
    };

    init_state init(const char* interface_name);

    void close();

    void update();
};
