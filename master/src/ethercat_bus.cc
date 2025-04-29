#include "ethercat_bus.hh"

#include <stdio.h>
#include <iostream>

extern "C"
{
#include "ethercat.h"
}

ethercat_bus::~ethercat_bus()
{
    close();
}


ethercat_bus::init_state ethercat_bus::init(const char* interface_name)
{
    // init SOEM
    if (ec_init(interface_name) <= 0)
    {
        return init_state::PORT_OPEN_FAILED;
    }

    // スレーブを検索し自動コンフィグ
    if (ec_config_init(FALSE) <= 0)
    {
        return init_state::SLAVES_NOT_FOUND;
    }

    std::cout << ec_slavecount << " slaves found and configured." << std::endl;

    ec_config_map(&IOmap);
    ec_configdc();

    std::cout << "Slaves mapped, state to SAFE_OP." << std::endl;

    // 全てのスレーブが SAFE_OP 状態に達するのを待つ
    ec_statecheck(0, EC_STATE_SAFE_OP, EC_TIMEOUTSTATE * 4);

    // 何じゃこりゃ
    // int oloop, iloop, chk;
    // oloop = master.Obytes;
    // if ((oloop == 0) && (master.Obits > 0)) oloop = 1;
    // if (oloop > 8) oloop = 8;
    // iloop = master.Ibytes;
    // if ((iloop == 0) && (master.Ibits > 0)) iloop = 1;
    // if (iloop > 8) iloop = 8;

    std::cout
        << "segments : " << ec_group[0].nsegments << " : "
        << ec_group[0].IOsegment[0] << " "
        << ec_group[0].IOsegment[1] << " "
        << ec_group[0].IOsegment[2] << " "
        << ec_group[0].IOsegment[3] << std::endl;

    std::cout << "Request operational state for all slaves" << std::endl;

    expectedWKC = (ec_group[0].outputsWKC * 2) + ec_group[0].inputsWKC;

    std::cout << "Calculated workcounter " << expectedWKC << std::endl;

    // 全てのスレーブにOP状態を要求
    // ec_slave[0] はマスターを指す
    auto& master = ec_slave[0];
    master.state = EC_STATE_OPERATIONAL;
    /* send one valid process data to make outputs in slaves happy*/    // ←意味不明
    ec_send_processdata();
    ec_receive_processdata(EC_TIMEOUTRET);
    ec_writestate(0);

    // 全てのスレーブがOP状態に達するのを待つ
    int chk = 40;
    do
    {
        ec_send_processdata();
        ec_receive_processdata(EC_TIMEOUTRET);
        ec_statecheck(0, EC_STATE_OPERATIONAL, 50000);
    } while (chk-- && (master.state != EC_STATE_OPERATIONAL));

    if (master.state == EC_STATE_OPERATIONAL)
    {
        return init_state::ALL_SLAVES_OP_STATE;
    }
    else
    {
        return init_state::NOT_ALL_OP_STATE;
    }
}

void ethercat_bus::close()
{
    // マスターを切断
    ec_slave[0].state = EC_STATE_INIT;
    ec_writestate(0);

    // ソケットを閉じ終了
    ec_close();
}

bool ethercat_bus::update()
{
    ec_send_processdata();

    const int wkc = ec_receive_processdata(EC_TIMEOUTRET);

    return wkc >= expectedWKC;
}
