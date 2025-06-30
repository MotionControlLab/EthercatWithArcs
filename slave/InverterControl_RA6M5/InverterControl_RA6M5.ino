/********************************************************************/
/* Project: 三相インバータ制御プログラム                               */
/*          MCU:RA6M5                                               */
/*          MCUボード:Portenta C33                                   */
/*          インバータ:BOOSTXL-3PHGANINV                             */
/*          FCLK=50MHz, ICLK=200MHz,                                */
/*          PCLKA=100MHz, PCLKB=50MHz, PCLKC=50MHz, PCLKD=100MHz    */
/********************************************************************/
/********************************************************************/
/* Description: インバータ制御 メインプログラム                        */
/* File: InverterControl_RA6M5.ino                                  */
/* Date: 2024/10/07                                                 */
/* Author: Takashi YOSHIOKA                                         */
/********************************************************************/


/********************************************************************/
/* ヘッダファイルのインクルード                                        */
/********************************************************************/
#include "pwm.h"
#include "operator.h"
#include "sequenceTimer.h"
#include "asrTimer.h"
#include "dacMonitor.h"
#include "encCounter.h"
#include "spiAdc.h"
#include "qspi.h"
#include "easyCat.h"


/********************************************************************/
/* グローバル変数の実体定義                                           */
/********************************************************************/
uint32_t Millis = 0;
uint32_t PreviousMillis = 0;


/********************************************************************/
/* 外部変数宣言                                                      */
/********************************************************************/
extern uint8_t ecatSts;
extern motorVar_t motorVar;
extern pramBuffer_out_t bufferOut;


/********************************************************************/
/* Description: 初期設定用関数 (1回だけ実行)                          */
/* Function: setup                                                  */
/* Arguments: なし                                                  */
/* Return value: なし                                               */
/********************************************************************/
void setup() {
  Serial.begin(115200);

  pinMode(LEDG, OUTPUT);      // EtherCATステートLED
  digitalWrite(LEDG, HIGH);   // OFF
  pinMode(LEDR, OUTPUT);      // EtherCATエラーLED
  digitalWrite(LEDR, HIGH);   // OFF
  
  setup_operator();
  setup_pwm_scan();
  setup_sequence_scan();
  setup_asr_scan();
  setup_dac_monitor();
  setup_enc_counter();
  setup_spi_adc();
  setup_qspi();
  if ( setup_ecat(DC_SYNC) == true ) {
    Serial.println("initialized");
  }
  else {
    Serial.println("initialization failed");
    while(1){
      digitalWrite(LEDR, LOW);
      delay(200);
      digitalWrite(LEDR, HIGH);
      delay(200);  
    }
  }

  R_PMISC->PWPR_b.B0WI = 0;
  R_PMISC->PWPR_b.PFSWE = 1;
  R_PFS->PORT[6].PIN[1].PmnPFS_b.PMR = 0;     // CALC_MON:汎用入出力
  R_PFS->PORT[6].PIN[1].PmnPFS_b.PODR = 0;    // CALC_MON:初期出力Low
  R_PFS->PORT[6].PIN[1].PmnPFS_b.PDR = 1;     // CALC_MON:出力端子
  R_PMISC->PWPR_b.PFSWE = 0;
  R_PMISC->PWPR_b.B0WI = 1;
}


/********************************************************************/
/* Description: ループ処理関数 (繰り返し実行)                         */
/* Function: loop                                                   */
/* Arguments: なし                                                  */
/* Return value: なし                                               */
/********************************************************************/
void loop() {
  Millis = millis();
  if ( ( Millis - PreviousMillis ) >= 200 ) {
    PreviousMillis = Millis;
    proc_led_indicator(ecatSts);
    Serial.print("state :");
    Serial.println(bufferOut.LONG[0]);
  }
  else {
    //Do nothing
  }
}