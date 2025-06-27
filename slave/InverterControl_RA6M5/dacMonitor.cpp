/********************************************************************/
/* Description: DAC12を用いたアナログモニタの設定                     */
/* File: dacMonitor.cpp                                             */
/* Date: 2024/08/20                                                 */
/* Author: Takashi YOSHIOKA                                         */
/********************************************************************/


/********************************************************************/
/* ヘッダファイルのインクルード                                        */
/********************************************************************/
#include "dacMonitor.h"


/********************************************************************/
/* Description: DAC12の初期化                                        */
/* Function: setup_dac_monitor                                      */
/* Arguments: なし                                                  */
/* Return value: なし                                               */
/********************************************************************/
void setup_dac_monitor(void)
{
  // モジュールストップ解除
  R_MSTP->MSTPCRD_b.MSTPD20 = 0;    // DAC12

  R_PMISC->PWPR_b.B0WI = 0;   // PFSロック解除(1)
  R_PMISC->PWPR_b.PFSWE = 1;  // PFSロック解除(2)

	R_PFS->PORT[0].PIN[14].PmnPFS_b.PMR = 0;  // P014(=A6)(DA0)を汎用入出力に設定
	R_PFS->PORT[0].PIN[14].PmnPFS_b.PDR = 0;  // P014(=A6)(DA0)を入力端子として使用
	R_PFS->PORT[0].PIN[14].PmnPFS_b.ASEL = 0; // P014(=A6)(DA0)をアナログ入力端子として使用しない

	R_PFS->PORT[0].PIN[15].PmnPFS_b.PMR = 0;  // P015(=A5)(DA1)を汎用入出力に設定
	R_PFS->PORT[0].PIN[15].PmnPFS_b.PDR = 0;  // P015(=A5)(DA1)を入力端子として使用
	R_PFS->PORT[0].PIN[15].PmnPFS_b.ASEL = 0; // P015(=A5)(DA1)をアナログ入力端子として使用しない

  R_PMISC->PWPR_b.PFSWE = 0;  // PFSロック(1)
  R_PMISC->PWPR_b.B0WI = 1;   // PFSロック(2)
  
  R_DAC->DADPR_b.DPSEL = 0;         // 0:フォーマット右詰め
  R_DAC->DAADSCR_b.DAADST = 0;      // 0:DAC12とADC14の動作は同期させない

  R_DAC->DACR_b.DAOE0 = 0;          // DA出力禁止
  R_DAC->DACR_b.DAOE1 = 0;
  R_DAC->DADR[0] = DACOUT_CEN;      // 初期化のためゼロイング
  R_DAC->DADR[1] = DACOUT_CEN;
  R_DAC->DAAMPCR_b.DAAMP0 = 0;      // 出力アンプ無効
  R_DAC->DAAMPCR_b.DAAMP1 = 0;

  R_DAC->DACR = 0b01111111;         // DA出力有効化
}


/********************************************************************/
/* Description: DAC12からのアナログモニタ値出力                       */
/* Function: set_dac_out                                            */
/* Arguments: int16_t型配列 値域:-2048 ~ 2047                        */
/* Return value: なし                                               */
/********************************************************************/
void set_dac_out(int16_t dacOutArg[])
{
  int16_t dacOutTmp[DACOUT_NUM];

  for (int8_t i = 0; i < DACOUT_NUM; i++) {
    dacOutTmp[i] = dacOutArg[i];
    if (dacOutTmp[i] > DACOUT_MAX) dacOutTmp[i] = DACOUT_MAX; // 引数のリミット処理
    if (dacOutTmp[i] < DACOUT_MIN) dacOutTmp[i] = DACOUT_MIN;
    dacOutTmp[i] += DACOUT_CEN;                               // -2048 -- 2047 を 0 -- 4095にオフセット
  }

  R_DAC->DADR[0] = dacOutTmp[0];
  R_DAC->DADR[1] = dacOutTmp[1];
}