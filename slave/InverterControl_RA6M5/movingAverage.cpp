/********************************************************************/
/* Description: 移動平均値の算出                                      */
/* File: movingAverage.cpp                                          */
/* Date: 2024/08/20                                                 */
/* Author: Takashi YOSHIOKA                                         */
/********************************************************************/


/********************************************************************/
/* ヘッダファイルのインクルード                                        */
/********************************************************************/
#include "movingAverage.h"


/********************************************************************/
/* Description: 移動平均値の計算                                     */
/* Function: calcMovAve                                             */
/* Arguments: int16_t型配列ポインタ                                  */
/*            (個数はVR_AVE_NUMで規定, 配列末尾が最新値)              */
/*            uint16_t型 移動平均のデータ個数                         */
/* Return value: int16_t型  移動平均値                               */
/********************************************************************/
int16_t calcMovAve(int16_t bufArg[], uint16_t datNumArg)
{
	int16_t dataSumTmp = 0;
	uint16_t datIdxTmp;
	int16_t aveDataTmp;

	// N回の移動平均
	for (datIdxTmp = 1; datIdxTmp <= datNumArg; datIdxTmp++) {  // 0番目は移動平均の計算に使用しないため1から開始
		dataSumTmp += bufArg[datIdxTmp];              // 値の積算
		bufArg[datIdxTmp-1] = bufArg[datIdxTmp];      // FIFOバッファを1つずらす (このときバッファ0番目にアクセスする)
	}
	aveDataTmp = (int16_t)(dataSumTmp / datNumArg); // 最後に割る

	return aveDataTmp;
}