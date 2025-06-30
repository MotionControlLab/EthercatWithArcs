/********************************************************************/
/* Description: 各種座標変換                                         */
/* File: coordTrans.h                                               */
/* Date: 2024/07/19                                                 */
/* Author: Takashi YOSHIOKA                                         */
/********************************************************************/


/********************************************************************/
/* 多重インクルード防止                                               */
/********************************************************************/
#ifndef	_COORDTRANS_H_
#define _COORDTRANS_H_


/********************************************************************/
/* ヘッダファイルのインクルード                                       */
/********************************************************************/
#include <Arduino.h>


/********************************************************************/
/* 外部公開関数のプロトタイプ宣言																		   */
/********************************************************************/
void trans_uvw_to_ab(float UphArg, float VphArg, float WphArg, float *AphArg, float *BphArg);
void trans_ab_to_uvw(float AphArg, float BphArg, float *UphArg, float *VphArg, float *WphArg);

void trans_ab_to_dq(float AphArg, float BphArg, uint16_t countThetaArg, float *DphArg, float *QphArg);
void trans_dq_to_ab(float DphArg, float QphArg, uint16_t countThetaArg, float *AphArg, float *BphArg);


#endif /* END _COORDTRANS_H_ */