/********************************************************************/
/* Description: EasyCATアクセス関数の設定 (LAN9252使用)               */
/* File: easyCat.cpp                                                */
/* Date: 2024/09/23                                                 */
/* Author: Takashi YOSHIOKA                                         */
/********************************************************************/


/********************************************************************/
/* ヘッダファイルのインクルード                                        */
/********************************************************************/
#include "easyCat.h"
#include "qspi.h"


/********************************************************************/
/* グローバル構造体の実体定義                                          */
/********************************************************************/
pramBuffer_out_t bufferOut;
pramBuffer_in_t  bufferIn;


/********************************************************************/
/* Description: EasyCATの初期化 (LAN9252の初期化)                    */
/* Function: setup_ecat                                             */
/* Arguments: syncMode_t列挙型 同期方式選択                           */
/* Return value: bool型 初期化の成否を返す                            */
/********************************************************************/
bool setup_ecat(syncMode_t sync)
{
  packedLong_t readReg;
  uint32_t timeOutCnt;

  // LAN9252のリセット処理
  write_qspi_direct(RESET_CTL, DIGITAL_RST);    // リセットレジスタに値をセット

  timeOutCnt = CNT_INIT_TIMEOUT;                // リセット待ちの待機カウント
  do {
    timeOutCnt--;
    readReg.LONG = read_qspi_direct(RESET_CTL); // リセット処理が終了し該当bitがクリアされるまで待機
  } while ( ( ( readReg.BYTE[0] & RESET_CTL_MSK ) != 0x00 ) && ( timeOutCnt > 0 ) );

  if ( timeOutCnt == 0 ) {
    return false;
  }
  else {
    // Do nothing (以降の処理継続)
  }

  // LAN9252のバイトオーダ確認
  timeOutCnt = CNT_INIT_TIMEOUT;                // バイトオーダ確認の待機カウント
  do {
    timeOutCnt--;
    readReg.LONG = read_qspi_direct(BYTE_TEST); // バイトオーダ確認
  } while ( ( readReg.LONG != BYTE_TEST_VAL ) && ( timeOutCnt > 0 ) );

  if ( timeOutCnt == 0 ) {                      
    return false;
  }
  else {
    // Do nothing (以降の処理継続)
  }

  // LAN9252のREADY確認
  timeOutCnt = CNT_INIT_TIMEOUT;                // READY待ちの待機カウント
  do {
    timeOutCnt--;
    readReg.LONG = read_qspi_direct(HW_CFG);    // リセット処理が終了し該当bitがクリアされるまで待機
  } while ( ( ( readReg.BYTE[3] & READY_MSK ) == 0x00 ) && ( timeOutCnt > 0 ) );

  if ( timeOutCnt == 0 ) {
    return false;
  }
  else {
    // Do nothing (以降の処理継続)
  }

  // チップID・リビジョンの表示
  readReg.LONG = read_qspi_direct(ID_REV);
  Serial.print("Detected chip ");
  Serial.print(readReg.WORD[1], HEX);
  Serial.print("  Rev ");
  Serial.println (readReg.WORD[0]);

  // 同期方式の選択
  switch ( sync ) {
    // 非同期
    case ASYNC:
      // Nothing to do
      Serial.println(F("Sync = ASYNC"));
      break;
    
    // DC同期
    case DC_SYNC:
      write_ecat_csr_indirect(AL_EVENT_MASK, 0x00000004, 4);  // SYNC0割り込みのみアンマスク
      write_qspi_direct(IRQ_CFG, 0x00000111);   // IRQ端子有効，IRQ Active High，IRQ プッシュプル
      write_qspi_direct(INT_EN, 0x00000001);    // 割り込み許可
      Serial.println(F("Sync = DC_SYNC"));
      break;

    // SM同期
    case SM_SYNC:
      write_ecat_csr_indirect(AL_EVENT_MASK, 0x00000100, 4);  // SM0割り込み(ECAT書き込み=BufferOut読み出し)のみアンマスク
      write_qspi_direct(IRQ_CFG, 0x00000111);   // IRQ端子有効，IRQ Active High，IRQ プッシュプル
      write_qspi_direct(INT_EN, 0x00000001);    // 割り込み許可
      Serial.println(F("Sync = SM_SYNC"));
      break;

    default:
      break;
  }
  
  // 初期化完了
  return true;
}


/********************************************************************/
/* Description: EasyCATの繰り返し処理                                */
/* Function: cyclic_proc_ecat                                       */
/* Arguments: なし                                                   */
/* Return value: uint8_t型 ステートマシンの状態を返す                  */
/********************************************************************/
uint8_t cyclic_proc_ecat(void)
{
  bool watchdogTout = false;
  bool operational = false;
  uint8_t status;
  packedLong_t readReg;

  readReg.LONG = read_ecat_csr_indirect(WDOG_STATUS, 4);// ウォッチドッグステータスのチェック
  if ( ( readReg.BYTE[0] & WDOG_STATUS_MSK ) == 0x01 ) {
    watchdogTout = false;
  }
  else {
    watchdogTout = true;
  }

  readReg.LONG = read_ecat_csr_indirect(AL_STATUS, 4);  // EtherCATステートマシンのチェック
  status = readReg.BYTE[0] & 0x0F;
  if ( status == ESM_OP ) {
    operational = true;
  }
  else {
    operational = false;
  }

  if ( watchdogTout || !operational ) {
    bufferOut = {0};
  }
  else {
    read_ecat_pram(PRAM_BUFFER_OUT, bufferOut.LONG, BUFFER_SIZE); // operational状態のときのみPRAM(bufferOut)を読み出し
  }

  write_ecat_pram(PRAM_BUFFER_IN, bufferIn.LONG, BUFFER_SIZE);    // PRAM(bufferIn)は常時反映

  if ( watchdogTout ) {
    status |= 0x80;         // statusのMSBにウォッチドッグステータスを付加
  }

  return status;
}


/********************************************************************/
/* Description: LEDインジケータ処理                   　              */
/* Function: proc_led_indicator                                     */
/* Arguments: status - エラー・ステートマシンステータス                */
/* Return value: なし                                                */
/********************************************************************/
void proc_led_indicator(uint8_t statusArg) {
  uint8_t errLedVal = 1;
  uint8_t stsLedVal = 1;
  static uint8_t cnt = 0;

  switch ( ( statusArg & 0x0F ) ) {
    case ESM_INIT:      // initial state
      stsLedVal = 1;    // ステートLED OFF
      break;
    
    case ESM_PREOP:     // pre-operational
      if ( cnt % 2 == 0 ) {
        stsLedVal = 0;  // ブリンキング
      }
      else {
        stsLedVal = 1;
      }
      break;
    
    case ESM_SAFEOP:    // safe-operational
      if ( cnt % 6 == 0 ) {
        stsLedVal = 0;  // シングルフラッシュ
      }
      else {
        stsLedVal = 1;
      }
      break;
     
    case ESM_OP:        // operational
      stsLedVal = 0;    // ステートLED ON
      break;
    
    default:
      break;
  }
  digitalWrite(LEDG, stsLedVal);
  
  /*switch ( ( statusArg & 0x80 ) ) {
    case 0x00:          // エラー無
      stsLedVal = 1;    // エラーLED OFF
      break;
    
    case 0x80:          // ウォッチドッグタイムアウト
      if ( ( cnt % 8 == 0 ) || ( cnt % 8 == 2 ) ) {
        errLedVal = 0;  // ダブルフラッシュ
      }
      else {
        errLedVal = 1;
      }
      break;
    
    default:
      break;
  }
  digitalWrite(LEDR, errLedVal);*/

  if ( cnt < 48 ) {
    cnt++;
  }
  else {
    cnt = 0;
  }
}


/********************************************************************/
/* Description: EtherCAT CSRレジスタ間接書き込み     　　             */
/* Function: write_ecat_csr_indirect                                */
/* Arguments: addrArg - レジスタアドレス                             */
/*            wDataArg - 書き込みデータ                              */
/*            lenArg - データ長                                     */
/* Return value: なし                                               */
/********************************************************************/
void write_ecat_csr_indirect(uint16_t addrArg, uint32_t wDataArg, uint8_t lenArg)
{
  packedLong_t packedData;
  packedLong_t readReg;

  write_qspi_direct(ECAT_CSR_DATA, wDataArg);       // 書き込みデータをデータレジスタにセット
  
  packedData.WORD[0] = addrArg;
  packedData.BYTE[2] = lenArg;
  packedData.BYTE[3] = ECAT_CSR_WRITE;
  write_qspi_direct(ECAT_CSR_CMD, packedData.LONG); // アドレス，データ長，R/Wをコマンドレジスタにセット

  do {
    readReg.LONG = read_qspi_direct(ECAT_CSR_CMD);  // 書き込み動作が完了するまで待機
  } while ( ( readReg.BYTE[3] & ECAT_CSR_BUSY_MSK ) != 0x00 );
}


/********************************************************************/
/* Description: EtherCAT CSRレジスタ間接読み出し     　　             */
/* Function: read_ecat_csr_indirect                                 */
/* Arguments: addrArg - レジスタアドレス                             */
/*            lenArg - データ長                                     */
/* Return value: 読み出しデータ                                      */
/********************************************************************/
uint32_t read_ecat_csr_indirect(uint16_t addrArg, uint8_t lenArg)
{
  packedLong_t packedData;
  packedLong_t readReg;
  
  packedData.WORD[0] = addrArg;
  packedData.BYTE[2] = lenArg;
  packedData.BYTE[3] = ECAT_CSR_READ;
  write_qspi_direct(ECAT_CSR_CMD, packedData.LONG); // アドレス，データ長，R/Wをコマンドレジスタにセット

  do {
    readReg.LONG = read_qspi_direct(ECAT_CSR_CMD);  // 読み出し動作が完了するまで待機
  } while ( ( readReg.BYTE[3] & ECAT_CSR_BUSY_MSK ) != 0x00 );

  readReg.LONG = read_qspi_direct(ECAT_CSR_DATA);   // 読み出しデータをデータレジスタから読み出し
  return readReg.LONG;
}


/********************************************************************/
/* Description: EtherCAT PRAM FIFO経由書き込み      　　             */
/* Function: write_ecat_pram                                        */
/* Arguments: addrArg - PRAM書き込み開始アドレス                      */
/*            wDataArg - 書き込みデータの配列ポインタ                 */
/*            lenArg - データ長                                     */
/* Return value: なし                                               */
/********************************************************************/
void write_ecat_pram(uint16_t addrArg, uint32_t *wDataArg, uint8_t lenArg)
{
  packedLong_t packedData;
  packedLong_t readReg;

  write_qspi_direct(ECAT_PRAM_WR_CMD, PRAM_ABORT);    // 書き込みFIFOリセット
  do {
    readReg.LONG = read_qspi_direct(ECAT_PRAM_WR_CMD);// 書き込み可能となるまで待機
  } while ( ( readReg.BYTE[3] & PRAM_BUSY_MSK ) != 0x00 );

  packedData.WORD[0] = addrArg;
  packedData.BYTE[2] = lenArg;
  packedData.BYTE[3] = 0x00;
  write_qspi_direct(ECAT_PRAM_WR_ADDR_LEN, packedData.LONG);  // アドレス，データ長をレジスタにセット

  write_qspi_direct(ECAT_PRAM_WR_CMD, PRAM_CMD_BUSY); // FIFO動作スタート

  do {
    readReg.LONG = read_qspi_direct(ECAT_PRAM_WR_CMD);// FIFOが空くまで待機
  } while ( readReg.BYTE[1] < lenArg/4 );

  write_qspi_direct_mul(ECAT_PRAM_WR_DATA, wDataArg, lenArg); // FIFO経由でPRAMに書き込み
}


/********************************************************************/
/* Description: EtherCAT PRAM FIFO経由読み出し      　　             */
/* Function: read_ecat_pram                                         */
/* Arguments: addrArg - PRAM読み出し開始アドレス                      */
/*            rDataArg - 読み出しデータの配列ポインタ                  */
/*            lenArg - データ長                                      */
/* Return value: なし                                                */
/********************************************************************/
void read_ecat_pram(uint16_t addrArg, uint32_t *rDataArg, uint8_t lenArg)
{
  packedLong_t packedData;
  packedLong_t readReg;

  write_qspi_direct(ECAT_PRAM_RD_CMD, PRAM_ABORT);    // 読み出しFIFOリセット
  do {
    readReg.LONG = read_qspi_direct(ECAT_PRAM_RD_CMD);// 読み出し可能となるまで待機
  } while ( ( readReg.BYTE[3] & PRAM_BUSY_MSK ) != 0x00 );

  packedData.WORD[0] = addrArg;
  packedData.BYTE[2] = lenArg;
  packedData.BYTE[3] = 0x00;
  write_qspi_direct(ECAT_PRAM_RD_ADDR_LEN, packedData.LONG);  // アドレス，データ長をレジスタにセット

  write_qspi_direct(ECAT_PRAM_RD_CMD, PRAM_CMD_BUSY); // FIFO動作スタート

  do {
    readReg.LONG = read_qspi_direct(ECAT_PRAM_RD_CMD);// FIFOにデータが貯まるまで待機
  } while ( readReg.BYTE[1] < lenArg/4 );

  read_qspi_direct_mul(ECAT_PRAM_RD_DATA, rDataArg, lenArg);  // FIFO経由でPRAMから読み出し
}