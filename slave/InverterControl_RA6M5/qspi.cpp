/********************************************************************/
/* Description: QSPIインターフェースの設定 (LAN9252用)                */
/* File: qspi.cpp                                                   */
/* Date: 2024/10/07                                                 */
/* Author: Takashi YOSHIOKA                                         */
/********************************************************************/


/********************************************************************/
/* ヘッダファイルのインクルード                                        */
/********************************************************************/
#include "qspi.h"


/********************************************************************/
/* ソース内グローバル変数の定義                                       */
/********************************************************************/
static volatile uint32_t* IOptr = (uint32_t*)QSPI_BASEADDR;


/********************************************************************/
/* Description: QSPIの初期化                                         */
/* Function: setup_qspi                                             */
/* Arguments: なし                                                  */
/* Return value: なし                                               */
/********************************************************************/
void setup_qspi(void)
{
  // モジュールストップ解除
  R_MSTP->MSTPCRB_b.MSTPB6 = 0;     // QSPI
  
  // QSPI転送モード設定
  R_QSPI->SFMSMD_b.SFMRM = 0b000;   // QSPIリードモード 0b000:標準リード
  R_QSPI->SFMSMD_b.SFMSE = 0b00;    // 0b00:SSLアサート延長無
  R_QSPI->SFMSMD_b.SFMPFE = 0;      // 0:プリフェッチ無効
  R_QSPI->SFMSMD_b.SFMMD3 = 0;      // 0:SPIモード0
  R_QSPI->SFMSMD_b.SFMCCE = 0;      // 0:標準命令コードを使用
  
  // QSPI転送プロトコル設定
  R_QSPI->SFMSPC_b.SFMSPI = 0b00;   // 0b00:標準or拡張SPI

  // QSPIクロック設定
  R_QSPI->SFMSKC_b.SFMDV = 0x02;    // QSPCKクロック 0x02:PCLKA/4=25MHz

  // QSPIアドレスモード設定
  R_QSPI->SFMSAC_b.SFMAS = 0b01;    // 0b01:2byteアドレス

  // ダミーサイクル設定
  R_QSPI->SFMSDC_b.SFMDN = 0x0;     // 0x0:ダミーサイクル標準値 (標準リードではダミーサイクルなし)
  R_QSPI->SFMSDC_b.SFMXEN = 0;      // 0:XIP禁止
  
  // QSPIインターフェース用DIOの設定
  R_PMISC->PWPR_b.B0WI = 0;
  R_PMISC->PWPR_b.PFSWE = 1;

  R_PFS->PORT[5].PIN[0].PmnPFS_b.PMR = 0;         // P500:いったん汎用入出力
  R_PFS->PORT[1].PIN[12].PmnPFS_b.PMR = 0;        // P112:いったん汎用入出力
  R_PFS->PORT[5].PIN[2].PmnPFS_b.PMR = 0;         // P502:いったん汎用入出力
  R_PFS->PORT[5].PIN[3].PmnPFS_b.PMR = 0;         // P503:いったん汎用入出力
  R_PFS->PORT[5].PIN[0].PmnPFS_b.PDR = 1;         // P500:出力端子
  R_PFS->PORT[1].PIN[12].PmnPFS_b.PDR = 1;        // P112:出力端子
  R_PFS->PORT[5].PIN[2].PmnPFS_b.PDR = 1;         // P502:出力端子
  R_PFS->PORT[5].PIN[3].PmnPFS_b.PDR = 0;         // P503:入力端子
  R_PFS->PORT[5].PIN[0].PmnPFS_b.PSEL = 0b10001;  // P500:QSPCK
  R_PFS->PORT[1].PIN[12].PmnPFS_b.PSEL = 0b10001; // P112:QSSL
  R_PFS->PORT[5].PIN[2].PmnPFS_b.PSEL = 0b10001;  // P502:QIO0
  R_PFS->PORT[5].PIN[3].PmnPFS_b.PSEL = 0b10001;  // P503:QIO1
  R_PFS->PORT[5].PIN[0].PmnPFS_b.PMR = 1;         // P500(QSPCK):周辺機能用端子
  R_PFS->PORT[1].PIN[12].PmnPFS_b.PMR = 1;        // P112(QSSL):周辺機能用端子
  R_PFS->PORT[5].PIN[2].PmnPFS_b.PMR = 1;         // P502(QIO0):周辺機能用端子
  R_PFS->PORT[5].PIN[3].PmnPFS_b.PMR = 1;         // P503(QIO1):周辺機能用端子

  R_PMISC->PWPR_b.PFSWE = 0;
  R_PMISC->PWPR_b.B0WI = 1;
}


/********************************************************************/
/* Description: QSPI経由でレジスタ直接書き込み                        */
/*              (QSPIメモリ空間へのアクセスはない)                    */
/* Function: write_qspi_direct                                      */
/* Arguments: offsAddrArg - オフセットアドレス                       */
/*                          ロングワードアクセスにつき4の倍数とすること */
/*            wDataArg - 書き込みデータ                              */
/* Return value: なし                                               */
/********************************************************************/
void write_qspi_direct(uint16_t offsAddrArg, uint32_t wDataArg)
{
  packedWord_t packedAddr;
  packedLong_t packedData;
  
  // パック共用体にキャスト
  packedAddr.WORD = (uint16_t)(offsAddrArg);
  packedData.LONG = (uint32_t)(wDataArg);
  
  // 直接通信モードで1byte毎に送信
  // アドレスとデータのバイトオーダーが逆に見えるが仕様書通り
  R_QSPI->SFMCMD = 1;       // 1:直接通信モード

  R_QSPI->SFMCOM = WRITE_CMD; // 書き込みコマンド送信
  R_QSPI->SFMCOM = packedAddr.BYTE[1];
  R_QSPI->SFMCOM = packedAddr.BYTE[0];
  R_QSPI->SFMCOM = packedData.BYTE[0];
  R_QSPI->SFMCOM = packedData.BYTE[1];
  R_QSPI->SFMCOM = packedData.BYTE[2];
  R_QSPI->SFMCOM = packedData.BYTE[3];
  R_QSPI->SFMCMD = 1;       // トランザクション終了時に何故かもう一度必要？

  R_QSPI->SFMCMD = 0;       // 0:通常モード(ROMアクセスモード)
}


/********************************************************************/
/* Description: QSPI経由でレジスタ直接読み出し                        */
/*              (QSPIメモリ空間から読み出される)                      */
/* Function: read_qspi_direct                                       */
/* Arguments: offsAddrArg - オフセットアドレス                        */
/*                          ロングワードアクセスにつき4の倍数とすること */
/* Return value: 読み出しデータ                                      */
/********************************************************************/
uint32_t read_qspi_direct(uint16_t offsAddrArg)
{
  return IOptr[offsAddrArg/4];    // バイト読み(uint8_t*)でのポインタ+4がロングワード読み(uint32_t*)での+1に相当するため，オフセットを1/4にしている
                                  // オフセット換算を避けるべく初めからuint8_t*にすると，今度は1バイトしかアクセスしてくれないためこのような処理としている
}


/********************************************************************/
/* Description: QSPI経由でレジスタ直接書き込み(マルチロングワード版)    */
/* Function: write_qspi_direct_mul                                  */
/* Arguments: offsAddrArg - オフセットアドレス                        */
/*                          ロングワードアクセスにつき4の倍数とすること */
/*            wDataArg - 書き込みデータの配列ポインタ                  */
/*            lenArg - データ長                                      */
/* Return value: なし                                               */
/********************************************************************/
void write_qspi_direct_mul(uint16_t offsAddrArg, uint32_t *wDataArg, uint8_t lenArg)
{
  packedWord_t packedAddr;
  uint8_t *wDataPtr;
  
  // パック共用体にキャスト
  packedAddr.WORD = (uint16_t)(offsAddrArg);
  
  // バイト単位のポインタにキャスト
  wDataPtr = (uint8_t*)wDataArg;
  
  // 直接通信モードで1byte毎に送信
  // アドレスとデータのバイトオーダーが逆に見えるが仕様書通り
  R_QSPI->SFMCMD = 1;       // 1:直接通信モード

  R_QSPI->SFMCOM = WRITE_CMD; // 書き込みコマンド送信
  R_QSPI->SFMCOM = packedAddr.BYTE[1];
  R_QSPI->SFMCOM = packedAddr.BYTE[0];

  for ( uint8_t i = 0; i < lenArg; i++ ) {
    R_QSPI->SFMCOM = *wDataPtr++;
  }
  R_QSPI->SFMCMD = 1;       // トランザクション終了時に何故かもう一度必要？

  R_QSPI->SFMCMD = 0;       // 0:通常モード(ROMアクセスモード)
}


/********************************************************************/
/* Description: QSPI経由でレジスタ直接読み出し(マルチロングワード版)    */
/* Function: read_qspi_direct_mul                                   */
/* Arguments: offsAddrArg - オフセットアドレス                        */
/*                          ロングワードアクセスにつき4の倍数とすること */
/*            rDataArg - 読み出しデータの配列ポインタ                  */
/*            lenArg - データ長                                      */
/* Return value: なし                                                */
/********************************************************************/
void read_qspi_direct_mul(uint16_t offsAddrArg, uint32_t *rDataArg, uint8_t lenArg)
{
  packedWord_t packedAddr;
  uint8_t *rDataPtr;
  
  // パック共用体にキャスト
  packedAddr.WORD = (uint16_t)(offsAddrArg);
  
  // バイト単位のポインタにキャスト
  rDataPtr = (uint8_t*)rDataArg;
  
  // 直接通信モードで1byte毎に送信
  // アドレスとデータのバイトオーダーが逆に見えるが仕様書通り
  R_QSPI->SFMCMD = 1;       // 1:直接通信モード

  R_QSPI->SFMCOM = READ_CMD;// 読み出しコマンド送信
  R_QSPI->SFMCOM = packedAddr.BYTE[1];
  R_QSPI->SFMCOM = packedAddr.BYTE[0];

  for ( uint8_t i = 0; i < lenArg; i++ ) {
    *rDataPtr++ = R_QSPI->SFMCOM;
  }
  R_QSPI->SFMCMD = 1;       // トランザクション終了時に何故かもう一度必要？

  R_QSPI->SFMCMD = 0;       // 0:通常モード(ROMアクセスモード)
}