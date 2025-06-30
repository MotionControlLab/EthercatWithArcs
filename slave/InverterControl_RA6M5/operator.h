/********************************************************************/
/* Description: オペレータ入出力の初期設定および状態取得                */
/* File: operator.h                                                 */
/* Date: 2024/08/18                                                 */
/* Author: Takashi YOSHIOKA                                         */
/********************************************************************/


/********************************************************************/
/* 多重インクルード防止                                               */
/********************************************************************/
#ifndef	_OPERATOR_H_
#define	_OPERATOR_H_


/********************************************************************/
/* ヘッダファイルのインクルード                                        */
/********************************************************************/
#include <Arduino.h>


/********************************************************************/
/* マクロ定義                                                        */
/********************************************************************/
#define OPR_INV_ON		    (R_PFS->PORT[7].PIN[6].PmnPFS_b)	// invOnのピンアサイン: P706
#define OPR_INV_OFF		    (R_PFS->PORT[7].PIN[7].PmnPFS_b)	// invOffのピンアサイン:P707
#define OPR_INV_RST 	    (R_PFS->PORT[7].PIN[8].PmnPFS_b)  // invRstのピンアサイン:P708

#define OPR_RUN_MON	      (R_PFS->PORT[4].PIN[9].PmnPFS_b)	// runMonのピンアサイン:P409
#define OPR_RUN_MON_PODR  (R_PORT4->PODR_b.PODR9)	          // runMonのピンアサイン:P409 (ロック解除しないとPFSへの書き込みができないため，解除不要のエイリアスを設定)
#define OPR_ERR_MON       (R_PFS->PORT[5].PIN[5].PmnPFS_b)	// errMonのピンアサイン:P505
#define OPR_ERR_MON_PODR	(R_PORT5->PODR_b.PODR5)	          // errMonのピンアサイン:P505 (ロック解除しないとPFSへの書き込みができないため，解除不要のエイリアスを設定)

#define OPR_REFIN	   	    (R_PFS->PORT[0].PIN[0].PmnPFS_b)	// refInのピンアサイン: P000


/********************************************************************/
/* オペレータ入出力状態構造体の定義                                    */
/********************************************************************/
typedef struct {
	uint8_t invOn : 1;        // オペレータ入力 INV_ONフラグ  (正論理)
	uint8_t invOff : 1;       // オペレータ入力 INV_OFFフラグ (正論理)
  uint8_t	invRst : 1;		    // オペレータ入力 INV_RSTフラグ (正論理)
  uint8_t	invErr : 1;		    // インバータ入力 INV_ERRフラグ (正論理)
	uint8_t runMon : 1;		    // オペレータ出力 RUN_MONフラグ (正論理)
  uint8_t	errMon : 1;		    // オペレータ出力 ERR_MONフラグ (正論理)
	uint8_t        : 2;       // for bit alignment
	uint8_t dummyByte;        // for byte alignment
	int16_t refIn;			      // オペレータから入力されるアナログVR信号 (0~1023)
}oprSts_t;


/********************************************************************/
/* 外部公開関数のプロトタイプ宣言																		   */
/********************************************************************/
void setup_operator(void);
void get_opr_status(oprSts_t *oprStsArg);
void set_opr_status(oprSts_t *oprStsArg);


#endif	/* END _OPERATOR_H_ */