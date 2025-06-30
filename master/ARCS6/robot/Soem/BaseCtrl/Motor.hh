#pragma once

#include "ARCSprint.hh"

class PIController
{
    double Kp;
    double Ki;
    const double Ts;
    
    double Integral;
    double LastError;
public:
    PIController(double Kp, double Ki, double Ts) noexcept
        : Kp(Kp)
        , Ki(Ki)
        , Ts(Ts)
        , Integral()
        , LastError()
    {}

    double Update(double Current, double Target) noexcept
    {
        double Error = Target - Current;
        Integral += Error * Ts;
        double Output = Kp * Error + Ki * Integral;

        LastError = Error;
        return Output;
    }

    void Reset() noexcept
    {
        Integral = 0.0;
        LastError = 0.0;
    }
};


class Motor
{
    struct MasterToSlave
    {
        float CurrentRef;
        bool ServoOn : 1;
        bool ServoOff : 1;
        bool ResetError : 1;
    } __attribute__((packed));

    struct SlaveToMaster
    {
        int32_t Position;
        int32_t Velocity;
        float Current;

        enum class StateKind
        {
            None,
            Run,
            Stop,
            Error,
        } State : 2;
    } __attribute__((packed));

    PIController fbctrl;

    EthercatSender<MasterToSlave> Sender;

    EthercatReceiver<SlaveToMaster> Receiver;

    MasterToSlave mtos{};

public:
    Motor(SlaveIndex Index, PIController&& fbctrl) noexcept
        : fbctrl(std::move(fbctrl))
        , Sender(Index)
        , Receiver(Index)
    {
    }

    void SetTargetVelocity(float TargetVelocity) noexcept
    {
        mtos.CurrentRef = fbctrl.Update(Receiver.GetData()->Velocity, TargetVelocity);
    }

    void ResetError() noexcept
    {
        fbctrl.Reset();
        mtos.ResetError = true;
    }

    void ServoOn() noexcept
    {
        mtos.ServoOn = true;
    }

    void ServoOff() noexcept
    {
        mtos.ServoOff = true;
    }

    void Update() noexcept
    {
        Sender.SetData(mtos);

        // エラー復帰していたら解除
        if (mtos.ResetError && Receiver.GetData()->State != SlaveToMaster::StateKind::Error)
        {
            mtos.ResetError = false; // Reset only if the state is Error
            std::cout << "Error reset successfully." << std::endl;
        }

        // サーボONしていたら解除
        if (mtos.ServoOn && Receiver.GetData()->State == SlaveToMaster::StateKind::Run)
        {
            mtos.ServoOn = false; // Reset only if the state is not Run
            std::cout << "Servo ON successfully." << std::endl;
        }

        // サーボOFFしていたら解除
        if (mtos.ServoOff && Receiver.GetData()->State == SlaveToMaster::StateKind::Stop)
        {
            mtos.ServoOff = false; // Reset only if the state is not Stop
            std::cout << "Servo OFF successfully." << std::endl;
        }
    }

    void Dump() const
    {
        // DebugPrintVarFmt("Motor State", "Position: %d, Velocity: %d, Current: %d, State: %d",
        //     Receiver.GetData()->Position,
        //     Receiver.GetData()->Velocity,
        //     Receiver.GetData()->Current,
        //     static_cast<int>(Receiver.GetData()->State));
        using namespace ARCS;
        DebugPrint("hogehoge");
    }

    int32_t GetVelocity() const noexcept
    {
        return Receiver.GetData()->Velocity;
    }

    int32_t GetPosition() const noexcept
    {
        return Receiver.GetData()->Position;
    }

    int GetState() const noexcept
    {
        return static_cast<int>(Receiver.GetData()->State);
    }
};
    