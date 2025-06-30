/********************************************************************/
/* Description: QSPIインターフェースの設定 (LAN9252用)                */
/* File: qspi.h                                                     */
/* Date: 2024/09/20                                                 */
/* Author: Takashi YOSHIOKA                                         */
/********************************************************************/


/********************************************************************/
/* 多重インクルード防止                                               */
/********************************************************************/
#ifndef _QSPI_H_
#define _QSPI_H_


/********************************************************************/
/* ヘッダファイルのインクルード                                       */
/********************************************************************/
#include <Arduino.h>


/********************************************************************/
/* 各種定数定義                                                      */
/********************************************************************/
static constexpr uint8_t WRITE_CMD     = 0x02;  // 書き込みコマンド
static constexpr uint8_t READ_CMD      = 0x03;  // 読み出しコマンド

static constexpr uint32_t QSPI_BASEADDR = 0x60000000; // QSPIベースアドレス


/********************************************************************/
/* パック共用体の定義                                                */
/********************************************************************/
typedef union {
  uint16_t WORD;
  uint8_t BYTE[2];
}packedWord_t;

typedef union {
  uint32_t LONG;
  uint16_t WORD[2];
  uint8_t BYTE[4];
}packedLong_t;


/********************************************************************/
/* 外部公開関数のプロトタイプ宣言                                      */
/********************************************************************/
void setup_qspi(void);

void write_qspi_direct(uint16_t offsAddrArg, uint32_t wDataArg);
uint32_t read_qspi_direct(uint16_t offsAddrArg);

void write_qspi_direct_mul(uint16_t offsAddrArg, uint32_t *wDataArg, uint8_t lenArg);
void read_qspi_direct_mul(uint16_t offsAddrArg, uint32_t *rDataArg, uint8_t lenArg);


#endif /* END _QSPI_H_ */