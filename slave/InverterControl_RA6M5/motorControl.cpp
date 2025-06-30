/********************************************************************/
/* Description: ベクトル制御に基づく各種モータ制御演算                 */
/* File: motorControl.cpp                                           */
/* Date: 2024/08/20                                                 */
/* Author: Takashi YOSHIOKA                                         */
/********************************************************************/


/********************************************************************/
/* ヘッダファイルのインクルード                                       */
/********************************************************************/
#include "motorControl.h"
#include "pwm.h"
#include "asrTimer.h"
#include "motorVar.h"


/********************************************************************/
/* 外部変数宣言                                                      */
/********************************************************************/
extern motorVar_t motorVar;


/********************************************************************/
/* Description: dq軸電流指令&電流からdq軸電圧指令を演算                */
/* Function: calc_cur_control                                       */
/* Arguments: IdRefArg [A] 入力側 float型                            */
/*            IqRefArg [A] 入力側 float型                            */
/*            IdArg [A] 入力側 float型                               */
/*            IqArg [A] 入力側 float型                               */
/*            *VdRefArg [V] 出力側 float型ポインタ                    */
/*            *fqRefArg [V] 出力側 float型ポインタ                    */
/* Return value: なし                                                */
/********************************************************************/
void calc_cur_control(float IdRefArg, float IqRefArg, float IdArg, float IqArg, float *VdRefArg, float *VqRefArg)
{
	float IdErrTmp;
	float IqErrTmp;

  IdErrTmp = IdRefArg - IdArg;					// 偏差の算出
	IqErrTmp = IqRefArg - IqArg;
  
	motorVar.IdErrInt += IdErrTmp/((float)CARRIER_FREQ);	// 偏差積分値の算出
	motorVar.IqErrInt += IqErrTmp/((float)CARRIER_FREQ);
    
	//*VdRefArg = ACR_GAIN_KPD*IdErrTmp + ACR_GAIN_KID*motorVar.IdErrInt;   // PI制御
	//*VqRefArg = ACR_GAIN_KPQ*IqErrTmp + ACR_GAIN_KIQ*motorVar.IqErrInt;
	*VdRefArg = - ACR_GAIN_KPD*IdArg + ACR_GAIN_KID*motorVar.IdErrInt;   // I-P制御
	*VqRefArg = - ACR_GAIN_KPQ*IqArg + ACR_GAIN_KIQ*motorVar.IqErrInt;
}


/********************************************************************/
/* Description: 角速度指令&角速度からq軸電流指令を演算		             */
/* Function: calc_spd_control                                       */
/* Arguments: omegaERefArg [rad/s] 入力側 float型                    */
/*            omegaEArg [rad/s] 入力側 float型                       */
/*            *IqRefArg [A] 出力側 float型ポインタ                    */
/* Return value: なし                                                */
/********************************************************************/
void calc_spd_control(float omegaERefArg, float omegaEArg, float *IqRefArg)
{
	float omegaErrTmp;

  omegaErrTmp = omegaERefArg - omegaEArg;				    // 偏差の算出

  motorVar.omegaEErrInt += omegaErrTmp*PERIOD_ASR/1000000.0f; // 偏差積分値の算出
  
  //*IqRefArg = ASR_GAIN_KPS*omegaErrTmp + ASR_GAIN_KIS*motorVar.omegaEErrInt;  // PI制御
  *IqRefArg = - ASR_GAIN_KPS*omegaEArg + ASR_GAIN_KIS*motorVar.omegaEErrInt;  // I-P制御
}


/********************************************************************/
/* Description: 非干渉制御に基づくdq軸電圧補償量を演算		             */
/* Function: calc_decoupling_control                                */
/* Arguments: IdArg [A] 入力側 float型                               */
/*            IqArg [A] 入力側 float型                               */
/*            omegaEArg [rad/s] 入力側 float型						           */
/*            *VdCompArg [V] 出力側 float型ポインタ                   */
/*            *VqCompArg [V] 出力側 float型ポインタ                   */
/* Return value: なし                                                */
/********************************************************************/
void calc_decoupling_control(float IdArg, float IqArg, float omegaEArg, float *VdCompArg, float *VqCompArg)
{
	*VdCompArg = - omegaEArg*PARAM_MTR_L*IqArg;                     // Vdcomp = -we*L*Iq
	*VqCompArg =   omegaEArg*(PARAM_MTR_L*IdArg + PARAM_MTR_KE);    // Vqcomp =  we*(L*Id + Ke)
}