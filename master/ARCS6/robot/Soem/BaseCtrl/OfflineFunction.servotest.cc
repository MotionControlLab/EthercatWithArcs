#include <cassert>
#include <chrono>
#include <thread>
#include <iostream>
#include <mutex>

#include "EthercatBus.hh"
#include "PIController.hh"
#include "AcMotor.hh"




// ループ周期一定にするやつ
using namespace std::chrono_literals;
void LoopCycleControl(std::chrono::microseconds loopDuration)
{
    static auto PrevLoopTime = std::chrono::steady_clock::now();

    auto Now = std::chrono::steady_clock::now();
    auto ElapsedUs = std::chrono::duration_cast<std::chrono::microseconds>(Now - PrevLoopTime);
    PrevLoopTime = Now;

    if (ElapsedUs < loopDuration)
    {
        std::this_thread::sleep_for(loopDuration - ElapsedUs);
    }
}


// スレッド間通信用
std::mutex mtx;
bool ServoOn = false;
bool ServoOff = false;
bool ResetError = false;
bool exitCtrlThread = false;


void ControlFunction()
{
    static EthercatBus Bus;

    static EthercatReceiver<int> Volume{ SlaveIndex{ 2 } };
    static EthercatSender<int> Servo{ SlaveIndex{ 1 } };

    // EtherCATバスの初期化
    assert(Bus.Init("enp1s0") == EthercatBus::InitState::ALL_SLAVES_OP_STATE);

    // メインループ
    for (;;)
    {
        Bus.Update();

        // 速度フィードバック
        if (const auto TargetSpeedRaw = Volume.GetData())
        {
            const float TargetSpeed = *TargetSpeedRaw / 1024. * 180;
            Servo.SetData(TargetSpeed);
        }
        else
        {
            std::cout << "[x] Failed to get target velocity from Volume." << std::endl;
        }

        // PCから状態遷移させる
        {
            std::lock_guard<std::mutex> lock(mtx);
            if (ServoOn)
            {
                ServoOn = false;
            }
            if (ServoOff)
            {
                ServoOff = false;
            }
            if (ResetError)
            {
                ResetError = false;
            }
            if (exitCtrlThread)
            {
                return;
            }
        }

        LoopCycleControl(500us);
    }
}


int main()
{

    std::thread ctrlThread(ControlFunction);

    for (;;)
    {
        char command;
        std::cout << "Enter command (o: ServoOn, f: ServoOff, r: ResetError, q: Quit): ";
        std::cin >> command;

        {
            std::lock_guard<std::mutex> lock(mtx);
            if (command == 'o')
                ServoOn = true;
            else if (command == 'f')
                ServoOff = true;
            else if (command == 'r')
                ResetError = true;
            else if (command == 'q')
            {
                exitCtrlThread = true;
                break;
            }
            else
                std::cout << "Unknown command." << std::endl;
        }
    }

    ctrlThread.join();
}
