/********************************************************************/
/* Description: ベクトル制御に基づく各種モータ制御演算                 */
/* File: motorControl.h                                             */
/* Date: 2024/08/20                                                 */
/* Author: Takashi YOSHIOKA                                         */
/********************************************************************/


/********************************************************************/
/* 多重インクルード防止                                               */
/********************************************************************/
#ifndef	_MOTORCONTROL_H_
#define _MOTORCONTROL_H_


/********************************************************************/
/* ヘッダファイルのインクルード                                       */
/********************************************************************/
#include <Arduino.h>
#include <math.h>
#include "motorVar.h"


/********************************************************************/
/* 各種定数定義                                                      */
/********************************************************************/
static constexpr float ACR_CUTOFF   = 2.0f*M_PI*1000.0f;  // [rad/s] 電流制御 カットオフ角周波数
static constexpr float ACR_ZETA     = 1.0f;               // [-]     電流制御 減衰係数
static constexpr float ASR_CUTOFF   = 2.0f*M_PI*20.0f;    // [rad/s] 速度制御 カットオフ角周波数
static constexpr float ASR_ZETA     = 1.0f;               // [-]     速度制御 減衰係数

static constexpr float ACR_GAIN_KPD = 2.0f*ACR_ZETA*ACR_CUTOFF*PARAM_MTR_L - PARAM_MTR_R; // [-] d軸電流制御 Pゲイン  Kpd = 2*zc*wc*L-R
static constexpr float ACR_GAIN_KID = ACR_CUTOFF*ACR_CUTOFF*PARAM_MTR_L;                  // [-] d軸電流制御 Iゲイン  Kid = omega^2*L
static constexpr float ACR_GAIN_KPQ = 2.0f*ACR_ZETA*ACR_CUTOFF*PARAM_MTR_L - PARAM_MTR_R; // [-] q軸電流制御 Pゲイン  Kpq = 2*zc*wc*L-R
static constexpr float ACR_GAIN_KIQ = ACR_CUTOFF*ACR_CUTOFF*PARAM_MTR_L;                  // [-] q軸電流制御 Iゲイン  Kiq = wc^2*L

static constexpr float ASR_GAIN_KPS = 2.0f*ASR_ZETA*ASR_CUTOFF*PARAM_MTR_J/(MTR_POLE_PAIR*MTR_POLE_PAIR*PARAM_MTR_PHI_A); // [-] 速度制御 Pゲイン  Kps = 2*zs*ws*J/(p^2*phi_a)
static constexpr float ASR_GAIN_KIS = ASR_CUTOFF*ASR_CUTOFF*PARAM_MTR_J/(MTR_POLE_PAIR*MTR_POLE_PAIR*PARAM_MTR_PHI_A);    // [-] 速度制御 Iゲイン  Kis = ws^2*J/(p^2*phi_a)


/********************************************************************/
/* 外部公開関数のプロトタイプ宣言																		   */
/********************************************************************/
void calc_cur_control(float IdRefArg, float IqRefArg, float IdArg, float fIqArg, float *VdRefArg, float *VqRefArg);
void calc_spd_control(float OmegaERefArg, float OmegaEArg, float *IqRefArg);
void calc_decoupling_control(float IdArg, float IqArg, float omegaEArg, float *VdCompArg, float *VqCompArg);


#endif /* END _MOTORCONTROL_H_ */