/************************************************************************/
/* システム名   : 無呼吸判定											*/
/* ファイル名   : getwav.h												*/
/* 機能         : inputファイルの読み込み								*/
/* 変更履歴     : 2017.07.12 Axia Soft Design mmura	初版作成			*/
/* 注意事項     : なし                                                  */
/************************************************************************/

#ifndef		_GETWAV_H_			/* 二重定義防止 */
#define		_GETWAV_H_

#include "sys.h"


/************************************************************/
/* マクロ													*/
/************************************************************/
// 無呼吸判定結果
#define APNEA_NORMAL	0	// 異常なし
#define APNEA_WARN		1	// 無呼吸区間あり
#define APNEA_ERROR		2	// 完全無呼吸
#define APNEA_NONE		3	// 判定エラー

#endif

/************************************************************/
/* END OF TEXT												*/
/************************************************************/

