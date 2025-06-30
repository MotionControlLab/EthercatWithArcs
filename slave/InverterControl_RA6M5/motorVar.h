/********************************************************************/
/* Description: モータ状態変数構造体の定義                            */
/* File: motorVar.h                                                 */
/* Date: 2024/10/10                                                 */
/* Author: Takashi YOSHIOKA                                         */
/********************************************************************/


/********************************************************************/
/* 多重インクルード防止                                               */
/********************************************************************/
#ifndef	_MOTORVAR_H_
#define _MOTORVAR_H_


/********************************************************************/
/* ヘッダファイルのインクルード                                       */
/********************************************************************/
#include <math.h>


/********************************************************************/
/* 各種定数定義                                                      */
/********************************************************************/
static constexpr float    PARAM_MTR_R     = 0.36f;      // [Ohm] モータ巻線抵抗
static constexpr float    PARAM_MTR_L     = 0.2e-3f;    // [H] モータインダクタンス
static constexpr float    PARAM_MTR_PHI_A = 7.833e-3f;  // [V/(rad/s)] 鎖交磁束 (二相換算)

static constexpr float    PARAM_MTR_J     = 1e-5f;      // [kgm^2] モータイナーシャ

static constexpr uint16_t ENC_PULSE       = 1000;       // [pulse/rev] 1逓倍あたりのエンコーダパルス数
static constexpr uint16_t ENC_MUL         = 4;		      // [-] N逓倍
static constexpr uint16_t MTR_POLE_NUM    = 8;          // [-] モータ極数 (NOT 極対数)
static constexpr uint16_t ENC_Z_OFFSET    = 320;		    // [count] エンコーダZ相とU相誘起電圧ゼロクロスのオフセット(実測値)

//static constexpr float    INV_VDC         = 24.0f;      // [V] 直流リンク電圧 (実測Vdcを使用するverでは不要)

static constexpr uint32_t CARRIER_FREQ    = 20000;      // [Hz] キャリア周波数
static constexpr uint16_t DEAD_TIME       = 250;        // [ns] デッドタイム
static constexpr uint16_t PERIOD_ASR	    = 1000;		    // [us] ASR演算周期
static constexpr uint16_t PERIOD_SEQ      = 10000;		  // [us] シーケンス演算周期

static constexpr float    PARAM_MTR_MAXSPD = 2.0f*M_PI*120.0f;        // [rad/s] 最大回転速度
static constexpr float    ALMLEVEL_OVERSPD = PARAM_MTR_MAXSPD*1.2f;   // [rad/s] オーバースピードの閾値 最大回転速度の1.2倍

static constexpr uint16_t MTR_POLE_PAIR   = MTR_POLE_NUM/2;   // [-] モータ極対数
static constexpr float    PARAM_MTR_KE    = PARAM_MTR_PHI_A;  // [V/(rad/s)] 誘起電圧定数 (二相換算)


/********************************************************************/
/* モータ状態変数構造体の定義								                           */
/********************************************************************/
typedef struct {
	float VuRef;    // [V] u相電圧指令 VuRef
	float VvRef;    // [V] v相電圧指令 VvRef
	float VwRef;    // [V] w相電圧指令 VwRef
	float Iu;		    // [A] u相電流検出値 Iu
	float Iv;       // [A] v相電流検出値 Iv
	float Iw;		    // [A] w相電流検出値 Iw

	float VaRef;	  // [V] α軸電圧指令 VaRed
	float VbRef;	  // [V] β軸電圧指令 VbRef
	float Ia;		    // [A] α軸電流検出値 Ia
	float Ib;		    // [A] β軸電流検出値 Ib

	float VdRef;	  // [V] d軸電圧指令 VdRef
	float VqRef;	  // [V] q軸電圧指令 VqRef
	float Id;		    // [A] d軸電流検出値 Id
	float Iq;		    // [A] q軸電流検出値 Iq
	float IdRef;	  // [A] d軸電圧指令 IdRef
	float IqRef;	  // [A] q軸電圧指令 IqRef
  
	float VdAcrOut;	// [V] d軸電流制御器出力 VdAcrOut
	float VqAcrOut;	// [V] q軸電流制御器出力 VqAcrOut
	float VdComp;	  // [V] d軸非干渉制御補償量 VdComp
	float VqComp;	  // [V] q軸非干渉制御補償量 VqComp

	float Vdc;		  // [V] 直流リンク電圧検出値 Vdc
	//float Idc;		  // [A] 直流リンク電流検出値 Idc

	float omegaERef;// [rad/s] 角速度指令(電気角) ωeRef
	float omegaE;   // [rad/s] 角速度指令(電気角) ωe
  
	float IdErrInt;     // [-] d軸電流偏差積分値 Wid
	float IqErrInt;     // [-] q軸電流偏差積分値 Wiq
	float omegaEErrInt; // [-] 角速度偏差積分値(電気角) Wwe

	uint16_t thetaECount; // [count] エンコーダ位相(電気角) θe
	uint16_t thetaEIni;		// [count] エンコーダ初期位相(電気角) θeini

	int32_t thetaMCount;  // [count] エンコーダ位相(機械角) θm
	int16_t thetaMIni;		// [count] エンコーダ初期位相(機械角) θmini

	int16_t IuRaw;  // [ADval] 出力電流Iuの生データ
	int16_t IvRaw;  // [ADval] 出力電流Ivの生データ
	int16_t IwRaw;  // [ADval] 出力電流Iwの生データ
	int16_t VdcRaw; // [ADval] 出力電流Vdcの生データ
	int16_t IuOffs; // [ADval] 出力電流Iuのオフセット値
	int16_t IvOffs; // [ADval] 出力電流Ivのオフセット値
	int16_t IwOffs; // [ADval] 出力電流Iwのオフセット値

	int16_t dummy;  // [-] for alignment
}motorVar_t;


#endif /* END _MOTORVAR_H_ */