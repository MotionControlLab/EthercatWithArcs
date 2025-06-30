/********************************************************************/
/* Description: オペレータ入出力の初期設定および状態取得                */
/* File: operator.cpp                                               */
/* Date: 2024/08/18                                                 */
/* Author: Takashi YOSHIOKA                                         */
/********************************************************************/


/********************************************************************/
/* ヘッダファイルのインクルード                                        */
/********************************************************************/
#include "operator.h"
#include "stateTransition.h"
#include "easyCat.h"


/********************************************************************/
/* 外部変数宣言                                                      */
/********************************************************************/
extern invState_t currentState;
extern pramBuffer_out_t bufferOut;


/********************************************************************/
/* Description: オペレータ入出力用I/Oポートの初期化                    */
/* Function: setup_operator                                         */
/* Arguments: なし                                                  */
/* Return value: なし                                               */
/********************************************************************/
void setup_operator(void)
{
  // オペレータ入出力用DIOの設定
  R_PMISC->PWPR_b.B0WI = 0;   // PFSロック解除(1)
  R_PMISC->PWPR_b.PFSWE = 1;  // PFSロック解除(2)

	OPR_INV_ON.PMR = 0;		      // invOnN:汎用入出力
	OPR_INV_ON.PDR = 0;		      // invOnN:入力

	OPR_INV_OFF.PMR = 0;		    // invOffN:汎用入出力
	OPR_INV_OFF.PDR = 0;		    // invOffN:入力

	OPR_INV_RST.PMR = 0;		    // invRstN:汎用入出力
	OPR_INV_RST.PDR = 0;		    // invRstN:入力

	OPR_RUN_MON.PMR = 0;		    // runMonN:汎用入出力
	OPR_RUN_MON.PODR = 1;		    // runMonN:初期出力High (LowでLEDが点灯するため：LED自体は負論理)
	OPR_RUN_MON.PDR = 1;		    // runMonN:出力

	OPR_ERR_MON.PMR = 0;		    // errMonN:汎用入出力
	OPR_ERR_MON.PODR = 1;		    // errMonN:初期出力High (LowでLEDが点灯するため：LED自体は負論理)
	OPR_ERR_MON.PDR = 1;		    // errMonN:出力

	OPR_REFIN.PMR = 0;		      // refIn:汎用入出力
	OPR_REFIN.PDR = 0;		      // refIn:入力
	OPR_REFIN.ASEL = 1;		      // refIn:アナログ入力

  R_PMISC->PWPR_b.PFSWE = 0;  // PFSロック(1)
  R_PMISC->PWPR_b.B0WI = 1;   // PFSロック(2)

  // モジュールストップ解除
  R_MSTP->MSTPCRD_b.MSTPD16 = 0;  // ADC12_0

  // オペレータ入力用ADCの設定
	R_ADC0->ADCSR_b.ADST = 0;		    // 明示的にAD変換OFF

	R_ADC0->ADCSR_b.ADCS = 0b00;		// 0b00:シングルスキャンモード
	R_ADC0->ADCSR_b.ADHSC = 0;      // 0:高速変換モード
  R_ADC0->ADANSA_b[0].ANSA0 = 1;  // 変換対象:AN000
  
  R_ADC0->ADSSTR[0] = 30;			    // ADサンプリング時間:30ステート=0.6us
                                  // (AD変換クロック:ADCLK = PCLKC = 50MHz)
                                  // さらに逐次変換時間0.666usを加えたものが全AD変換時間となる

  R_ADC0->ADCER_b.ADPRC = 0b00;   // 0b00:変換精度12bit
  R_ADC0->ADCER_b.ADRFMT = 0;     // 0:データ右詰め
}


/********************************************************************/
/* Description: オペレータ入力状態の取得                              */
/* Function: get_opr_status                                         */
/* Arguments: oprSts_t構造体のポインタ                                */
/* Return value: なし                                               */
/********************************************************************/
void get_opr_status(oprSts_t *oprStsArg)
{
	// oprStsArg->invOn = ~OPR_INV_ON.PIDR;            // プルアップにより平常時Highのため反転してセット
	// oprStsArg->invOff = ~OPR_INV_OFF.PIDR;
	// oprStsArg->invRst = ~OPR_INV_RST.PIDR;
	oprStsArg->invOn = bufferOut.DATA.ServoOn;            // プルアップにより平常時Highのため反転してセット
	oprStsArg->invOff = not bufferOut.DATA.ServoOn;
	oprStsArg->invRst = bufferOut.DATA.ResetError;

	oprStsArg->invErr = R_GPT_POEG0->POEGG_b.PIDF;  // POEG出力禁止フラグを直接反映
  
	R_ADC0->ADCSR_b.ADST = 1;		            // AD変換スタート
	while (R_ADC0->ADCSR_b.ADST == 1);      // AD変換終了まで待機 (1.25us)
  oprStsArg->refIn = (int16_t)((R_ADC0->ADDR_b[0].ADDR) >> 2);  // VR入力については高い精度は不要なため10bit分解能に縮小
}


/********************************************************************/
/* Description: オペレータ出力状態の設定                              */
/* Function: sset_opr_status                                        */
/* Arguments: oprSts_t構造体のポインタ                                */
/* Return value: なし                                               */
/********************************************************************/
void set_opr_status(oprSts_t *oprStsArg)
{
  switch ( currentState ) {
    case stateStop:
      OPR_RUN_MON_PODR = 1;     // stateStop中は出力なし LEDも点灯なし
	    OPR_ERR_MON_PODR = 1;
			break;
    
    case stateRun:
      OPR_RUN_MON_PODR = 0;     // stateRun中はRUN LEDを点灯＆RUNを出力
	    OPR_ERR_MON_PODR = 1;
			break;
    
    case stateError:
      OPR_RUN_MON_PODR = 1;
	    OPR_ERR_MON_PODR = 0;     // stateError中はERR LEDを点灯＆ERRを出力
			break;
    
    default:
      // Nothing to do
			break;
  }
}