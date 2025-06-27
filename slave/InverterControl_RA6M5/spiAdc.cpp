/********************************************************************/
/* Description: ADコンバータAD7367用RSPIインターフェースの設定         */
/*              RSPI1をマスタとしSPCKを供給(並びにSSLを制御),          */
/*              スレーブ(RSPI0)はSPCKによって駆動                     */
/* File: spiAdc.cpp                                                 */
/* Date: 2024/09/15                                                 */
/* Author: Takashi YOSHIOKA                                         */
/********************************************************************/


/********************************************************************/
/* ヘッダファイルのインクルード                                        */
/********************************************************************/
#include "spiAdc.h"
#include "stateTransition.h"
#include "movingAverage.h"
#include "motorVar.h"


/********************************************************************/
/* 外部変数宣言                                                      */
/********************************************************************/
extern motorVar_t motorVar;
extern invState_t currentState;


/********************************************************************/
/* Description: RSPIの初期化                                        */
/* Function: setup_spi_adc                                          */
/* Arguments: なし                                                  */
/* Return value: なし                                               */
/********************************************************************/
void setup_spi_adc(void)
{
  // モジュールストップ解除
  R_MSTP->MSTPCRB_b.MSTPB19 = 0;    // SPI0
  R_MSTP->MSTPCRB_b.MSTPB18 = 0;    // SPI1

	// ビットレートの設定
  R_SPI0->SPBR = 1;                 // 1:分周比4 SPIベースクロック25MHz = 100MHz/4
  R_SPI0->SPCMD_b[0].BRDV = 0b00;   // 0b00:SPIベースクロックから分周なし
  R_SPI1->SPBR = 1;
  R_SPI1->SPCMD_b[0].BRDV = 0b00;

	// SPIのモード設定
  R_SPI0->SPCMD_b[0].CPOL = 1;      // 1:アイドル時のSPCKはHigh
  R_SPI0->SPCMD_b[0].CPHA = 0;      // 0:SPCK奇数エッジ(Highスタートなら立ち下がり)で値取得，SPCK偶数エッジ(Highスタートなら立ち上がり)で値変化
  R_SPI1->SPCMD_b[0].CPOL = 1;
  R_SPI1->SPCMD_b[0].CPHA = 0;

  // 転送フォーマットの設定
  R_SPI0->SPDCR_b.SPLW = 1;         // 1:SPDRはロングワード(32bit)アクセス
  R_SPI0->SPCMD_b[0].SPB = 0b0011;  // 0b0011:32bit
  R_SPI0->SPCMD_b[0].LSBF = 0;      // 0:MSBファースト
  R_SPI0->SPCMD_b[0].SSLA = 0b000;  // 0b000:SSL0
  R_SPI0->SSLP_b.SSL0P = 0;         // 0:SSL0はActive Low
  R_SPI1->SPDCR_b.SPLW = 1;
  R_SPI1->SPCMD_b[0].SPB = 0b0011;
  R_SPI1->SPCMD_b[0].LSBF = 0;
  R_SPI1->SPCMD_b[0].SSLA = 0b000;
  R_SPI1->SSLP_b.SSL0P = 0;

  // SPIインターフェース用DIOの設定
  R_PMISC->PWPR_b.B0WI = 0;
  R_PMISC->PWPR_b.PFSWE = 1;

  R_PFS->PORT[2].PIN[4].PmnPFS_b.PMR = 0;         // P204:いったん汎用入出力
  R_PFS->PORT[2].PIN[3].PmnPFS_b.PMR = 0;         // P203:いったん汎用入出力
  R_PFS->PORT[2].PIN[5].PmnPFS_b.PMR = 0;         // P205:いったん汎用入出力
  R_PFS->PORT[2].PIN[4].PmnPFS_b.PDR = 0;         // P204:入力端子
  R_PFS->PORT[2].PIN[3].PmnPFS_b.PDR = 0;         // P203:入力端子
  R_PFS->PORT[2].PIN[5].PmnPFS_b.PDR = 0;         // P205:入力端子
  R_PFS->PORT[2].PIN[4].PmnPFS_b.PSEL = 0b00110;  // P204:SPCKA
  R_PFS->PORT[2].PIN[3].PmnPFS_b.PSEL = 0b00110;  // P203:MOSIA
  R_PFS->PORT[2].PIN[5].PmnPFS_b.PSEL = 0b00110;  // P205:SSLA0
  R_PFS->PORT[2].PIN[4].PmnPFS_b.PMR = 1;         // P204(SPCKA):周辺機能用端子
  R_PFS->PORT[2].PIN[3].PmnPFS_b.PMR = 1;         // P203(MISOA):周辺機能用端子
  R_PFS->PORT[2].PIN[5].PmnPFS_b.PMR = 1;         // P205(SSLA0):周辺機能用端子
  
  R_PFS->PORT[4].PIN[12].PmnPFS_b.PMR = 0;        // P412:いったん汎用入出力
  R_PFS->PORT[4].PIN[10].PmnPFS_b.PMR = 0;        // P410:いったん汎用入出力
  R_PFS->PORT[4].PIN[13].PmnPFS_b.PMR = 0;        // P413:いったん汎用入出力
  R_PFS->PORT[4].PIN[12].PmnPFS_b.PDR = 1;        // P412:出力端子
  R_PFS->PORT[4].PIN[10].PmnPFS_b.PDR = 0;        // P410:入力端子
  R_PFS->PORT[4].PIN[13].PmnPFS_b.PDR = 1;        // P413:出力端子
  R_PFS->PORT[4].PIN[12].PmnPFS_b.PSEL = 0b00110; // P412:SPCKB
  R_PFS->PORT[4].PIN[10].PmnPFS_b.PSEL = 0b00110; // P410:MISOB
  R_PFS->PORT[4].PIN[13].PmnPFS_b.PSEL = 0b00110; // P413:SSLB0
  R_PFS->PORT[4].PIN[12].PmnPFS_b.PMR = 1;        // P412(SPCKB):周辺機能用端子
  R_PFS->PORT[4].PIN[10].PmnPFS_b.PMR = 1;        // P410(MISOB):周辺機能用端子
  R_PFS->PORT[4].PIN[13].PmnPFS_b.PMR = 1;        // P413(SSLB0):周辺機能用端子

  SPI_ADC_CONVST.PMR = 0;     // CONVSTn:汎用入出力
  SPI_ADC_CONVST.PODR = 1;    // CONVSTn:初期出力High
  SPI_ADC_CONVST.PDR = 1;     // CONVSTn:出力端子
  SPI_ADC_BUSY.PMR = 0;       // BUSY:汎用入出力
  SPI_ADC_BUSY.PDR = 0;       // BUSY:入力端子

  R_PMISC->PWPR_b.PFSWE = 0;
  R_PMISC->PWPR_b.B0WI = 1;

  // SPIスレーブモードの設定
  R_SPI0->SPCR_b.MODFEN = 0;  // 0:モードフォルトエラー無効
  R_SPI0->SPCR_b.SPMS = 0;    // 0:4線式SPI
  R_SPI0->SPCR_b.TXMD = 0;    // 0:全二重モード
  R_SPI0->SPCR_b.MSTR = 0;    // 0:SPIスレーブ

  // SPIシングルマスタモードの設定
  R_SPI1->SPCR_b.MODFEN = 0;
  R_SPI1->SPCR_b.SPMS = 0;
  R_SPI1->SPCR_b.TXMD = 0;
  R_SPI1->SPCR_b.MSTR = 1;    // 1:SPIマスタ
  // クロックタイミングの設定
  R_SPI1->SPCMD_b[0].SCKDEN = 1;  // 1:SPCK遅延許可
  R_SPI1->SPCKD = 0;              // SSLアサート後，val+1周期分だけSPCK遅延

  uint8_t dummy;
  dummy = R_SPI0->SPCR;       //　ダミー読み出し
  dummy = R_SPI1->SPCR;
}


/********************************************************************/
/* Description: SPI ADCからの値取得                                  */
/* Function: get_spi_adc                                            */
/* Arguments: なし                                                  */
/* Return value: なし                                               */
/********************************************************************/
void get_spi_adc(void)
{
  // AD変換開始
  SPI_ADC_CONVST_PODR = 0;      // CONVSTをアサート
  SPI_ADC_CONVST_PODR = 1;      // BUSYフラグが立つまでLowを維持しているとパワーダウンするので即戻す
  while ( SPI_ADC_BUSY.PIDR == 1 );   // BUSYフラグが立っている間待機

  R_SPI0->SPCR_b.SPE = 1;       // 受信直前にSPIを有効化
  R_SPI1->SPCR_b.SPE = 1;

  // ダミー送信
  uint8_t dummy;
  dummy = R_SPI0->SPSR;         // ダミー読み出し
  dummy = R_SPI1->SPSR;
  R_SPI0->SPDR = 0x00000000;    // ダミーデータ送信
  R_SPI1->SPDR = 0x00000000;

  // SPIデータ受信
  dummy = R_SPI0->SPSR;         // ダミー読み出し
  dummy = R_SPI1->SPSR;
  while ( R_SPI0->SPSR_b.SPRF == 0 );   // 受信終了フラグが立つまで待機
  while ( R_SPI1->SPSR_b.SPRF == 0 );
  
  // 受信データの格納
  packedDWord_t rcvData1;
  packedDWord_t rcvData2;
  rcvData1.DWord = (int32_t)(R_SPI0->SPDR);    // データ受信
  rcvData2.DWord = (int32_t)(R_SPI1->SPDR);

  R_SPI0->SPCR_b.SPE = 0;       // 受信終了後はSPIを無効化
  R_SPI1->SPCR_b.SPE = 0;

  // AD変換値の取り出し
  rcvData1.DWord = rcvData1.DWord >> 2;                   // signed型なので算術シフト扱い 2bitシフトで上位wordは位置が合う
  motorVar.IuRaw = (int16_t)(rcvData1.Word.H) - OFFSET_CODE_IUVW;     // 上位wordの取り出し
  motorVar.IvRaw = (int16_t)(rcvData1.Word.L >> 2) - OFFSET_CODE_IUVW;// 下位wordはもう2bitシフトが必要
  rcvData2.DWord = rcvData2.DWord >> 2;
  motorVar.IwRaw = (int16_t)(rcvData2.Word.H) - OFFSET_CODE_IUVW;
  motorVar.VdcRaw = (int16_t)(rcvData2.Word.L >> 2) - OFFSET_CODE_VDC;

  motorVar.Iu = ADVAL_TO_IUVW*(motorVar.IuRaw - motorVar.IuOffs);
  motorVar.Iv = ADVAL_TO_IUVW*(motorVar.IvRaw - motorVar.IvOffs);
  motorVar.Iw = ADVAL_TO_IUVW*(motorVar.IwRaw - motorVar.IwOffs);
  motorVar.Vdc = ADVAL_TO_VDC*motorVar.VdcRaw;
}


/********************************************************************/
/* Description: 電流オフセットの算出 GB中に実施すること                */
/* Function: calc_cur_offset                                        */
/* Arguments: なし                                                  */
/* Return value: なし                                               */
/********************************************************************/
void calc_cur_offset(void)
{
  static int16_t IuBuf[CURR_OFFS_AVE_NUM+1] = {0};	  // 0初期化 +1しているのは現在値の分
  static int16_t IvBuf[CURR_OFFS_AVE_NUM+1] = {0};
  static int16_t IwBuf[CURR_OFFS_AVE_NUM+1] = {0};

  if ( currentState == stateStop ) {    // 停止状態(ゲートブロック中)は都度オフセット値を更新
    IuBuf[CURR_OFFS_AVE_NUM] = motorVar.IuRaw;
    IvBuf[CURR_OFFS_AVE_NUM] = motorVar.IvRaw;
    IwBuf[CURR_OFFS_AVE_NUM] = motorVar.IwRaw;

    motorVar.IuOffs = calcMovAve(IuBuf, CURR_OFFS_AVE_NUM);
    motorVar.IvOffs = calcMovAve(IvBuf, CURR_OFFS_AVE_NUM);
    motorVar.IwOffs = calcMovAve(IwBuf, CURR_OFFS_AVE_NUM);
  }
  else {
    // do nothing
  }
}