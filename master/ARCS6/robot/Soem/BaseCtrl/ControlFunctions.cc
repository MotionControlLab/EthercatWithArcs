//! @file ControlFunctions.cc
//! @brief 制御用周期実行関数群クラス
//! @date 2025/03/23
//! @author Yokokura, Yuki
//
// Copyright (C) 2011-2025 Yokokura, Yuki
// MIT License. For details, see the LICENSE file.

// 基本のインクルードファイル
#include <cmath>
#include <iostream>
#include <chrono>
#include <thread>
#include "ARCSprint.hh"
#include "ARCSassert.hh"
#include "ARCSmemory.hh"
#include "ARCSscrparams.hh"
#include "ARCSgraphics.hh"
#include "ConstParams.hh"
#include "ControlFunctions.hh"
#include "InterfaceFunctions.hh"

// SOEMラッパー
#include "EthercatBus.hh"
#include "EthercatSlave.hh"
#include "AcMotor.hh"

// 追加のARCSライブラリをここに記述
#include "ArcsMatrix.hh"

using namespace ARCS;

//! @brief スレッド間通信用グローバル変数の無名名前空間
namespace
{
    // スレッド間で共有したい変数をここに記述
    ArcsMat<EquipParams::ACTUATOR_NUM, 1> thm;      //!< [rad]  位置ベクトル
    ArcsMat<EquipParams::ACTUATOR_NUM, 1> iqref;    //!< [A,Nm] 電流指令,トルク指令ベクトル
}    // namespace

//! @brief 制御用周期実行関数1
//! @param[in]	t		時刻 [s]
//! @param[in]	Tact	計測周期 [s]
//! @param[in]	Tcmp	消費時間 [s]
//! @return		クロックオーバーライドフラグ (true = リアルタイムループ, false = 非リアルタイムループ)
bool ControlFunctions::ControlFunction1(const double t, const double Tact, const double Tcmp)
{
    // 制御用定数設定
    [[maybe_unused]] constexpr double Ts = ConstParams::SAMPLING_TIME[0] * 1e-9;    // [s]	制御周期

    // 制御用変数宣言
    static EthercatBus Bus;

    static AcMotor AcMotor{
        SlaveIndex{ 1 },
    };

    // PIController{ 0.0201, 1.2600, Ts }
    
    static EthercatReceiver<int> Volume{ SlaveIndex{ 2 } };

    if (CmdFlag == CTRL_INIT)
    {
        // 初期化モード (ここは制御開始時/再開時に1度だけ呼び出される(非リアルタイム空間なので重い処理もOK))
        Initializing = true;          // 初期化中ランプ点灯
        Screen.InitOnlineSetVar();    // オンライン設定変数の初期値の設定
        Interface.ServoON();          // サーボON指令の送出
        Initializing = false;         // 初期化中ランプ消灯

        switch (Bus.Init("enp1s0"))
        {
        case EthercatBus::InitState::ALL_SLAVES_OP_STATE:
            // std::cout << "[o] All slaves are in OP state." << std::endl;
            break;
        case EthercatBus::InitState::PORT_OPEN_FAILED:
            std::cout << "[x] Port open failed." << std::endl;
            return 1;
        case EthercatBus::InitState::SLAVES_NOT_FOUND:
            std::cout << "[x] Slaves found but not all are in OP state." << std::endl;
            return 2;
        case EthercatBus::InitState::NOT_ALL_OP_STATE:
            std::cout << "[x] Not all slaves are in OP state." << std::endl;
            return 3;
        }
    }
    if (CmdFlag == CTRL_LOOP)
    {

        // 周期モード (ここは制御周期 SAMPLING_TIME[0] 毎に呼び出される(リアルタイム空間なので処理は制御周期内に収めること))
        // リアルタイム制御ここから
        Interface.GetPosition(thm);    // [rad] 位置ベクトルの取得

        Interface.SetCurrent(iqref);    // [A] 電流指令ベクトルの出力
        Graph.SetTime(Tact, t);         // [s] グラフ描画用の周期と時刻のセット


        Bus.Update();


        // if (const auto TargetVelocity = Volume.GetData())
        // {
        //     AcMotor.SetTargetVelocity(*TargetVelocity);
        //     Screen.SetVarIndicator(*TargetVelocity, 0, 0, 0, 0, 0, 0, 0, 0, 0);    // 任意変数インジケータ(変数0, ..., 変数9)
        // }
        // else
        // {
        //     std::cout << "[x] Failed to get target velocity from Volume." << std::endl;
        // }
        AcMotor.SetCurrentRef(0.5);
        
        // オンライン設定用変数の書き換えを入力として使う"(-""-)"
        const auto Input = [&](int varIndex) -> bool
        {
            std::array<double, ARCSparams::ONLINEVARS_MAX> vars;
            Screen.GetOnlineSetVars(vars);    // オンライン設定変数の読み込み

            double value = vars.at(varIndex);    // オンライン設定変数の値を取得

            if (value <= 0.0)
                return false;

            std::cout << "[o] Online set variable " << varIndex << " changed to " << value << std::endl;

            // 書き換えられた場合
            Screen.SetOnlineSetVar(varIndex, 0.0);
            return true;
        };

        if (Input(1))
            AcMotor.ResetErrorAsync();
        if (Input(2))
            AcMotor.ServoOnAsync();
        if (Input(3))
            AcMotor.ServoOffAsync();

        AcMotor.Update();

        Screen.SetVarIndicator(AcMotor.GetTheta(), // [rev] モーターの回転角 (2πで割って回転数に変換)
                               AcMotor.GetOmega(),    // [rad/s] モーターの角速度
                               AcMotor.GetIqCurrent(),      // [A] モーターの電流
                               static_cast<uint8_t>(AcMotor.GetState()));    // モーターの状態

        Graph.SetVars(0, AcMotor.GetTheta());
        Graph.SetVars(1, AcMotor.GetOmega());
        Graph.SetVars(2, AcMotor.GetIqCurrent());
        Graph.SetVars(3, (uint8_t)AcMotor.GetState());

        UsrGraph.SetVars(0, 10);                                // ユーザカスタムプロット（例）
        Memory.SetData(Tact, t, 0, 0, 0, 0, 0, 0, 0, 0, 0);    // CSVデータ保存変数 (周期, A列, B列, ..., J列)
                                                               // リアルタイム制御ここまで
    }
    if (CmdFlag == CTRL_EXIT)
    {
        // 終了処理モード (ここは制御終了時に1度だけ呼び出される(非リアルタイム空間なので重い処理もOK))
        Interface.SetZeroCurrent();    // 電流指令を零に設定
        Interface.ServoOFF();          // サーボOFF信号の送出
    }
    return true;    // クロックオーバーライドフラグ(falseにすると次の周期時刻を待たずにスレッドが即刻動作する)
}

//! @brief 制御用周期実行関数2
//! @param[in]	t		時刻 [s]
//! @param[in]	Tact	計測周期 [s]
//! @param[in]	Tcmp	消費時間 [s]
//! @return		クロックオーバーライドフラグ (true = リアルタイムループ, false = 非リアルタイムループ)
bool ControlFunctions::ControlFunction2(const double t, const double Tact, const double Tcmp)
{
    // 制御用定数宣言
    [[maybe_unused]] constexpr double Ts = ConstParams::SAMPLING_TIME[1] * 1e-9;    // [s]	制御周期

    // 制御用変数宣言

    // 制御器等々の宣言

    if (CmdFlag == CTRL_INIT)
    {
        // 初期化モード (ここは制御開始時/再開時に1度だけ呼び出される(非リアルタイム空間なので重い処理もOK))
    }
    if (CmdFlag == CTRL_LOOP)
    {
        // 周期モード (ここは制御周期 SAMPLING_TIME[1] 毎に呼び出される(リアルタイム空間なので処理は制御周期内に収めること))
        // リアルタイム制御ここから

        // リアルタイム制御ここまで
    }
    if (CmdFlag == CTRL_EXIT)
    {
        // 終了処理モード (ここは制御終了時に1度だけ呼び出される(非リアルタイム空間なので重い処理もOK))
    }
    return true;    // クロックオーバーライドフラグ(falseにすると次の周期時刻を待たずにスレッドが即刻動作する)
}

//! @brief 制御用周期実行関数3
//! @param[in]	t		時刻 [s]
//! @param[in]	Tact	計測周期 [s]
//! @param[in]	Tcmp	消費時間 [s]
//! @return		クロックオーバーライドフラグ (true = リアルタイムループ, false = 非リアルタイムループ)
bool ControlFunctions::ControlFunction3(const double t, const double Tact, const double Tcmp)
{
    // 制御用定数宣言
    [[maybe_unused]] constexpr double Ts = ConstParams::SAMPLING_TIME[2] * 1e-9;    // [s]	制御周期

    // 制御用変数宣言

    if (CmdFlag == CTRL_INIT)
    {
        // 初期化モード (ここは制御開始時/再開時に1度だけ呼び出される(非リアルタイム空間なので重い処理もOK))
    }
    if (CmdFlag == CTRL_LOOP)
    {
        // 周期モード (ここは制御周期 SAMPLING_TIME[2] 毎に呼び出される(リアルタイム空間なので処理は制御周期内に収めること))
        // リアルタイム制御ここから

        // リアルタイム制御ここまで
    }
    if (CmdFlag == CTRL_EXIT)
    {
        // 終了処理モード (ここは制御終了時に1度だけ呼び出される(非リアルタイム空間なので重い処理もOK))
    }
    return true;    // クロックオーバーライドフラグ(falseにすると次の周期時刻を待たずにスレッドが即刻動作する)
}

//! @brief 制御用変数値を更新する関数
void ControlFunctions::UpdateControlValue(void)
{
    // ARCS画面パラメータに値を書き込む
    Screen.SetNetworkLink(NetworkLink);          // ネットワークリンクフラグを書き込む
    Screen.SetInitializing(Initializing);        // ロボット初期化フラグを書き込む
    Screen.SetCurrentAndPosition(iqref, thm);    // 電流指令と位置を書き込む
}
