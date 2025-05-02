#pragma once

class ethercat_bus
{

    char IOmap[4096];    // IOデータバッファ

    int expectedWKC;    // 期待されるWKC値

public:

    ethercat_bus() = default;

    ~ethercat_bus();

    enum class init_state
    {
        ALL_SLAVES_OP_STATE,    // 全てのスレーブがOP状態に (成功)
        PORT_OPEN_FAILED,       // ポートが見つからない
        SLAVES_FOUND,           // スレーブがみつかりません
        NOT_ALL_OP_STATE        // OP状態にならないスレーブがある
    };

    init_state init(const char* interface_name);

    void close();

    void update();

};

    