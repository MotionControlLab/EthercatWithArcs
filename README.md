# EthercatWithArcs

## setup

```sh
git clone https://github.com/MotionControlLab/EthercatWithArcs.git --recursive
cd ./EthercatWithArcs/master
```

## build & run

```sh
sudo ./run.sh
```

## EtherCAT 経由でのACモーター制御

## 構造体

マスター → スレーブ

```cpp
struct MasterToSlaveSchema
{
    float IqCurrentRef;  // Iq軸電流指令値 [A]

    enum class ControlKind : uint8_t
    {
        None,
        ServoOn,
        ServoOff,
        ResetError,
    } Control : 2;  // 状態遷移指令
} __attribute__((packed));
```

マスター ← スレーブ

```cpp
struct SlaveToMasterSchema
{
    int32_t ThetaECount;  // 角位置 [ECount]   → 機械角にできそう
    int32_t OmegaECount;  // 角速度 [ECount/s] → 機械角にできそう
    float   IqCurrent;    // Iq軸電流 [A]

    enum class StateKind : uint8_t
    {
        None,
        Run,
        Stop,
        Error,
    } State : 2;  // モータードライバの状態
} __attribute__((packed));
```

### 疑似コード / 電流制御 / 1つのモーター

```cpp
static EthercatBus Bus;

static AcMotor AcMotor{
    SlaveIndex{ 1 },
};

int main()
{
    Bus.Init("enp1s0");  // 通信開始

    for ( ;; )
    {
        AcMotor.SetCurrentRef(CurrentRef);  // 電流指令値設定
        AcMotor.Update();
        
        Bus.Update();

        wait(500us);
    }
}
```


### 疑似コード / 電流制御 / 2つのモーター

```cpp
static EthercatBus Bus;

static AcMotor AcMotor1{
    SlaveIndex{ 1 },
};

static AcMotor AcMotor2{
    SlaveIndex{ 2 },
};

int main()
{
    Bus.Init("enp1s0");  // 通信開始

    for ( ;; )
    {
        AcMotor1.SetCurrentRef(0.01);  // 電流指令値設定
        AcMotor1.Update();

        AcMotor2.SetCurrentRef(0.01);  // 電流指令値設定
        AcMotor2.Update();
        
        Bus.Update();

        wait(500us);
    }
}
```

### 疑似コード / 速度制御 / 1つのモーター

```cpp
static EthercatBus Bus;

static AcMotor AcMotor{
    SlaveIndex{ 1 },
};

static PIController SpeedController{ Kp, Ki, 0.000'500 };

int main()
{
    Bus.Init("enp1s0");  // 通信開始

    for ( ;; )
    {
        auto TargetSpeed = 2 * PI;  // 目標速度 [rad/s]
        auto CurrentSpeed = AcMotor.GetOmega();  // 現在速度 [rad/s]
        auto CurrentRef = SpeedController.Update(CurrentSpeed, TargetSpeed);  // 電流指令値を計算
        AcMotor.SetCurrentRef(CurrentRef);
        AcMotor.Update();
        
        Bus.Update();

        wait(500us);
    }
}
```

### 疑似コード / 状態遷移

```cpp
static EthercatBus Bus;

static AcMotor AcMotor{
    SlaveIndex{ 1 },
};

int main()
{
    Bus.Init("enp1s0");  // 通信開始

    for ( ;; )
    {
        auto Input = GetInput();
        if (Input == "on")
            AcMotor.ServoOnAsync();
        else if (Input == "off")
            AcMotor.ServoOffAsync();
        else if (Input == "reset")
            AcMotor.ErrorResetAsync();

        AcMotor.SetCurrentRef(0.01);
        AcMotor.Update();
        
        Bus.Update();

        wait(500us);
    }
}
```

## AcMotorクラス 全API

```cpp
// 送信系
AcMotor.SetCurrentRef(CurrentRef);  // 電流指令値設定
AcMotor.ServoOnAsync();             // サーボオン
AcMotor.ServoOffAsync();            // サーボオフ
AcMotor.ErrorResetAsync();          // エラーリセット

// 受信系
AcMotor.GetIqCurrent();     // Iq軸電流取得 (float [A])
AcMotor.GetOmega();         // 角速度取得   (float [rad/s])
AcMotor.GetTheta();         // 角位置取得   (float [rad])
AcMotor.GetState();         // 状態取得     (StateKind [None, Run, Stop, Error])

// 更新
AcMotor.Update();
```
