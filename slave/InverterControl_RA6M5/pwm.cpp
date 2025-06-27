/********************************************************************/
/* Description: GPTモジュールおよびPWM出力周りの設定　　　　　　　　　  */
/* File: pwm.cpp                                                    */
/* Date: 2024/09/15                                                 */
/* Author: Takashi YOSHIOKA                                         */
/********************************************************************/


/********************************************************************/
/* ヘッダファイルのインクルード                                        */
/********************************************************************/
#include <IRQManager.h>
#include "pwm.h"
#include "controlFunction.h"
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
static void MYISR_GPT6_INT(void);


/********************************************************************/
/* キャリア谷スキャン割り込み関数の外部宣言                             */
/********************************************************************/
extern void carrier_valley_scan(void);


/********************************************************************/
/* Description: GPTモジュールおよびPWM出力周りの初期化                 */
/* Function: setup_pwm_scan                                         */
/* Arguments: なし                                                  */
/* Return value: なし                                               */
/********************************************************************/
void setup_pwm_scan(void)
{
  // モジュールストップ解除
  R_MSTP->MSTPCRD_b.MSTPD14 = 0;    // POEGGA
  R_MSTP->MSTPCRD_b.MSTPD13 = 0;    // POEGGB

  R_MSTP->MSTPCRE_b.MSTPE25 = 0;    // GPT6
  R_MSTP->MSTPCRE_b.MSTPE24 = 0;    // GPT7
  R_MSTP->MSTPCRE_b.MSTPE23 = 0;    // GPT8

  // POEG設定
  R_GPT_POEG0->POEGG = 0x00000018;  // GPT Gr.A出力禁止, GTETRGAによる出力禁止許可, GTETRGA=Highレベルで出力禁止
  R_GPT_POEG1->POEGG = 0x00000018;  // GPT Gr.B出力禁止, 以降同上
  
  // disable module stop (GPT)
  R_GPT6->GTIOR_b.OAE = 0;          // GTIOCA端子出力許可
  R_GPT6->GTIOR_b.OBE = 0;          // GTIOCB端子出力許可
  R_GPT6->GTIOR_b.OADF = 0b10;      // 出力禁止時0出力
  R_GPT6->GTIOR_b.OBDF = 0b10;      // 出力禁止時0出力
  R_GPT7->GTIOR_b.OAE = 0;
  R_GPT7->GTIOR_b.OBE = 0;
  R_GPT7->GTIOR_b.OADF = 0b10;
  R_GPT7->GTIOR_b.OBDF = 0b10;
  R_GPT8->GTIOR_b.OAE = 0;
  R_GPT8->GTIOR_b.OBE = 0;
  R_GPT8->GTIOR_b.OADF = 0b10;
  R_GPT8->GTIOR_b.OBDF = 0b10;
  
  // set GPT timer mode
  R_GPT6->GTCR_b.CST = 0;       // stop timer count
  R_GPT6->GTSSR_b.CSTRT = 1;    // enable software count start
  R_GPT6->GTCR_b.MD = 0b100;    // TIMER_MODE_TRIANGLE_WAVE_SYMMETRIC_PWM
  R_GPT6->GTCR_b.TPCS = 0b0000; // PCLK/1
  R_GPT7->GTCR_b.CST = 0;
  R_GPT7->GTSSR_b.CSTRT = 1;
  R_GPT7->GTCR_b.MD = 0b100;
  R_GPT7->GTCR_b.TPCS = 0b0000;
  R_GPT8->GTCR_b.CST = 0;
  R_GPT8->GTSSR_b.CSTRT = 1;
  R_GPT8->GTCR_b.MD = 0b100;
  R_GPT8->GTCR_b.TPCS = 0b0000;
  
  // set pwm freq. and timer count
  R_GPT6->GTPR_b.GTPR = CARRIER_PERIOD_CYC;
  R_GPT6->GTCNT_b.GTCNT = 0;
  R_GPT7->GTPR_b.GTPR = CARRIER_PERIOD_CYC;
  R_GPT7->GTCNT_b.GTCNT = 0;
  R_GPT8->GTPR_b.GTPR = CARRIER_PERIOD_CYC;
  R_GPT8->GTCNT_b.GTCNT = 0;
  
  // set comparematch register and buffer register
  R_GPT6->GTBER_b.CCRA = 0b01;
  R_GPT6->GTCCR_b[0].GTCCR = INIT_DUTY_CYC + DEAD_TIME_CYC/2;
  R_GPT6->GTCCR_b[2].GTCCR = INIT_DUTY_CYC + DEAD_TIME_CYC/2;
  R_GPT7->GTBER_b.CCRA = 0b01;
  R_GPT7->GTCCR_b[0].GTCCR = INIT_DUTY_CYC + DEAD_TIME_CYC/2;
  R_GPT7->GTCCR_b[2].GTCCR = INIT_DUTY_CYC + DEAD_TIME_CYC/2;
  R_GPT8->GTBER_b.CCRA = 0b01;
  R_GPT8->GTCCR_b[0].GTCCR = INIT_DUTY_CYC + DEAD_TIME_CYC/2;
  R_GPT8->GTCCR_b[2].GTCCR = INIT_DUTY_CYC + DEAD_TIME_CYC/2;
  
  // set dead-time configuration
  R_GPT6->GTDTCR_b.TDE = 1;
  R_GPT6->GTDVU_b.GTDVU = DEAD_TIME_CYC;
  R_GPT7->GTDTCR_b.TDE = 1;
  R_GPT7->GTDVU_b.GTDVU = DEAD_TIME_CYC;
  R_GPT8->GTDTCR_b.TDE = 1;
  R_GPT8->GTDVU_b.GTDVU = DEAD_TIME_CYC;
  
  // set port configuration
  R_GPT6->GTIOR_b.OAHLD = 0;        // カウント中：GTIOA設定に従う，カウント停止：OADFLT設定に従う
  R_GPT6->GTIOR_b.OADFLT = 0;       // カウント停止時0出力
  R_GPT6->GTIOR_b.OBHLD = 0;        // 
  R_GPT6->GTIOR_b.OBDFLT = 0;       // 
  R_GPT7->GTIOR_b.OAHLD = 0;        // カウント中：GTIOA設定に従う，カウント停止：OADFLT設定に従う
  R_GPT7->GTIOR_b.OADFLT = 0;       // カウント停止時0出力
  R_GPT7->GTIOR_b.OBHLD = 0;        // 
  R_GPT7->GTIOR_b.OBDFLT = 0;       // 
  R_GPT8->GTIOR_b.OAHLD = 0;        // カウント中：GTIOA設定に従う，カウント停止：OADFLT設定に従う
  R_GPT8->GTIOR_b.OADFLT = 0;       // カウント停止時0出力
  R_GPT8->GTIOR_b.OBHLD = 0;        // 
  R_GPT8->GTIOR_b.OBDFLT = 0;       // 
  
  R_GPT6->GTIOR_b.GTIOA = 0b00111;  // GTIOCA端子機能選択
  R_GPT6->GTIOR_b.GTIOB = 0b11011;  // GTIOCB端子機能選択
  R_GPT6->GTIOR_b.OAE = 1;          // GTIOCA端子出力許可
  R_GPT6->GTIOR_b.OBE = 1;          // GTIOCB端子出力許可
  R_GPT7->GTIOR_b.GTIOA = 0b00111;
  R_GPT7->GTIOR_b.GTIOB = 0b11011;
  R_GPT7->GTIOR_b.OAE = 1;
  R_GPT7->GTIOR_b.OBE = 1;
  R_GPT8->GTIOR_b.GTIOA = 0b00111;
  R_GPT8->GTIOR_b.GTIOB = 0b11011;
  R_GPT8->GTIOR_b.OAE = 1;
  R_GPT8->GTIOR_b.OBE = 1;
  
  R_PMISC->PWPR_b.B0WI = 0;
  R_PMISC->PWPR_b.PFSWE = 1;
  R_PFS->PORT[1].PIN[0].PmnPFS_b.PMR = 0;     // J2-40(P100)を汎用入出力に設定
  R_PFS->PORT[1].PIN[1].PmnPFS_b.PMR = 0;     // J2-42(P101)を汎用入出力に設定
  R_PFS->PORT[1].PIN[0].PmnPFS_b.PDR = 0;     // J2-40(P100)を入力端子として使用
  R_PFS->PORT[1].PIN[1].PmnPFS_b.PDR = 0;     // J2-42(P101)を入力端子として使用
  R_PFS->PORT[1].PIN[0].PmnPFS_b.PSEL = 0x02; // J2-40(P100)をPOEGA端子として使用
  R_PFS->PORT[1].PIN[1].PmnPFS_b.PSEL = 0x02; // J2-42(P101)をPOEGB端子として使用
  R_PFS->PORT[1].PIN[0].PmnPFS_b.PMR = 1;     // J2-40(P100)を周辺機能用の端子に設定
  R_PFS->PORT[1].PIN[1].PmnPFS_b.PMR = 1;     // J2-42(P101)を周辺機能用の端子に設定
  
  R_PFS->PORT[4].PIN[7].PmnPFS_b.PMR = 0;     // J1-44(P407)を汎用入出力に設定
  R_PFS->PORT[4].PIN[8].PmnPFS_b.PMR = 0;     // J1-46(P408)を汎用入出力に設定
  R_PFS->PORT[4].PIN[7].PmnPFS_b.PDR = 1;     // J1-44(P407)を出力端子として使用
  R_PFS->PORT[4].PIN[8].PmnPFS_b.PDR = 1;     // J1-46(P408)を出力端子として使用
  R_PFS->PORT[4].PIN[7].PmnPFS_b.PSEL = 0x03; // J1-44(P407)をGPT端子として使用
  R_PFS->PORT[4].PIN[8].PmnPFS_b.PSEL = 0x03; // J1-46(P408)をGPT端子として使用
  R_PFS->PORT[4].PIN[7].PmnPFS_b.PMR = 1;     // J1-44(P407)を周辺機能用の端子に設定
  R_PFS->PORT[4].PIN[8].PmnPFS_b.PMR = 1;     // J1-46(P408)を周辺機能用の端子に設定
  
  R_PFS->PORT[6].PIN[3].PmnPFS_b.PMR = 0;     // J1-37(P603)を汎用入出力に設定
  R_PFS->PORT[6].PIN[2].PmnPFS_b.PMR = 0;     // J1-33(P602)を汎用入出力に設定
  R_PFS->PORT[6].PIN[3].PmnPFS_b.PDR = 1;     // J1-37(P603)を出力端子として使用
  R_PFS->PORT[6].PIN[2].PmnPFS_b.PDR = 1;     // J1-33(P602)を出力端子として使用
  R_PFS->PORT[6].PIN[3].PmnPFS_b.PSEL = 0x03; // J1-37(P603)をGPT端子として使用
  R_PFS->PORT[6].PIN[2].PmnPFS_b.PSEL = 0x03; // J1-33(P602)をGPT端子として使用
  R_PFS->PORT[6].PIN[3].PmnPFS_b.PMR = 1;     // J1-37(P603)を周辺機能用の端子に設定
  R_PFS->PORT[6].PIN[2].PmnPFS_b.PMR = 1;     // J1-33(P602)を周辺機能用の端子に設定
  
  R_PFS->PORT[6].PIN[5].PmnPFS_b.PMR = 0;     // J2-64(P605)を汎用入出力に設定
  R_PFS->PORT[1].PIN[6].PmnPFS_b.PMR = 0;     // J2-61(P106)を汎用入出力に設定
  R_PFS->PORT[6].PIN[5].PmnPFS_b.PDR = 1;     // J2-64(P605)を出力端子として使用
  R_PFS->PORT[1].PIN[6].PmnPFS_b.PDR = 1;     // J2-61(P106)を出力端子として使用
  R_PFS->PORT[6].PIN[5].PmnPFS_b.PSEL = 0x03; // J2-64(P605)をGPT端子として使用
  R_PFS->PORT[1].PIN[6].PmnPFS_b.PSEL = 0x03; // J2-61(P106)をGPT端子として使用
  R_PFS->PORT[6].PIN[5].PmnPFS_b.PMR = 1;     // J2-64(P605)を周辺機能用の端子に設定
  R_PFS->PORT[1].PIN[6].PmnPFS_b.PMR = 1;     // J2-61(P106)を周辺機能用の端子に設定
  R_PMISC->PWPR_b.PFSWE = 0;
  R_PMISC->PWPR_b.B0WI = 1;

  // 割り込み設定
  cfg.irq = FSP_INVALID_VECTOR;     // 良く分からんが固定値
  cfg.ipl = 3;                      // 割り込み優先度 0(最高)-15(最低)
  cfg.event = ELC_EVENT_GPT6_COUNTER_UNDERFLOW;   // 割り込み要因としてGPT6アンダーフロー割り込みを指定
  IRQManager::getInstance().addGenericInterrupt(cfg, MYISR_GPT6_INT);   // 割り込み要因のセット
  
  // timer count start
  R_GPT_POEG0->POEGG_b.PIDF = 0;    // なぜか出力禁止要求フラグが立っているのでクリアしておく
  R_GPT_POEG1->POEGG_b.PIDF = 0;

  R_GPT6->GTSTR = 0x01C0;             // GPT6-8カウントスタート (0x 0000 0001 1100 0000 = 0x01C0)
}


/********************************************************************/
/* Description: 割り込みサービスルーチン(GPT6アンダーフロー割り込み)    */
/* Function: MYISR_GPT6_INT                                         */
/* Arguments: なし                                                  */
/* Return value: なし                                               */
/********************************************************************/
void MYISR_GPT6_INT(void)
{
  carrier_valley_scan();            // キャリア谷スキャン割り込み処理
  R_ICU->IELSR_b[cfg.irq].IR = 0;   // 割り込み要因フラグのクリア
}


/********************************************************************/
/* Description: デューティに応じたPWMパターンを出力 (三角波比較)				*/
/* Function: set_pwm_duty                                           */
/* Arguments: VuRefArg - U相電圧指令 (-VDC/2[V] ~ VDC/2[V])          */
/*            VvRefArg - V相電圧指令 (-VDC/2[V] ~ VDC/2[V])          */
/*            VwRefArg - W相電圧指令 (-VDC/2[V] ~ VDC/2[V])          */
/* Return value: なし                                               */
/********************************************************************/
void set_pwm_duty(float VuRefArg, float VvRefArg, float VwRefArg)
{
  int32_t duty_cyc_u, duty_cyc_v, duty_cyc_w;
  
	duty_cyc_u = (int32_t)(CARRIER_PERIOD_CYC*(0.5f - VuRefArg/motorVar.Vdc) + DEAD_TIME_CYC/2);	// デッドタイム補償は暫定措置　(実際には相電流の向きに依存)
	duty_cyc_v = (int32_t)(CARRIER_PERIOD_CYC*(0.5f - VvRefArg/motorVar.Vdc) + DEAD_TIME_CYC/2);
	duty_cyc_w = (int32_t)(CARRIER_PERIOD_CYC*(0.5f - VwRefArg/motorVar.Vdc) + DEAD_TIME_CYC/2);
  
  if ( duty_cyc_u > (CARRIER_PERIOD_CYC - 1) ) duty_cyc_u = CARRIER_PERIOD_CYC - 1;   // こちらはMAX値でも問題ないが下限に合わせて1だけ狭める
  if ( duty_cyc_u < 1 ) duty_cyc_u = 1;   // 0だとパルスがバグるので下限を1に
  if ( duty_cyc_v > (CARRIER_PERIOD_CYC - 1) ) duty_cyc_v = CARRIER_PERIOD_CYC - 1;
  if ( duty_cyc_v < 1 ) duty_cyc_v = 1;
  if ( duty_cyc_w > (CARRIER_PERIOD_CYC - 1) ) duty_cyc_w = CARRIER_PERIOD_CYC - 1;
  if ( duty_cyc_w < 1 ) duty_cyc_w = 1;
  
  R_GPT6->GTCCR_b[2].GTCCR = (uint16_t)(duty_cyc_u);
  R_GPT7->GTCCR_b[2].GTCCR = (uint16_t)(duty_cyc_v);
  R_GPT8->GTCCR_b[2].GTCCR = (uint16_t)(duty_cyc_w);
}


/********************************************************************/
/* Description: PWM出力の許可                                        */
/* Function: enable_pwm_out                                         */
/* Arguments: なし                                                  */
/* Return value: なし                                               */
/********************************************************************/
void enable_pwm_out(void)
{
  R_GPT_POEG0->POEGG_b.SSF = 0;     // GPTポート出力許可
  R_GPT_POEG1->POEGG_b.SSF = 0;
}


/********************************************************************/
/* Description: PWM出力の禁止                                        */
/* Function: disable_pwm_out                                        */
/* Arguments: なし                                                  */
/* Return value: なし                                               */
/********************************************************************/
void disable_pwm_out(void)
{
  R_GPT_POEG0->POEGG_b.SSF = 1;     // GPTポート出力禁止
  R_GPT_POEG1->POEGG_b.SSF = 1;
}