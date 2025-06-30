/********************************************************************/
/* Description: GPTモジュールおよびPWM出力周りの設定                   */
/* File: pwm.h                                                      */
/* Date: 2024/08/18                                                 */
/* Author: Takashi YOSHIOKA                                         */
/********************************************************************/


/********************************************************************/
/* 多重インクルード防止                                               */
/********************************************************************/
#ifndef _PWM_H_
#define _PWM_H_


/********************************************************************/
/* ヘッダファイルのインクルード                                       */
/********************************************************************/
#include <Arduino.h>
#include "motorVar.h"


/********************************************************************/
/* 各種定数定義                                                      */
/********************************************************************/
static constexpr uint32_t PWM_TIMER_FREQ = 100000;    // GPT timer frequency [kHz]

static constexpr uint16_t CARRIER_PERIOD_CYC = (uint16_t)(PWM_TIMER_FREQ/(2*CARRIER_FREQ/1000));
static constexpr uint16_t DEAD_TIME_CYC = (uint16_t)(PWM_TIMER_FREQ*DEAD_TIME/1000000);
static constexpr uint16_t INIT_DUTY_CYC = CARRIER_PERIOD_CYC/2;


/********************************************************************/
/* 外部公開関数のプロトタイプ宣言																		   */
/********************************************************************/
void setup_pwm_scan(void);
void set_pwm_duty(float duty_u, float duty_v, float duty_w);
void enable_pwm_out(void);
void disable_pwm_out(void);
 

#endif /* END _PWM_H_ */