/********************************************************************/
/* Description: 各種座標変換                                         */
/* File: coordTrans.cpp                                             */
/* Date: 2024/07/19                                                 */
/* Author: Takashi YOSHIOKA                                         */
/********************************************************************/


/********************************************************************/
/* ヘッダファイルのインクルード                                       */
/********************************************************************/
#include <math.h>
#include "coordTrans.h"
#include "sinTable.h"


/********************************************************************/
/* 外部変数宣言                                                      */
/********************************************************************/
extern "C" float sinTable[NUM_SINTBL];


/********************************************************************/
/* Description: 三相->二相変換	(注:絶対変換)                         */
/* Function: trans_uvw_to_ab                                        */
/* Arguments: UphArg [V] or [A] 入力側 float型                       */
/*            VphArg [V] or [A] 入力側 float型                       */
/*            WphArg [V] or [A] 入力側 float型                       */
/*            *AphArg [V] or [A] 出力側 float型ポインタ               */
/*            *BphArg [V] or [A] 出力側 float型ポインタ               */
/* Return value: なし                                                */
/********************************************************************/
void trans_uvw_to_ab(float UphArg, float VphArg, float WphArg, float *AphArg, float *BphArg)
{
	const float C11 = 8.1650e-01, C12 = 4.0825e-01, C13 = C12;
	const float C22 = 7.0711e-01, C23 = C22;

	*AphArg = C11*UphArg - C12*VphArg - C13*WphArg;	  // [ Aph ] = sqrt(2/3)*[ 1*Uph -       (1/2)*Vph -       (1/2)*Wph]
	*BphArg =              C22*VphArg - C23*WphArg;	  // [ Bph ] =           [ 0*Uph + (sqrt(3)/2)*Vph - (sqrt(3)/2)*Wph]
}


/********************************************************************/
/* Description: 二相->三相変換	(注:絶対変換)                         */
/* Function: trans_ab_to_uvw                                        */
/* Arguments: AphArg [V] or [A] 入力側 float型                       */
/*            BphArg [V] or [A] 入力側 float型                       */
/*            *UphArg [V] or [A] 出力側 float型ポインタ               */
/*            *VphArg [V] or [A] 出力側 float型ポインタ               */
/*            *WphArg [V] or [A] 出力側 float型ポインタ               */
/* Return value: なし                                                */
/********************************************************************/
void trans_ab_to_uvw(float AphArg, float BphArg, float *UphArg, float *VphArg, float *WphArg)
{
	const float C11 = 8.1650e-01;
	const float C21 = 4.0825e-01, C22 = 7.0711e-01;
	const float C31 = C21, C32 = C22;

	*UphArg =   C11*AphArg;				  			  // [ Uph ] =           [       1*Aph +           0*Bph]
	*VphArg = - C21*AphArg + C22*BphArg;		// [ Vph ] = sqrt(2/3)*[ - (1/2)*Aph + (sqrt(3)/2)*Bph]
	*WphArg = - C31*AphArg - C32*BphArg;		// [ Wph ] =           [ - (1/2)*Aph - (sqrt(3)/2)*Bph]
}


/********************************************************************/
/* Description: αβ->dq変換 (順変換)                                  */
/* Function: trans_ab_to_dq                                         */
/* Arguments: AphArg [V] or [A] 入力側 float型                       */
/*            BphArg [V] or [A] 入力側 float型                       */
/*            countThetaArg [count]   入力側 uint16_t型              */
/*            *DphArg [V] or [A] 出力側 float型ポインタ               */
/*            *QphArg [V] or [A] 出力側 float型ポインタ               */
/* Return value: なし                                                */
/********************************************************************/
void trans_ab_to_dq(float AphArg, float BphArg, uint16_t countThetaArg, float *DphArg, float *QphArg)
{
	uint16_t sinIndexTmp, cosIndexTmp;

	sinIndexTmp = countThetaArg;
	cosIndexTmp = sinIndexTmp + THETA_PIDIV2;						        // cos用のインデックスはpi/2(512)だけ位相を進めておく
	if (cosIndexTmp > NUM_SINTBL) cosIndexTmp -= NUM_SINTBL;    // 一周したらリセット

	*DphArg =   sinTable[cosIndexTmp]*AphArg + sinTable[sinIndexTmp]*BphArg;	// [ Dph ] = [   cos(ThetaE)*Aph + sin(ThetaE)*Bph]
	*QphArg = - sinTable[sinIndexTmp]*AphArg + sinTable[cosIndexTmp]*BphArg;	// [ Qph ] = [ - sin(ThetaE)*Aph + cos(ThetaE)*Bph]
}


/********************************************************************/
/* Description: dq->αβ変換 (逆変換)                                  */
/* Function: trans_dq_to_ab                                         */
/* Arguments: DphArg [V] or [A] 入力側 float型                       */
/*            QphArg [V] or [A] 入力側 float型                       */
/*            countThetaArg [count]   入力側 uint16_t型              */
/*            *AphArg [V] or [A] 出力側 float型ポインタ               */
/*            *BphArg [V] or [A] 出力側 float型ポインタ               */
/* Return value: なし                                                */
/********************************************************************/
void trans_dq_to_ab(float DphArg, float QphArg, uint16_t countThetaArg, float *AphArg, float *BphArg)
{
	uint16_t sinIndexTmp, cosIndexTmp;

	sinIndexTmp = countThetaArg;
	cosIndexTmp = sinIndexTmp + THETA_PIDIV2;						      // cos用のインデックスはpi/2(512)だけ位相を進めておく
	if (cosIndexTmp > NUM_SINTBL) cosIndexTmp -= NUM_SINTBL;  // 一周したらリセット

	*AphArg =   sinTable[cosIndexTmp]*DphArg - sinTable[sinIndexTmp]*QphArg;	// [ Aph ] = [   cos(ThetaE)*Dph - sin(ThetaE)*Qph]
	*BphArg =   sinTable[sinIndexTmp]*DphArg + sinTable[cosIndexTmp]*QphArg;	// [ Bph ] = [   sin(ThetaE)*Dph + cos(ThetaE)*Qph]
}