/********************************************************************/
/* Description: EasyCATアクセス関数の設定 (LAN9252使用)               */
/* File: easyCat.h                                                  */
/* Date: 2024/09/23                                                 */
/* Author: Takashi YOSHIOKA                                         */
/********************************************************************/


/********************************************************************/
/* 多重インクルード防止                                               */
/********************************************************************/
#ifndef _EASYCAT_H_
#define _EASYCAT_H_


/********************************************************************/
/* ヘッダファイルのインクルード                                       */
/********************************************************************/
#include <Arduino.h>


/********************************************************************/
/* 同期方式列挙型の定義                 		                          */
/********************************************************************/
typedef enum {
  ASYNC,
  DC_SYNC,
  SM_SYNC
}syncMode_t;


/********************************************************************/
/* 各種定数定義                                                      */
/********************************************************************/
// LAN9252レジスタアドレス
static constexpr uint16_t ID_REV    = 0x0050;             // チップID・リビジョンのアドレス
static constexpr uint16_t BYTE_TEST = 0x0064;             // バイトオーダテストレジスタのアドレス
static constexpr uint16_t HW_CFG    = 0x0074;             // ハードウェアコンフィグレーションレジスタのアドレス
static constexpr uint16_t RESET_CTL = 0x01F8;             // リセット制御レジスタのアドレス

static constexpr uint16_t IRQ_CFG   = 0x0054;             // 割り込み設定レジスタのアドレス
static constexpr uint16_t INT_EN    = 0x005C;             // 割り込み許可レジスタのアドレス

static constexpr uint16_t ECAT_CSR_DATA = 0x0300;         // EtherCAT CSRインターフェースデータレジスタのアドレス
static constexpr uint16_t ECAT_CSR_CMD  = 0x0304;         // EtherCAT CSRインターフェースコマンドレジスタのアドレス

static constexpr uint16_t ECAT_PRAM_RD_DATA     = 0x0000; // EtherCAT PRAM読み出しFIFOのアドレス
static constexpr uint16_t ECAT_PRAM_RD_ADDR_LEN = 0x0308; // EtherCAT PRAM読み出しアドレスおよび長さレジスタのアドレス
static constexpr uint16_t ECAT_PRAM_RD_CMD      = 0x030C; // EtherCAT PRAM読み出しコマンドレジスタのアドレス

static constexpr uint16_t ECAT_PRAM_WR_DATA     = 0x0020; // EtherCAT PRAM書き込みFIFOのアドレス
static constexpr uint16_t ECAT_PRAM_WR_ADDR_LEN = 0x0310; // EtherCAT PRAM書き込みアドレスおよび長さレジスタのアドレス
static constexpr uint16_t ECAT_PRAM_WR_CMD      = 0x0314; // EtherCAT PRAM書き込みコマンドレジスタのアドレス

// LAN9252レジスタ制御値
static constexpr uint32_t DIGITAL_RST   = 0x00000001; // RESET_CTL[0]=1:LAN9252デジタルリセット
static constexpr uint32_t BYTE_TEST_VAL = 0x87654321; // BYTE_TEST[31-0]=0x87654321

static constexpr uint8_t  RESET_CTL_MSK = 0x01;       // RESET_CTL[0]:デジタルリセットのマスクbit
static constexpr uint8_t  READY_MSK     = 0x08;       // HW_CFG[27]:READYのマスクbit

static constexpr uint8_t  ECAT_CSR_WRITE     = 0x80;  // ECAT_CSR_CMDの書き込みコマンド
static constexpr uint8_t  ECAT_CSR_READ      = 0xC0;  // ECAT_CSR_CMDの読み出しコマンド
static constexpr uint8_t  ECAT_CSR_BUSY_MSK  = 0x80;  // ECAT_CSR_CMD:CSR_BUSYのマスクbit

static constexpr uint32_t PRAM_ABORT    = 0x40000000; // ECAT_PRAM_xx_CMD[30]:PRAMリセット
static constexpr uint32_t PRAM_CMD_BUSY = 0x80000000; // ECAT_PRAM_xx_CMD[31]:PRAM FIFO動作開始/動作中
static constexpr uint8_t  PRAM_BUSY_MSK = 0x80;       // ECAT_PRAM_xx_CMD:PRAM_BUSYのマスクbit

// EtherCAT CSRレジスタアドレス
static constexpr uint16_t AL_STATUS     = 0x0130;     // ALステータスレジスタのアドレス
static constexpr uint16_t AL_EVENT_MASK = 0x0204;     // ALイベントマスクレジスタのアドレス
static constexpr uint16_t WDOG_STATUS   = 0x0440;     // ウォッチドッグステータスレジスタのアドレス

// プロセスデータRAMのオフセットアドレス
static constexpr uint16_t PRAM_BUFFER_OUT = 0x1000;   // プロセスデータ出力側のオフセットアドレス
static constexpr uint16_t PRAM_BUFFER_IN  = 0x1200;   // プロセスデータ入力側のオフセットアドレス

static constexpr uint8_t  BUFFER_SIZE = 32;           // プロセスデータRAMのサイズ

// EtherCAT CSR制御値
static constexpr uint8_t  WDOG_STATUS_MSK = 0x01;     // WDOG_STATUS[0]:プロセスデータのウォッチドッグステータス

static constexpr uint8_t  ESM_INIT   = 0x01;          // EtherCATステートマシン init
static constexpr uint8_t  ESM_PREOP  = 0x02;          // pre-operational
static constexpr uint8_t  ESM_BOOT   = 0x03;          // 
static constexpr uint8_t  ESM_SAFEOP = 0x04;          // safe-operational
static constexpr uint8_t  ESM_OP     = 0x08;          // operational

// 初期化時のタイムアウトカウント
static constexpr uint32_t CNT_INIT_TIMEOUT = 1000;    // カウント値は適当


/********************************************************************/
/* PRAMバッファ構造体の定義             		                          */
/********************************************************************/
typedef union {
  uint32_t LONG[BUFFER_SIZE/4];
  uint8_t BYTE[BUFFER_SIZE];
  
  struct MasterToSlave
  {
      float CurrentRef;
      bool ServoOn : 1;
      bool ResetError : 1;
  } __attribute__((packed)) DATA;
}pramBuffer_out_t;

typedef union {
  uint32_t LONG[BUFFER_SIZE/4];
  uint8_t BYTE[BUFFER_SIZE];
  
  struct SlaveToMaster
  {
      int32_t Position;
      int32_t Velocity;
      int32_t Current;

      enum class StateKind
      {
        None,
        Run,
        Stop,
        Error,
      } State : 2;
  } __attribute__((packed)) DATA;
}pramBuffer_in_t;


/********************************************************************/
/* 外部公開関数のプロトタイプ宣言                                      */
/********************************************************************/
bool setup_ecat(syncMode_t sync);
uint8_t cyclic_proc_ecat(void);

void proc_led_indicator(uint8_t statusArg);

void write_ecat_csr_indirect(uint16_t addrArg, uint32_t wDataArg, uint8_t lenArg);
uint32_t read_ecat_csr_indirect(uint16_t addrArg, uint8_t lenArg);

void write_ecat_pram(uint16_t addrArg, uint32_t *wDataArg, uint8_t lenArg);
void read_ecat_pram(uint16_t addrArg, uint32_t *rDataArg, uint8_t lenArg);


#endif /* END _EASYCAT_H_ */