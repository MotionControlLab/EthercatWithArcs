/********************************************************************/
/* Description: 定時実行される制御演算部の記述                         */
/* File: controlFunction.cpp                                        */
/* Date: 2024/10/10                                                 */
/* Author: Takashi YOSHIOKA                                         */
/********************************************************************/


/********************************************************************/
/* ヘッダファイルのインクルード                                        */
/********************************************************************/
#include "controlFunction.h"
#include "operator.h"
#include "stateTransition.h"
#include "pwm.h"
#include "sequenceTimer.h"
#include "asrTimer.h"
#include "dacMonitor.h"
#include "encCounter.h"
#include "spiAdc.h"
#include "coordTrans.h"
#include "motorVar.h"
#include "motorControl.h"
#include "easyCat.h"


/********************************************************************/
/* グローバル変数の実体定義                                           */
/********************************************************************/
oprSts_t oprSts;
invState_t currentState = stateStop;
motorVar_t motorVar;
int16_t dacOut[DACOUT_NUM];

uint8_t ecatSts;


/********************************************************************/
/* 外部変数宣言                                                      */
/********************************************************************/
extern pramBuffer_out_t bufferOut;
extern pramBuffer_in_t bufferIn;


/********************************************************************/
/* Description: キャリア谷スキャン割り込み処理                         */
/* Function: carrier_valley_scan                                    */
/* Arguments: (実質的に)なし                                         */
/* Return value: なし                                               */
/********************************************************************/
void carrier_valley_scan(void)
{
	int16_t refIn;
  
  //R_PORT6->PODR_b.PODR1 = 1;    // 演算時間測定用
  
  // キャリア谷スキャンの頭で検出処理を実施
  motorVar.thetaECount = get_enc_count();
  get_spi_adc();

  // dq軸電流電流の演算
  trans_uvw_to_ab(motorVar.Iu, motorVar.Iv, motorVar.Iw, &(motorVar.Ia), &(motorVar.Ib));
  trans_ab_to_dq(motorVar.Ia, motorVar.Ib, motorVar.thetaECount, &(motorVar.Id), &(motorVar.Iq));

  switch ( currentState ) {
    case stateStop:
      refIn = 0;
      motorVar.omegaERef = 0.0f;    // 再起動時に値が残っているとバグりそうなものを初期化
      motorVar.IdRef = 0.0f;
      motorVar.IqRef = 0.0f;
      motorVar.IdErrInt = 0.0f;
      motorVar.IqErrInt = 0.0f;
      motorVar.omegaEErrInt = 0.0f;
			break;

    case stateRun:
      // VR入力値の取得
      refIn = bufferOut.DATA.CurrentRef;
      // refIn = oprSts.refIn - 12;        // 0~1023を-12~1011にシフト
      // refIn = (int16_t)(bufferOut.LONG[0]) - 12;        // 0~1023を-12~1011にシフト
	    // if (refIn > 1000) refIn = 1000;   // >1000はクランプ (=元の値で1012~1023は不感帯)
	    // if (refIn < 0) refIn = 0;				  // <0はクランプ (=元の値で0~12は不感帯)
      
      // VR入力値を速度指令(電気角)に換算
      motorVar.omegaERef = PARAM_MTR_MAXSPD*refIn/1000.0f;  // 120Hz(電気角) = 30Hz(機械角) = 1800rpm


      // dq軸電流制御 q軸電流指令はASRスキャンにて演算
      motorVar.IdRef = 0.0f;
			calc_cur_control(motorVar.IdRef, motorVar.IqRef, motorVar.Id, motorVar.Iq, &(motorVar.VdAcrOut), &(motorVar.VqAcrOut));

      // 非干渉制御
			calc_decoupling_control(motorVar.Id, motorVar.Iq, motorVar.omegaE, &(motorVar.VdComp), &(motorVar.VqComp));
      motorVar.VdRef = motorVar.VdAcrOut + motorVar.VdComp;
      motorVar.VqRef = motorVar.VqAcrOut + motorVar.VqComp;

      // dq変換により三相電圧指令を演算
      trans_dq_to_ab(motorVar.VdRef, motorVar.VqRef, motorVar.thetaECount, &(motorVar.VaRef), &(motorVar.VbRef));
      trans_ab_to_uvw(motorVar.VaRef, motorVar.VbRef, &(motorVar.VuRef), &(motorVar.VvRef), &(motorVar.VwRef));

      // 三相電圧出力
      set_pwm_duty(motorVar.VuRef, motorVar.VvRef, motorVar.VwRef);
			break;
    
    case stateError:
      disable_pwm_out();
			break;

    default:
			break;
  }

  dacOut[0] = (int16_t)(motorVar.omegaE);
  //dacOut[0] = (int16_t)(-2048);
  dacOut[1] = (int16_t)(-2048);
  set_dac_out(dacOut);

  bufferIn.DATA.Position = motorVar.thetaECount;
  bufferIn.DATA.Velocity = motorVar.omegaE;
  bufferIn.DATA.Current = motorVar.Id;
  

  //R_PORT6->PODR_b.PODR1 = 0;    // 演算時間測定用*/
}


/********************************************************************/
/* Description: ASRスキャン割り込み処理                               */
/* Function: asr_scan                                               */
/* Arguments: なし                                                  */
/* Return value: なし                                               */
/********************************************************************/
void asr_scan(void)
{
  get_enc_pos_speed();  // 位置検出・速度演算

  switch ( currentState ) {
    case stateStop:
      // Nothing to do
			break;

    case stateRun:
      // 速度制御
			calc_spd_control(motorVar.omegaERef, motorVar.omegaE, &(motorVar.IqRef));
			break;
    
    case stateError:
      disable_pwm_out();
			break;

    default:
			break;
  }

  ecatSts = cyclic_proc_ecat();   // プロセスデータRAM読み出し&書き込み
}


/********************************************************************/
/* Description: シーケンススキャン割り込み処理                         */
/* Function: sequence_scan                                          */
/* Arguments: なし                                                  */
/* Return value: なし                                               */
/********************************************************************/
void sequence_scan(void)
{
  get_opr_status(&oprSts);
  currentState = update_state(currentState, &oprSts);
  set_opr_status(&oprSts);

  switch ( currentState ) {
    case stateStop:
      disable_pwm_out();
      calc_cur_offset();    // ゲートブロック中は都度オフセット値を計算
      bufferIn.DATA.State = pramBuffer_in_t::SlaveToMaster::StateKind::Stop;
			break;

    case stateRun:
      enable_pwm_out();
      bufferIn.DATA.State = pramBuffer_in_t::SlaveToMaster::StateKind::Run;
			break;
    
    case stateError:
      // エラー処理諸々
      disable_pwm_out();

      get_opr_status(&oprSts);
      if ( oprSts.invRst == 1 ) {
        // Z相パルス位置以外のモータ状態変数を初期化
        uint16_t thetaEIniBukp = motorVar.thetaEIni;
        motorVar = {0};
        motorVar.thetaEIni = thetaEIniBukp;

        R_GPT_POEG0->POEGG_b.PIDF = 0;    // 出力禁止要求フラグのクリア
        R_GPT_POEG1->POEGG_b.PIDF = 0;
        currentState = stateStop;         // 停止状態に復帰
      }

      bufferIn.DATA.State = pramBuffer_in_t::SlaveToMaster::StateKind::Error;
			break;

    default:

      bufferIn.DATA.State = pramBuffer_in_t::SlaveToMaster::StateKind::None;
			break;
  }
}