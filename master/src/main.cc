#include <iostream>
#include <chrono>
#include <thread>

#include "ethercat_bus.hh"
#include "ethercat_slave.hh"

int main()
{
    ethercat_bus bus;
    
    switch (bus.init("eth0"))
    {
    case ethercat_bus::init_state::ALL_SLAVES_OP_STATE:
        std::cout << "[o] All slaves are in OP state." << std::endl;
        break;
    case ethercat_bus::init_state::PORT_OPEN_FAILED:
        std::cout << "[x] Port open failed." << std::endl;
        return 1;
    case ethercat_bus::init_state::SLAVES_NOT_FOUND:
        std::cout << "[x] Slaves found but not all are in OP state." << std::endl;
        return 2;
    case ethercat_bus::init_state::NOT_ALL_OP_STATE:
        std::cout << "[x] Not all slaves are in OP state." << std::endl;
        return 3;
    }

    ethercat_slave_receiver<int> volume_slave{ 
        slave_index{ 1 },
    };

    ethercat_slave_sender<int> servo_slave{ 
        slave_index{ 2 },
    };

    for (;;)
    {
        bus.update();

        if (const std::optional<int> volume = volume_slave.get_data())
        {
            servo_slave.set_data(*volume / 1024. * 180);
            std::cout << "[o] Volume data: " << *volume << std::endl;
        }
        else
        {
            std::cout << "[x] Failed to get volume data." << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));  // 待機
    }

}
