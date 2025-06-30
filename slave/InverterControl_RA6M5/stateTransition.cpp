/********************************************************************/
/* Description: オペレータ入力状態に基づく状態遷移の実装                */
/* File: stateTransition.cpp                                        */
/* Date: 2024/08/18                                                 */
/* Author: Takashi YOSHIOKA                                         */
/********************************************************************/


/********************************************************************/
/* ヘッダファイルのインクルード                                        */
/********************************************************************/
#include "stateTransition.h"
#include "motorVar.h"


/********************************************************************/
/* 外部変数宣言                                                      */
/********************************************************************/
extern motorVar_t motorVar;


/********************************************************************/
/* 外部非公開関数のプロトタイプ宣言                                    */
/********************************************************************/
static oprEvent_t get_event(oprSts_t *oprStsArg);


/********************************************************************/
/* 二次元配列インデックスによる状態遷移表の表現                         */
/********************************************************************/
static const invState_t stateMatrix[xStateNum][xEventNum] = {
// current State  eventNone   eventStop   eventRun    eventError
  [stateStop]  = {stateStop,	stateStop,	stateRun,   stateError},
  [stateRun]   = {stateRun,   stateStop,	stateRun,   stateError},
  [stateError] = {stateError,	stateError,	stateError, stateError},
};


/********************************************************************/
/* Contents: トリガイベントに応じたevent型の値を返す                   */
/* Function: get_event                                              */
/* Arguments: oprSts_t構造体のポインタ                                */
/* Return value: oprEvent_t型 (列挙型:トリガイベントのインデックス)     */
/********************************************************************/
oprEvent_t get_event(oprSts_t *oprStsArg)
{
	oprEvent_t event;

  if (( oprStsArg->invErr == 1 ) || ( motorVar.omegaE > ALMLEVEL_OVERSPD ) || ( motorVar.omegaE < -ALMLEVEL_OVERSPD ) ) {     // invErrがアクティブ，もしくはオーバースピードならeventErrorを返す
    event = eventError;
  }
  else {
    if ( oprStsArg->invOff == 1 ) {   // invOffがアクティブならeventStopを返す
      event = eventStop;
    }
    else {
      if ( oprStsArg->invOn == 1 ) {  // invOnがアクティブならeventRunを返す
        event = eventRun;
      }
      else {
        event = eventNone;            // トリガイベントが何もない場合はeventNoneを返す
      }
    }
  }

  return event;
}


/********************************************************************/
/* Contents: 運転状態・トリガイベントに基づいて現在の状態を更新         */
/* Function: update_state                                           */
/* Arguments: invState_t構造体のポインタ, oprSts_t構造体のポインタ     */
/* Return value: invState_t型 (列挙型:状態のインデックス)             */
/********************************************************************/
invState_t update_state(invState_t invStateArg, oprSts_t *oprStsArg)
{
	oprEvent_t event;
  invState_t state;

  event = get_event(oprStsArg);		          // トリガイベントを取得
  state = stateMatrix[invStateArg][event];  // 現在の状態とトリガイベントに応じて状態遷移

  return state;
}