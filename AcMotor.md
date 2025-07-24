
# AcMotor クラス

ACモーターを制御するクラスです。EtherCATのマスター側で使用することを想定しています。

EtherCATを介して、電流制御、フィードバック値の取得、状態遷移を行えます。

## 使用例

ARCS を用いる場合の実装例を示します。モーター制御以外の処理は省略しています。

### 電流制御 / 1つのモーター

```cpp
#include "AcMotor.hh"

bool ControlFunctions::ControlFunction1(...)
{
    static EthercatBus Bus;
    
    static AcMotor AcMotor{
        SlaveIndex{ 1 },
    };
    
    // 初期化時
    if (CmdFlag == CTRL_INIT)
    {
        Bus.Init("enp1s0"); 　 // 通信開始 ("enp1s0" はネットワークインターフェース名)
    }

    // ループ中
    if (CmdFlag == CTRL_LOOP)
    {
        AcMotor.SetCurrentRef(CurrentRef);   // 電流指令値設定
        AcMotor.Update();                    // EtherCATの送受信バッファーに指令値などを書き込み

        Bus.Update();   // スレーブと通信
    }
}
```

### 電流制御 / 2つのモーター

```cpp
#include "AcMotor.hh"

bool ControlFunctions::ControlFunction1(...)
{
    static EthercatBus Bus;

    static AcMotor AcMotor1{
        SlaveIndex{ 1 },
    };

    static AcMotor AcMotor2{
        SlaveIndex{ 2 },
    };

    // 初期化時
    if (CmdFlag == CTRL_INIT)
    {
        Bus.Init("enp1s0");  // 通信開始
    }

    // ループ中
    if (CmdFlag == CTRL_LOOP)
    {
        AcMotor1.SetCurrentRef(0.01);  // 電流指令値設定
        AcMotor1.Update();           

        AcMotor2.SetCurrentRef(0.01);  // 電流指令値設定
        AcMotor2.Update();             // EtherCATの送受信バッファーに

        Bus.Update();
    }
}
```

### 速度制御 / 1つのモーター

```cpp
#include "AcMotor.hh"

bool ControlFunctions::ControlFunction1(...)
{
    static EthercatBus Bus;

    static AcMotor AcMotor{
        SlaveIndex{ 1 },
    };

    static PIController SpeedController{ Kp, Ki, 0.000'500 };  // PI制御器の初期化

    // 初期化時
    if (CmdFlag == CTRL_INIT)
    {
        Bus.Init("enp1s0");  // 通信開始
    }

    // ループ中
    if (CmdFlag == CTRL_LOOP)
    {
        auto TargetSpeed = 2 * PI;  // 目標速度 [rad/s]
        auto CurrentSpeed = AcMotor.GetOmega();  // 現在速度 [rad/s]
        auto CurrentRef = SpeedController.Update(CurrentSpeed, TargetSpeed);  // 電流指令値を計算
        AcMotor.SetCurrentRef(CurrentRef);
        AcMotor.Update();

        Bus.Update();
    }
}
```

### 状態遷移

```cpp
#include "AcMotor.hh"

bool ControlFunctions::ControlFunction1(...)
{
    static EthercatBus Bus;

    static AcMotor AcMotor{
        SlaveIndex{ 1 },
    };

    // 初期化時
    if (CmdFlag == CTRL_INIT)
    {
        Bus.Init("enp1s0");  // 通信開始
    }

    // ループ中
    if (CmdFlag == CTRL_LOOP)
    {
        if (/*オンにする条件*/)
        {
            AcMotor.ServoOnAsync();  // サーボオン指令を送信
        }
        if (/*オフにする条件*/)
        {
            AcMotor.ServoOffAsync();  // サーボオフ指令を送信
        }
        if (/*エラーリセットする条件*/)
        {
            AcMotor.ErrorResetAsync();  // エラーリセット指令を送信
        }

        AcMotor.Update();

        Bus.Update();
    }
}
```

## API 詳解

### コンストラクタ

- `AcMotor::AcMotor(SlaveIndex index, int encoderResolution)`

    ※ `encoderResolution` は現在未実装

    コンストラクタで、引数にスレーブのインデックスを指定します。一つのインスタンスが一つのモーターを制御するため、複数のモーターを制御する場合は、複数のインスタンスを生成することになります。

    スレーブのインデックスとは、接続順に1から始まる整数です。次のように接続している場合、スレーブのインデックスはかっこ内の数値のように振られます。

    ```txt
    マスター ⇔ スレーブ(1) ⇔ スレーブ(2) ⇔ スレーブ(3) ⇔ ...
    ```

    この場合、次のようにインスタンスを生成します。

    ```cpp
    static AcMotor Motor1{
        SlaveIndex{ 1 },  // スレーブ(1)
        4096,             // エンコーダの分解能
    };

    static AcMotor Motor2{
        SlaveIndex{ 2 },  // スレーブ(2)
        4096,             // エンコーダの分解能
    };

    static AcMotor Motor3{
        SlaveIndex{ 3 },  // スレーブ(3)
        4096,             // エンコーダの分解能
    };
    ```

### 更新処理

- `AcMotor::Update()`

    毎ループ呼び出す必要があります。EtherCATの送受信バッファーを更新する処理を行います。

### 電流指令値送信

- `AcMotor::SetCurrentRef(float CurrentRef)`

    電流指令値を設定します。単位は[A]です。

### フィードバック値取得

- `AcMotor::GetTheta() -> float`

    シャフトの機械角度を取得します。単位は[rad]です。

- `AcMotor::GetOmega() -> float`

    シャフトの機械角速度を取得します。単位は[rad/s]です。

- `AcMotor::GetIqCurrent() -> float`

    電流値(Q軸)を取得します。単位は[A]です。

- `GetState() -> StateKind`

    現在のモーターの状態を取得できます。返り値は `StateKind` 型で、次の状態が定義されています。

    ```cpp
    enum class StateKind {
        None,    // 状態なし
        Run,     // 動作中
        Stop,    // 停止中
        Error,   // エラー状態
    };
    ```

### 状態遷移指令送信

モータードライバ上の状態遷移ボタンを押すのと同じように、マスターからの指令で状態遷移させることができます。

- `AcMotor::ServoOnAsync() -> bool`

    サーボオン

- `AcMotor::ServoOffAsync() -> bool`

    サーボオフ

- `AcMotor::ErrorResetAsync() -> bool`

    エラーリセット

これらの関数は一度呼び出すだけでよく、遷移は非同期的に行われます。

またサーボオン状態で `ServoOnAsync()` を呼び出しても何も起こりません。他の状態でも同様です。

戻り値から遷移中かどうかを確認できます。状態遷移が完了すると `true` が返り、遷移中は `false` が返されます。この機能を使う場合、毎ループ呼び出し続ける必要があります。

```cpp
// ループ中

if (Motor1.ServoOnAsync()) {
    // サーボオンが完了した
} else {
    // サーボオンがまだ完了していない
}
```

## 通信仕様

次のように定義される構造体を、memcopy などでバイト列にマップし通信しています。

- マスターからスレーブへ (指令)

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

- スレーブからマスターへ (フィードバック)

    ```cpp
    struct SlaveToMasterSchema
    {
        int32_t ThetaECount;  // 角位置 [ECount]  
        int32_t OmegaECount;  // 角速度 [ECount/s]
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
