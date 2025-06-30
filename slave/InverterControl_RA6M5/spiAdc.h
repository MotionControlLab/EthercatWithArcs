/********************************************************************/
/* Description: ADコンバータAD736x用SPIインターフェースの設定          */
/* File: spiAdc.h                                                   */
/* Date: 2024/08/20                                                 */
/* Author: Takashi YOSHIOKA                                         */
/********************************************************************/


/********************************************************************/
/* 多重インクルード防止                                               */
/********************************************************************/
#ifndef _SPIADC_H_
#define _SPIADC_H_


/********************************************************************/
/* ヘッダファイルのインクルード                                       */
/********************************************************************/
#include <Arduino.h>


/********************************************************************/
/* マクロ定義                                                        */
/********************************************************************/
#define SPI_ADC_CONVST      (R_PFS->PORT[4].PIN[14].PmnPFS_b) // CONVSTnのピンアサイン
#define SPI_ADC_CONVST_PODR (R_PORT4->PODR_b.PODR14)	        // CONVSTnのピンアサイン:P414 (ロック解除しないとPFSへの書き込みができないため，解除不要のエイリアスを設定)
#define SPI_ADC_BUSY		    (R_PFS->PORT[4].PIN[15].PmnPFS_b)	// BUSYのピンアサイン
#define SPI_ADC_CONVST_PIDR (R_PORT4->PODR_b.PIDR15)	        // BUSYのピンアサイン:P415


/********************************************************************/
/* 各種定数定義                                                      */
/********************************************************************/
static constexpr uint16_t ADC_RES_DIV2      = 8192; // ADCの正方向分解能 2^(x-1)

static constexpr uint16_t OFFSET_CODE_IUVW  = 2650; // 電流センサ信号(Iuvw)のオフセット
static constexpr uint16_t OFFSET_CODE_VDC   = -77;  // 電圧センサ信号(Vdc)のオフセット

static constexpr uint16_t CURR_OFFS_AVE_NUM = 64;   // 電流オフセット算出時の移動平均回数

static constexpr float ADVAL_TO_IUVW = -10.0f*5.0f/(float)ADC_RES_DIV2;      // (10A/1Vadc)*(+5Vadc/8192) BOOSTXL-3PHGANINVは相電流が符号反転しているので戻しておく
//static constexpr float ADVAL_TO_VDC = 5.0f*80.0f/(3.3f*(float)ADC_RES_DIV2); // (80Vdc/3.3Vadc)*(+5Vadc/8192)
static constexpr float ADVAL_TO_VDC = 0.01506319f;                           // 実測値


/********************************************************************/
/* パック共用体の定義                                                */
/********************************************************************/
typedef union {
  int32_t DWord;
  struct {
    int16_t L;
    int16_t H;
  }Word;
}packedDWord_t;


/********************************************************************/
/* 外部公開関数のプロトタイプ宣言																		   */
/********************************************************************/
void setup_spi_adc(void);
void get_spi_adc(void);
void calc_cur_offset(void);


#endif /* END _SPIADC_H_ */