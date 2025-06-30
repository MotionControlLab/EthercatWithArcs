/********************************************************************/
/* Description: GPT位相計数モードを用いたエンコーダカウンタの設定       */
/* File: encCounter.cpp                                             */
/* Date: 2024/10/10                                                 */
/* Author: Takashi YOSHIOKA                                         */
/********************************************************************/


/********************************************************************/
/* ヘッダファイルのインクルード                                        */
/********************************************************************/
#include <IRQManager.h>
#include "encCounter.h"
#include "sinTable.h"
#include "motorVar.h"


/********************************************************************/
/* 外部変数宣言                                                      */
/********************************************************************/
extern motorVar_t motorVar;


/********************************************************************/
/* ソース内グローバルクラスの定義                                      */
/********************************************************************/
static GenericIrqCfg_t cfg;


/********************************************************************/
/* ソース内関数のプロトタイプ宣言                                      */
/********************************************************************/
static void MYISR_IRQ4_INT(void);


/********************************************************************/
/* Description: GPT2 位相計数モードの初期化                           */
/* Function: setup_enc_counter                                      */
/* Arguments: なし                                                  */
/* Return value: なし                                               */
/********************************************************************/
void setup_enc_counter(void)
{
  // モジュールストップ解除
  R_MSTP->MSTPCRE_b.MSTPE29 = 0;    // GPT2

  // GPTの設定
  R_GPT2->GTCR_b.CST = 0;           // カウントストップ
  R_GPT2->GTUPSR = 0x6900;          // 位相計数モード1に設定
  R_GPT2->GTDNSR = 0x9600;
  R_GPT2->GTPR = ENC_COEFF - 1;     // 機械角1回転毎にカウンタクリア
  
  // IRQ検出設定
  R_ICU->IRQCR_b[4].IRQMD = 0b01;   // 01:立ち上がりでIRQ発生
  R_ICU->IRQCR_b[4].FLTEN = 0;      // 0:デジタルフィルタ無効
  
  R_PMISC->PWPR_b.B0WI = 0;
  R_PMISC->PWPR_b.PFSWE = 1;
  R_PFS->PORT[1].PIN[3].PmnPFS_b.PMR = 0;     // J2-36(P103)を汎用入出力に設定
  R_PFS->PORT[1].PIN[2].PmnPFS_b.PMR = 0;     // J2-38(P102)を汎用入出力に設定
  R_PFS->PORT[1].PIN[3].PmnPFS_b.PDR = 0;     // J2-36(P103)を入力端子として使用
  R_PFS->PORT[1].PIN[2].PmnPFS_b.PDR = 0;     // J2-38(P102)を入力端子として使用
  R_PFS->PORT[1].PIN[3].PmnPFS_b.PSEL = 0x03; // J2-36(P103)をGPT端子(エンコーダA相)として使用
  R_PFS->PORT[1].PIN[2].PmnPFS_b.PSEL = 0x03; // J2-38(P102)をGPT端子(エンコーダB相)として使用
  R_PFS->PORT[1].PIN[3].PmnPFS_b.PMR = 1;     // J2-36(P103)を周辺機能用の端子に設定
  R_PFS->PORT[1].PIN[2].PmnPFS_b.PMR = 1;     // J2-38(P102)を周辺機能用の端子に設定

  R_PFS->PORT[1].PIN[11].PmnPFS_b.PMR = 0;     // J2-63(P111)を汎用入出力に設定
  R_PFS->PORT[1].PIN[11].PmnPFS_b.PDR = 0;     // J2-63(P111)を入力端子として使用
  R_PFS->PORT[1].PIN[11].PmnPFS_b.ISEL = 1;    // J2-63(P111)をIRQ(IRQ4)端子として使用
  R_PFS->PORT[1].PIN[11].PmnPFS_b.PMR = 1;     // J2-63(P111)を周辺機能用の端子に設定
  R_PMISC->PWPR_b.PFSWE = 0;
  R_PMISC->PWPR_b.B0WI = 1;

  // 割り込み設定
  cfg.irq = FSP_INVALID_VECTOR;     // 良く分からんが固定値
  cfg.ipl = 2;                      // 割り込み優先度 0(最高)-15(最低)
  cfg.event = ELC_EVENT_ICU_IRQ4;   // 割り込み要因としてIRQ4割り込みを指定
  IRQManager::getInstance().addGenericInterrupt(cfg, MYISR_IRQ4_INT);   // 割り込み要因のセット
  
  R_GPT2->GTCR_b.CST = 1;           // カウントスタート
}


/********************************************************************/
/* Description: エンコーダカウント値の取得                            */
/* Function: get_enc_count                                          */
/* Arguments: uint16_t型                                            */
/* Return value: なし                                               */
/********************************************************************/
uint16_t get_enc_count(void)
{
  uint16_t countRaw, countTmp, countIniComp, count, thetaEcount;

  countRaw = (uint16_t)(R_GPT2->GTCNT);
  countTmp = countRaw % ENC_COEFF_ELEC;    // 電気角スケール内に収まるようラッピング

  countIniComp = ((countTmp + ENC_COEFF_ELEC) - motorVar.thetaEIni) % ENC_COEFF_ELEC;  // 初期磁極位置分を差し引いてから再ラッピング 
                                                                                // 1電気角分だけ加算してから初期磁極位置を差し引くことで，減算結果が負の値にならないようにする
  count = ((countIniComp + ENC_COEFF_ELEC) - ENC_Z_OFFSET) % ENC_COEFF_ELEC;    // Z相オフセット位置分を差し引いてから再ラッピング 
                                                                                // 1電気角分だけ加算してから初期磁極位置を差し引くことで，減算結果が負の値にならないようにする
  
  thetaEcount = (uint16_t)(count*NUM_SINTBL/ENC_COEFF_ELEC);  // エンコーダカウント値をsinテーブルindex値に換算
  return thetaEcount;
}


/********************************************************************/
/* Description: 割り込みサービスルーチン(IRQ4割り込み)                 */
/* Function: MYISR_IRQ4_INT                                         */
/* Arguments: なし                                                  */
/* Return value: なし                                               */
/********************************************************************/
void MYISR_IRQ4_INT(void)
{
  uint16_t countIniTmp;
  static bool thetaMIniFlag = true;

  countIniTmp = (uint16_t)(R_GPT2->GTCNT);  // Z相パルス入力時のカウント値を保持
  motorVar.thetaEIni = countIniTmp % ENC_COEFF_ELEC;  // 初期磁極位置が電気角スケール内に収まるようラッピング

  if ( thetaMIniFlag ) {
    if ( R_GPT2->GTST_b.TUCF == 1 ) {       // アップカウント？ (1:アップカウント)
      motorVar.thetaMIni = (int16_t)countIniTmp;      // アップカウントでZ相を検出した場合は初期機械角をそのままセット
    }
    else {
      motorVar.thetaMIni = (int16_t)countIniTmp - ENC_COEFF;  // ダウンカウントでZ相を検出した場合は初期機械角を補正してセット
    }
    thetaMIniFlag = false;
  }
  else {
    // Nothing to do
  }

  // IRQ割り込み禁止処理
  /*R_PMISC->PWPR_b.B0WI = 0;
  R_PMISC->PWPR_b.PFSWE = 1;
  R_PFS->PORT[1].PIN[11].PmnPFS_b.ISEL = 0;   // J2-63(P111)のIRQを禁止 (Z相割り込みは起動後1回だけ実行すれば良い)
  R_PFS->PORT[1].PIN[11].PmnPFS_b.PMR = 0;    // J2-63(P111)を汎用入出力に再設定
  R_PMISC->PWPR_b.PFSWE = 0;
  R_PMISC->PWPR_b.B0WI = 1;*/

  R_ICU->IELSR_b[cfg.irq].IR = 0; // 割り込み要因フラグのクリア
}


/********************************************************************/
/* Description: エンコーダカウント値・差分値の取得                     */
/* Function: get_enc_pos_speed                                      */
/* Arguments: なし                                                  */
/* Return value: なし                                               */
/********************************************************************/
void get_enc_pos_speed(void)
{
  int16_t countDiff;
  static uint16_t countPre = 0;
  static int16_t countDiffPre = 0;

  static int32_t rotNum = 0;
  static int32_t thetaMPre = 0;

  uint16_t countRaw = (uint16_t)(R_GPT2->GTCNT);
  if ( R_GPT2->GTST_b.TCFPO == 1 ) {    // カウンタオーバーフロー(OV)時はOV発生前の状態に戻して差分を計算
    countDiff = ((int16_t)(countRaw) + ENC_COEFF) - (int16_t)(countPre);
    rotNum++;
    R_GPT2->GTST_b.TCFPO = 0;
  }
  else {
    if ( R_GPT2->GTST_b.TCFPU == 1 ) {  // カウンタアンダーフロー(UV)時はUV発生前の状態に戻して差分を計算
      countDiff = (int16_t)(countRaw) - ((int16_t)(countPre) + ENC_COEFF);
      rotNum--;
      R_GPT2->GTST_b.TCFPU = 0;
    }
    else {                              // 何もなければそのまま差分計算
      countDiff = (int16_t)(countRaw) - (int16_t)(countPre);
    }
  }

  int32_t thetaMCount = (rotNum*ENC_COEFF + countRaw) - motorVar.thetaMIni; // N回転 + 現回転カウント値 - Z相基準初期位相

  // 希に(数十秒～数分程度)に1回ほどの頻度でエンコーダ差分値にひげが乗るのでその対策
  // カウント値がジャギっている場合, OV/UV処理が不要の場合も(一瞬だけ限界突破して)OV/UVフラグが立つことがあり,
  // 結果として本来必要のないカウント値補正をやってしまうことがある？
  int16_t countDiffDiff = countDiff - countDiffPre;   // 差分の差分を算出
  while ( countDiffDiff > DIFF_ERR_THRESH ) {         // 差分の差分が閾値以内に収まるまでエンコーダ係数の整数倍を加減算してひげを相殺
    countDiff -= ENC_COEFF;
    countDiffDiff = countDiff - countDiffPre;
  }
  while ( countDiffDiff < -DIFF_ERR_THRESH ) {
    countDiff += ENC_COEFF;
    countDiffDiff = countDiff - countDiffPre;
  }

  // 同様に, エンコーダカウント値に段差が生じる場合にはrotNumの値を補正
  int32_t thetaMDiff = thetaMCount - thetaMPre;   // 差分を算出
  while ( thetaMDiff > DIFF_ERR_THRESH ) {        // 差分が閾値以内に収まるまでエンコーダ係数の整数倍を加減算してひげを相殺
    rotNum--;
    thetaMCount = (rotNum*ENC_COEFF + countRaw) - motorVar.thetaMIni;
    thetaMDiff = thetaMCount - thetaMPre;
  }
  while ( thetaMDiff < -DIFF_ERR_THRESH ) {
    rotNum++;
    thetaMCount = (rotNum*ENC_COEFF + countRaw) - motorVar.thetaMIni;
    thetaMDiff = thetaMCount - thetaMPre;
  }

  countPre = countRaw;        // 次サンプリング用にカウント値を保存
  countDiffPre = countDiff;
  thetaMPre = thetaMCount;

  motorVar.thetaMCount = thetaMCount;
  motorVar.omegaE = ENC_TO_OMEGAE*(float)(countDiff);
}