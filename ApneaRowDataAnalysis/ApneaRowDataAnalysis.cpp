// ApneaRowDataAnalysis.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include	"stdafx.h"
#include	<stdio.h>
#include	<stdlib.h>
#include	<math.h>
#include	<time.h>
#include	<direct.h>
#include	<string.h>
#include	"getwav.h"
#include	"apnea_param.h"

/*==============================================================================*/
/*	define定義																	*/
/*==============================================================================*/
#define DATA_SIZE				200	// 10秒間、50msに1回データ取得した数
#define BUF_SIZE				256
#define RIREKI					3
#define CalcDataNumApnea		200
#define LOOP_CNT				65536

/*==============================================================================*/
/*	グローバル変数																*/
/*==============================================================================*/
double	dc_[DATA_SIZE];
double	movave_[DATA_SIZE];
double	ave_[DATA_SIZE];
double	eval_[DATA_SIZE];
double	rms_[DATA_SIZE];
double	point_[DATA_SIZE];

char    path_[BUF_SIZE] = "C:/ax/apnea/";
char	tempPath_[BUF_SIZE] = { '\0' };

int		len = CalcDataNumApnea;
int		apnea_ = APNEA_NONE;	// 呼吸状態

/*==============================================================================*/
/*	プロトタイプ宣言															*/
/*==============================================================================*/
void	getwav_apnea(const double* pData, int DSize, int Param1, double Param2, double Param3, double Param4);
void	debug_out(char *f, const double d[], int size, const char* ppath);
void	debug_out_int(char *f, const int d[], int size, const char* ppath);

/*==============================================================================*/
/*	main																		*/
/*==============================================================================*/
int main()
{
	struct tm timeptr;
	FILE *fp;
	char folder[BUF_SIZE] = { '\0' };
	char dataPath[BUF_SIZE] = { '\0' };
	char str[BUF_SIZE] = { '\0' };
	time_t timer = 0;
	int i, ii = 0;

	//現在時刻取得
	timer = time(NULL);
	if (localtime_s(&timeptr, &timer))
	{
		return 1;
	}

	//現在時刻でフォルダ作成
	strftime(folder, BUF_SIZE, "%Y%m%d%H%M%S", &timeptr);
	if (!strcat_s(path_, sizeof path_, folder)) {
		_mkdir(path_);
	}

	//データフォルダあるだけループ
	for (ii = 0; ii < LOOP_CNT; ii++)
	{
		strcpy_s(dataPath, sizeof dataPath, ".");
		sprintf_s(str, BUF_SIZE, "/%d/", ii);
		strcat_s(dataPath, sizeof dataPath, str);
		strcat_s(dataPath, sizeof dataPath, "movave.txt");

		if (fopen_s(&fp, dataPath, "r")) {
			printf("ファイルを開くことが出来ませんでした。\n");
			break;
		}

		for (i = 0; i < len; i++) {
			fscanf_s(fp, "%lf", &(movave_[i]));     /*  1行読む  */
		}
		double* ptest1 = (double*)calloc(len, sizeof(double));
		for (i = 0; i < len; ++i) {
			ptest1[i] = movave_[i] / APNEA_PARAM_RAW;
		}

		strcpy_s(tempPath_, sizeof tempPath_, path_);
		strcat_s(tempPath_, sizeof tempPath_, str);
		_mkdir(tempPath_);

		getwav_apnea(ptest1, len, APNEA_PARAM_AVE_CNT, APNEA_PARAM_AVE_THRE, APNEA_PARAM_BIN_THRE, APNEA_PARAM_APNEA_THRE);

		fclose(fp);
	}
}

/************************************************************************/
/* 関数     : getwav_apnea												*/
/* 関数名   : 無呼吸演算処理											*/
/* 引数     : なし														*/
/* 戻り値   : なし														*/
/* 変更履歴 : 2017.07.12 Axia Soft Design mmura	初版作成				*/
/************************************************************************/
/* 機能 :																*/
/************************************************************************/
/* 注意事項 :															*/
/* なし																	*/
/************************************************************************/
void getwav_apnea(const double* pData, int DSize, int Param1, double Param2, double Param3, double Param4)
{
	// (35) = Param1
	// (36) = Param2
	// (40) = Param3

	// (37)
	for (int ii = 0; ii < DSize; ++ii) {
		int min = 0;
		int loop = 0;
		if (ii <= DSize - 1) {
			int tmp = DSize - 1 - ii;
			if (tmp > Param1) {
				min = Param1;
			}
			else {
				min = tmp;
			}
			loop = min * 2 + 1;
			min = ii - min;
			if (min < 0) {
				min = 0;
			}
		}
		else {
			min = 0;
			loop = ii * 2 + 1;
		}
		double ave = 0.0f;
		for (int jj = 0; jj < loop; ++jj) {
			ave += pData[min + jj];
		}
		ave /= loop;
		ave_[ii] = ave;
	}
	debug_out("ave", ave_, DSize, tempPath_);

	// (38) - (39)
	for (int ii = 0; ii < DSize; ++ii) {
		if (ave_[ii] >= Param2) {
			eval_[ii] = 1;
		}
		else {
			eval_[ii] = 0;
		}
	}
	debug_out("eval2", eval_, DSize, tempPath_);

	// (41) ... 使用していないため省略
	// (42)
	// (43) = prms
	int datasize = DSize / 20;
	memset(rms_, 0x00, DSize);
	for (int ii = 0; ii < datasize; ++ii) {
		rms_[ii] = 0.0f;
		double tmp = 0.0f;
		for (int jj = 0; jj < 20; ++jj) {
			tmp += (pData[ii * 20 + jj] * pData[ii * 20 + jj]);
		}
		tmp /= 20;
		rms_[ii] = sqrt(tmp);
	}
	debug_out("RMS", rms_, datasize, tempPath_);

	// (44) = ppoint
	// (45) = rms_
	memset(point_, 0x00, DSize);
	for (int ii = 0; ii < datasize; ++ii) {
		if (rms_[ii] >= Param3) {
			point_[ii] = 1;
		}
		else {
			point_[ii] = 0;
		}
	}
	debug_out("point", point_, datasize, tempPath_);

	// (46)
	if (datasize == 0) {
		apnea_ = APNEA_NORMAL;
	}
	else if (datasize > 9) {
		apnea_ = APNEA_WARN;
		int loop = datasize - 9;
		for (int ii = 0; ii < loop; ++ii) {
			double apnea = 0;
			for (int jj = 0; jj < 9; ++jj) {
				apnea += point_[ii + jj];
			}
			if (apnea != 0) {
				apnea_ = APNEA_NORMAL;
			}
		}
	}
	else {
		apnea_ = APNEA_NORMAL;
	}

	// 完全無呼吸の判定
	if (apnea_ == APNEA_WARN) {
		apnea_ = APNEA_ERROR;
		for (int ii = 0; ii < datasize; ++ii) {
			if (pData[ii] > Param4) {
				apnea_ = APNEA_WARN;
				break;
			}
		}
	}

	double tmpapnea = (double)apnea_;
	debug_out("apnea", &tmpapnea, 1, tempPath_);
}

/*==============================================================================*/
/*	debug_out																	*/
/*==============================================================================*/
void	debug_out(char *f, const double d[], int size, const char* ppath)
{
	FILE		*fp;
	char		b[1024];
	errno_t error;

	sprintf_s(b, 1024, "%s\\%s.txt", ppath, f);

	error = fopen_s(&fp, b, "w");
	if (error != 0)
	{
		printf("file open error [debug_out]\n");
		exit(0);
	}

	for (int i = 0; i < size; i++)
	{
		fprintf(fp, "%lf\n", d[i]);
	}
	fclose(fp);
}

/*==============================================================================*/
/*	debug_out_int																*/
/*==============================================================================*/
void	debug_out_int(char *f, const int d[], int size, const char* ppath)
{
	FILE		*fp;
	char		b[1024];

	sprintf_s(b, sizeof(b), "%s\\%s.txt", ppath, f);


	if (fopen_s(&fp, b, "w"))
	{
		printf("file open error [debug_out]\n");
		exit(0);
	}

	for (int i = 0; i < size; i++)
	{
		fprintf(fp, "%d\n", d[i]);
	}
	fclose(fp);
}
/* EOF */
