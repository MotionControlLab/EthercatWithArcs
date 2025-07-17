#pragma once

#include <cstdint>
#include <cmath>

/********************************************************************/
/* 各種定数定義                                                      */
/********************************************************************/
static constexpr float PARAM_MTR_R = 0.36f;            // [Ohm] モータ巻線抵抗
static constexpr float PARAM_MTR_L = 0.2e-3f;          // [H] モータインダクタンス
static constexpr float PARAM_MTR_PHI_A = 7.833e-3f;    // [V/(rad/s)] 鎖交磁束 (二相換算)

static constexpr float PARAM_MTR_J = 1e-5f;    // [kgm^2] モータイナーシャ

static constexpr uint16_t ENC_PULSE = 1000;      // [pulse/rev] 1逓倍あたりのエンコーダパルス数
static constexpr uint16_t ENC_MUL = 4;           // [-] N逓倍
static constexpr uint16_t MTR_POLE_NUM = 8;      // [-] モータ極数 (NOT 極対数)
static constexpr uint16_t ENC_Z_OFFSET = 320;    // [count] エンコーダZ相とU相誘起電圧ゼロクロスのオフセット(実測値)

// static constexpr float    INV_VDC         = 24.0f;      // [V] 直流リンク電圧 (実測Vdcを使用するverでは不要)

static constexpr uint32_t CARRIER_FREQ = 20000;    // [Hz] キャリア周波数
static constexpr uint16_t DEAD_TIME = 250;         // [ns] デッドタイム
static constexpr uint16_t PERIOD_ASR = 500;        // [us] ASR演算周期
static constexpr uint16_t PERIOD_SEQ = 10000;      // [us] シーケンス演算周期

static constexpr float PARAM_MTR_MAXSPD = 2.0f * M_PI * 120.0f;       // [rad/s] 最大回転速度
static constexpr float ALMLEVEL_OVERSPD = PARAM_MTR_MAXSPD * 1.2f;    // [rad/s] オーバースピードの閾値 最大回転速度の1.2倍

static constexpr uint16_t MTR_POLE_PAIR = MTR_POLE_NUM / 2;    // [-] モータ極対数
static constexpr float PARAM_MTR_KE = PARAM_MTR_PHI_A;         // [V/(rad/s)] 誘起電圧定数 (二相換算)

/********************************************************************/
/* 各種定数定義                                                      */
/********************************************************************/
static constexpr uint16_t ENC_COEFF = ENC_PULSE * ENC_MUL;               // [count/rev] エンコーダ係数
static constexpr uint16_t ENC_COEFF_ELEC = ENC_COEFF / MTR_POLE_PAIR;    // [count/rev] 電気角あたりのエンコーダ係数

static constexpr float ENC_TO_OMEGAE = 2.0f * M_PI * 1000000.0f / ((float)PERIOD_ASR * ENC_COEFF_ELEC);    // 2π[rad/rev]*p[-]*1000000/(Tasr[us]*ENC[count/rev])

static constexpr float ECOUNT_TO_RADI = (2.0f * M_PI) / (ENC_COEFF * MTR_POLE_PAIR);

    
/********************************************************************/
/* 各種定数定義                                                      */
/********************************************************************/
static constexpr float ACR_CUTOFF   = 2.0f*M_PI*1000.0f;  // [rad/s] 電流制御 カットオフ角周波数
static constexpr float ACR_ZETA     = 1.0f;               // [-]     電流制御 減衰係数
static constexpr float ASR_CUTOFF   = 2.0f*M_PI*20.0f;    // [rad/s] 速度制御 カットオフ角周波数
static constexpr float ASR_ZETA     = 1.0f;               // [-]     速度制御 減衰係数

static constexpr float ASR_GAIN_KPS = MTR_POLE_PAIR * 2.0f*ASR_ZETA*ASR_CUTOFF*PARAM_MTR_J/(MTR_POLE_PAIR*MTR_POLE_PAIR*PARAM_MTR_PHI_A); // [-] 速度制御 Pゲイン  Kps = 2*zs*ws*J/(p^2*phi_a)
static constexpr float ASR_GAIN_KIS = MTR_POLE_PAIR * ASR_CUTOFF*ASR_CUTOFF*PARAM_MTR_J/(MTR_POLE_PAIR*MTR_POLE_PAIR*PARAM_MTR_PHI_A);    // [-] 速度制御 Iゲイン  Kis = ws^2*J/(p^2*phi_a)

#include "EthercatSlave.hh"

class AcMotor
{
public:
    enum class StateKind : uint8_t
    {
        None,
        Run,
        Stop,
        Error,
    };

    AcMotor(SlaveIndex Index) noexcept
        : Sender(Index)
        , Receiver(Index)
    {
    }

    /// @brief 電流指令を設定
    /// @param IqCurrentRef [A] 電流指令値
    void SetCurrentRef(float IqCurrentRef) noexcept
    {
        MasterToSlaveData.IqCurrentRef = IqCurrentRef;
    }


    /// @brief サーボON指令を非同期で送信
    /// @return true: サーボON状態に移行完了, false: サーボON状態に移行中
    bool ServoOnAsync() noexcept
    {
        if (SlaveToMasterData.State == StateKind::Run)
            return true;
        MasterToSlaveData.Control = MasterToSlaveSchema::ControlKind::ServoOn;
        return false;    // Run状態に移行中
    }

    /// @brief サーボOFF指令を非同期で送信
    /// @return true: サーボOFF状態に移行完了, false: サーボOFF状態に移行中
    bool ServoOffAsync() noexcept
    {
        if (SlaveToMasterData.State == StateKind::Stop)
            return true;
        MasterToSlaveData.Control = MasterToSlaveSchema::ControlKind::ServoOff;
        return false;    // Stop状態に移行中
    }

    /// @brief エラー解除指令を非同期で送信
    /// @return true: エラー解除完了, false: エラー解除中
    bool ResetErrorAsync() noexcept
    {
        if (SlaveToMasterData.State != StateKind::Error)
            return true;
        MasterToSlaveData.Control = MasterToSlaveSchema::ControlKind::ResetError;
        return false;    // Error解除中
    }

    /// @brief 更新
    /// @return true: データ更新成功, false: 失敗
    bool Update() noexcept
    {
        if (const auto Data = Receiver.GetData())
        {
            // データ受信成功
            SlaveToMasterData = *Data;
        }
        else
        {
            // データ受信失敗
            // std::cerr << "[x] Failed to receive data from slave." << std::endl;
            return false;
        }

        // 指令状態がモーターに反映されているかを確認し、反映されていたら指令を消去する
        // ドライバ上の状態遷移ボタンと共存させるためにこうしている (常に指令が送られるとOR条件をとれなくなるため)
        if (MasterToSlaveData.Control == MasterToSlaveSchema::ControlKind::ResetError &&
            SlaveToMasterData.State != StateKind::Error)
        {
            MasterToSlaveData.Control = MasterToSlaveSchema::ControlKind::None;
        }

        if (MasterToSlaveData.Control == MasterToSlaveSchema::ControlKind::ServoOn &&
            SlaveToMasterData.State == StateKind::Run)
        {
            MasterToSlaveData.Control = MasterToSlaveSchema::ControlKind::None;
        }

        if (MasterToSlaveData.Control == MasterToSlaveSchema::ControlKind::ServoOff &&
            SlaveToMasterData.State == StateKind::Stop)
        {
            MasterToSlaveData.Control = MasterToSlaveSchema::ControlKind::None;
        }

        Sender.SetData(MasterToSlaveData);

        return true;
    }


    /// @brief 機械角を取得
    /// @return 機械角 [rad]
    float GetTheta() const noexcept
    {
        return static_cast<float>(SlaveToMasterData.ThetaECount) * ECOUNT_TO_RADI;
    }

    /// @brief 角速度を取得
    /// @return 角速度 [rad/s]
    float GetOmega() const noexcept
    {
        return static_cast<float>(SlaveToMasterData.OmegaECount) * ECOUNT_TO_RADI;
    }

    /// @brief 電流を取得
    /// @return 電流 [A]
    float GetIqCurrent() const noexcept
    {
        return SlaveToMasterData.IqCurrent;
    }

    /// @brief モーターの状態を取得
    /// @return モーターの状態
    ///         - Run: サーボON状態
    ///         - Stop: サーボOFF状態
    ///         - Error: エラー状態
    ///         - None: 状態不明
    StateKind GetState() const noexcept
    {
        return SlaveToMasterData.State;
    }

private:
    struct MasterToSlaveSchema
    {
        float IqCurrentRef;

        enum class ControlKind : uint8_t
        {
            None,
            ServoOn,
            ServoOff,
            ResetError,
        } Control : 2;
    } __attribute__((packed));

    struct SlaveToMasterSchema
    {
        int32_t ThetaECount;
        int32_t OmegaECount;
        float IqCurrent;

        StateKind State : 2;
    } __attribute__((packed));

    EthercatSender<MasterToSlaveSchema> Sender;
    MasterToSlaveSchema MasterToSlaveData{};

    EthercatReceiver<SlaveToMasterSchema> Receiver;
    SlaveToMasterSchema SlaveToMasterData{};

};
