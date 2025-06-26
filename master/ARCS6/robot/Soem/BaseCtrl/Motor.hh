#pragma once

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
};


class Motor
{
    struct MasterToSlave
    {
        float CurrentRef;
        bool ServoOn : 1;
        bool ResetError : 1;
    } __attribute__((packed));

    struct SlaveToMaster
    {
        int32_t Position;
        int32_t Velocity;
        int32_t Current;

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
        mtos.ResetError = true;
    }

    void ServoOn() noexcept
    {
        mtos.ServoOn = true;
    }

    void Move() noexcept
    {
        Sender.SetData(mtos);

        // エラー復帰していたら解除
        // if (mtos.ResetError)
        // {
        //     if (Receiver.GetData()->State != SlaveToMaster::StateKind::Error)
        //     {
        //         mtos.ResetError = false; // Reset only if the state is Error
        //         std::cout << "Error reset successfully." << std::endl;
        //     }
        // }
    }

    int32_t GetVelocity() const noexcept
    {
        return Receiver.GetData()->Velocity;
    }

    int GetState() const noexcept
    {
        return static_cast<int>(Receiver.GetData()->State);
    }
};
    