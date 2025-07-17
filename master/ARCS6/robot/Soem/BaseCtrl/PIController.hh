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
    {
    }

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
