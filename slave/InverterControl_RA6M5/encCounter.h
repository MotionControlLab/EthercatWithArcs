/********************************************************************/
/* Description: GPT位相計数モードを用いたAB相エンコーダカウンタの設定   */
/* File: encCounter.h                                               */
/* Date: 2024/10/10                                                 */
/* Author: Takashi YOSHIOKA                                         */
/********************************************************************/


/********************************************************************/
/* 多重インクルード防止                                               */
/********************************************************************/
#ifndef _ENCCOUNTER_H_
#define _ENCCOUNTER_H_


/********************************************************************/
/* ヘッダファイルのインクルード                                       */
/********************************************************************/
#include <Arduino.h>
#include <math.h>
#include "motorVar.h"


/********************************************************************/
/* 各種定数定義                                                      */
/********************************************************************/
static constexpr uint16_t ENC_COEFF = ENC_PULSE*ENC_MUL;            // [count/rev] エンコーダ係数
static constexpr uint16_t ENC_COEFF_ELEC = ENC_COEFF/MTR_POLE_PAIR; // [count/rev] 電気角あたりのエンコーダ係数

static constexpr float ENC_TO_OMEGAE = 2.0f*M_PI*1000000.0f / ((float)PERIOD_ASR*ENC_COEFF_ELEC); // 2π[rad/rev]*p[-]*1000000/(Tasr[us]*ENC[count/rev])

static constexpr uint16_t DIFF_ERR_THRESH = 1000;   // エンコーダ差分値のひげ対策の閾値


/********************************************************************/
/* 外部公開関数のプロトタイプ宣言																		   */
/********************************************************************/
void setup_enc_counter(void);
uint16_t get_enc_count(void);
void get_enc_pos_speed(void);


#endif /* END _ENCCOUNTER_H_ */