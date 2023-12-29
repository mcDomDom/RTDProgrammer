// RTD2662ModeTableDump.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//
// RTD2660やRTD2556のファームウェアからモードテーブルをダンプ
// 2022/12/06 mcDomDom
//

#include "stdafx.h"

#ifndef BYTE
#define	BYTE unsigned char
#endif
#ifndef WORD
#define	WORD unsigned short
#endif

#ifndef WIN32
#define	stricmp strcasecmp
#endif

#pragma pack(push, 1) 
	struct T_Info {
		BYTE	polarity;	// 極性フラグ 0x20:SDTV? 0x40:HDTV?  0x10:Interlace?
		WORD	width;
		WORD	height;
		WORD	hfreq;
		WORD	vfreq;
		BYTE	htolerance;
		BYTE	vtolerance;
		WORD	htotal;
		WORD	vtotal;
		WORD	hstart;
		WORD	vstart;
	};

	// RTD2668
	struct T_Info_23 {
		BYTE	no;
		BYTE	type;
		BYTE	polarity;	// 極性フラグ 0x20:SDTV? 0x40:HDTV?  0x10:Interlace?
		WORD	width;
		WORD	height;
		WORD	hfreq;
		WORD	vfreq;
		BYTE	htolerance;
		BYTE	vtolerance;
		WORD	htotal;
		WORD	vtotal;
		WORD	hstart;
		WORD	vstart;
		WORD	vcount;
	};
#pragma pack(pop) 

enum enModel
{
	UNKNOWN = 0,
	P2314H,				// DELL P2314H
	LHRD56_IPAD97,		// 青ジャック基板にiPad 9.7型液晶を使用した15KHzモニタ用→なんか良い愛称ないんでしょうか
	LHRD56_1366x768,	// 同 1366x768液晶用ファーム
	M_RT2556_FHD,		// 黒ジャック基板にgithubで見つけたファームウェア適用
	PHI_252B9,			// PHILIPS 252B9/11
	PCB800099,			// RTD2662使用基板
	RTD2668,
	V_M56VDA_IPAD97,	// V.M56VDA iPad基板 okaz氏情報提供ありがとうございます
	JG2555TC_IPAD97,	// JG2555TC iPad基板 Thanks Newman
};

enum enIndex
{
	X68_15K_I,		// X68000 15KHz Interlace
	X68_15K_P,		// X68000 15KHz Progressive
	X68_24K_I,		// X68000 24KHz Interlace
	X68_24K_P,		// X68000 24KHz Progressive
	X68_31K,		// X68000 31KHz
	X68_Memtest,	// X68000 memtest68K 31KHz
	X68_Dash,		// X68000 ダッシュ野郎 31KHz
	X68_FZ24K,		// X68000 Fantasy Zone 24KHz
	X68_Druaga,		// X68000 Druaga 31KHz
	FMT_Raiden,		// FM TOWNS 雷電伝説 31KHz
	FMT_SRMP2PS,	// FM TOWNS スーパーリアル麻雀P2&P3 31KHz
	M72_RTYPE,		// IREM M72 R-TYPE 15KHz
	MVS,			// NEO GEO MVS 15KHZ
	GEN_15K_P,		// Generic 15KHz Progressive
	MAX_INDEX
};

BYTE *buf = NULL;
int nModeTableStart = 0;
int nFileLen = 0;

char *MakePath(const char *szBasePath, const char *szAddFname, const char *szModExt=NULL)
{
	static char szPath[4096];
	char	szDrive[256], szDir[256], szFilename[256], szExt[256];
#ifdef WIN32
	_splitpath(szBasePath, szDrive, szDir ,szFilename, szExt);
	strcat(szFilename, szAddFname);
	if (szModExt) strcpy(szExt, szModExt);
	_makepath(szPath, szDrive, szDir, szFilename, szExt);
#else
	strcpy(szDir, szBasePath);
	strcpy(szDir, dirname(szDir));
	strcpy(szFilename, szBasePath);
	strcpy(szFilename, basename(szFilename));
	char *p = strchr(szFilename, '.');
	if (p) {
	    strcpy(szExt, p);
	    *p = '\0';
	}
	else {
	    strcpy(szExt, "");
	}
	if (szModExt) strcpy(szExt, szModExt);
	strcpy(szPath, szDir);		
	strcat(szPath, "/");
	strcat(szPath, szFilename);
	strcat(szPath, szAddFname);
	strcat(szPath, szExt);
#endif
	return szPath;
}


template <typename T>
int FindModeTable(int nLength, int &nCount)
{
	int nStart = -1, nSearchStart = 0;

	nCount = 0;

L_RETRY:
	int nInfoSize = sizeof(T);
	for (int i=nSearchStart; i<nLength-nInfoSize*2; i++) {
		T *pInfo1 = (T *)&buf[i];
		T *pInfo2 = (T*)&buf[i+nInfoSize];
		short nWidth = ntohs(pInfo1->width);
		short nHeight = ntohs(pInfo1->height);
		short nHStart = ntohs(pInfo1->hstart);
		short nVStart = ntohs(pInfo1->vstart);
		if (512 <= nWidth && nWidth <= 4096 &&
			200 <= nHeight && nHeight <= 2048 &&
			nHStart < nWidth && nVStart < nHeight && 
			5 <= pInfo1->htolerance && pInfo1->htolerance <= 10 &&
			5 <= pInfo1->vtolerance && pInfo1->vtolerance <= 10 &&
			5 <= pInfo2->htolerance && pInfo2->htolerance <= 10 &&
			5 <= pInfo2->vtolerance && pInfo2->vtolerance <= 10) {
			nStart = i;
			break;
		}
	}

	if (0 <= nStart) {
		for (int i=0; i<999; i++) {
			T *pInfo = (T *)&buf[nStart+nInfoSize*i];
			short nWidth = ntohs(pInfo->width);
			short nHeight = ntohs(pInfo->height);
			if (nWidth < 512 || 4096 < nWidth ||
				nHeight < 200 || 2048 < nHeight) {
				break;
			}
			nCount++;
		}
	}
	if (nCount == 0 && 0 <= nStart) {
		nSearchStart = nStart+1;
		goto L_RETRY;
	}

	return nStart;
}

void DumpModeTableRecord(FILE *fpCsv, T_Info *pInfo, int nNo, int nOffset)
{
	char	szStr[1024], szWk[1024];
	short	nWidth, nHeight, nHFreq, nVFreq, nHTotal, nVTotal, nHStart, nVStart;

	nWidth = ntohs(pInfo->width);
	nHeight = ntohs(pInfo->height);
	nHFreq = ntohs(pInfo->hfreq);
	nVFreq = ntohs(pInfo->vfreq);
	nHTotal = ntohs(pInfo->htotal);
	nVTotal = ntohs(pInfo->vtotal);
	nHStart = ntohs(pInfo->hstart);
	nVStart = ntohs(pInfo->vstart);

	sprintf(szStr, "%3d,0x%04X,", nNo, nOffset);
	sprintf(szWk, "0x%02X,", pInfo->polarity); strcat(szStr, szWk);
	sprintf(szWk, "%4d,", nWidth); strcat(szStr, szWk);
	sprintf(szWk, "%4d,", nHeight); strcat(szStr, szWk);
	sprintf(szWk, "%4d,", nHFreq); strcat(szStr, szWk);
	sprintf(szWk, "%4d,", nVFreq); strcat(szStr, szWk);
	sprintf(szWk, "%3d,", pInfo->htolerance); strcat(szStr, szWk);
	sprintf(szWk, "%3d,", pInfo->vtolerance); strcat(szStr, szWk);
	sprintf(szWk, "%4d,", nHTotal); strcat(szStr, szWk);
	sprintf(szWk, "%4d,", nVTotal); strcat(szStr, szWk);
	sprintf(szWk, "%4d,", nHStart); strcat(szStr, szWk);
	sprintf(szWk, "%4d\n", nVStart); strcat(szStr, szWk);

	fputs(szStr, fpCsv);
}

void DumpModeTableRecord(FILE *fpCsv, T_Info_23 *pInfo, int nIdx, int nOffset)
{
	char	szStr[1024], szWk[1024];
	short	nWidth, nHeight, nHFreq, nVFreq, nHTotal, nVTotal, nHStart, nVStart, nVCount;

	nWidth = ntohs(pInfo->width);
	nHeight = ntohs(pInfo->height);
	nHFreq = ntohs(pInfo->hfreq);
	nVFreq = ntohs(pInfo->vfreq);
	nHTotal = ntohs(pInfo->htotal);
	nVTotal = ntohs(pInfo->vtotal);
	nHStart = ntohs(pInfo->hstart);
	nVStart = ntohs(pInfo->vstart);
	nVCount = ntohs(pInfo->vcount);

	sprintf(szStr, "%3d,0x%04X,", nIdx, nOffset);
	sprintf(szWk, "0x%02X,", pInfo->no); strcat(szStr, szWk);
	sprintf(szWk, "0x%02X,", pInfo->type); strcat(szStr, szWk);
	sprintf(szWk, "0x%02X,", pInfo->polarity); strcat(szStr, szWk);
	sprintf(szWk, "%4d,", nWidth); strcat(szStr, szWk);
	sprintf(szWk, "%4d,", nHeight); strcat(szStr, szWk);
	sprintf(szWk, "%4d,", nHFreq); strcat(szStr, szWk);
	sprintf(szWk, "%4d,", nVFreq); strcat(szStr, szWk);
	sprintf(szWk, "%3d,", pInfo->htolerance); strcat(szStr, szWk);
	sprintf(szWk, "%3d,", pInfo->vtolerance); strcat(szStr, szWk);
	sprintf(szWk, "%4d,", nHTotal); strcat(szStr, szWk);
	sprintf(szWk, "%4d,", nVTotal); strcat(szStr, szWk);
	sprintf(szWk, "%4d,", nHStart); strcat(szStr, szWk);
	sprintf(szWk, "%4d,", nVStart); strcat(szStr, szWk);
	sprintf(szWk, "%4d\n", nVCount); strcat(szStr, szWk);

	fputs(szStr, fpCsv);
}

template <typename T>
int SetParameter(
int		nModeTableNo,
char	cPolarity,
WORD	nWidth,
WORD	nHeight,
WORD	nHFreq,
WORD	nVFreq,
BYTE	nHTol,
BYTE	nVTol,
WORD	nHTotal,
WORD	nVTotal,
WORD	nHStart,
WORD	nVStart
)
{
	if (nModeTableNo < 0) return -1;
	int nOffset = nModeTableStart+sizeof(T)*nModeTableNo;
	T *pInfo = (T *)&buf[nOffset];	

	if (0 < cPolarity) pInfo->polarity = cPolarity;
	if (0 < nWidth) pInfo->width = htons(nWidth);
	if (0 < nHeight) pInfo->height = htons(nHeight);
	if (0 < nHFreq) pInfo->hfreq = htons(nHFreq);
	if (0 < nVFreq) pInfo->vfreq = htons(nVFreq);
	if (0 < nHTol) pInfo->htolerance = nHTol;
	if (0 < nVTol) pInfo->vtolerance = nVTol;
	if (0 < nHTotal) pInfo->htotal = htons(nHTotal);
	if (0 < nVTotal) pInfo->vtotal = htons(nVTotal);
	if (0 < nHStart) pInfo->hstart = htons(nHStart);
	if (0 < nVStart) pInfo->vstart = htons(nVStart);

//	DumpModeTableRecord(fpCsv, pInfo, nModeTableNo, nOffset);

	return 0;
}

int FindKey(BYTE key[], int nKeyLen)
{
	int nRet = -1;

	for (int i=0; i<nFileLen-nKeyLen; i++) {
		if (memcmp(&buf[i], key, nKeyLen) == 0) {
			nRet = i;
			break;
		}
	}

	return nRet;
}

/**
		1.水平同期信号幅のチェックを無効化
		2.・プリセット画面高さのチェックを変更
		2.i水色ジャック基板(LH-RD56)のiPad液晶用ファームのピクセルクロック上限・下限を変更

		水平同期信号幅が水平同期幅の1/7を越える信号が表示できない
		X68000のFantasy Zone 24KやFM TOWNSのスーパーリアル麻雀P2&P3が該当
		P2314H,252B9,iPad液晶用基板などで効果ありそう
*/
bool ModifyFirmware(enModel model)
{
	bool	bRet = false;
	int		nPosSyncWidthCheck, nPosVHeightCheck, nPosVHeightCheck2, nPosDClkMin;
	int		nOfsVHeightCheck;
	BYTE	keyHSyncWidthCheck[] = {0xE0, 0xFA, 0xA3, 0xE0, 0xFB, 0x7C, 0x00, 0x7D, 0x07};
	BYTE	keyVHeightCheck[] = {0x50, 0x12, 0xC3, 0xED, 0x94, 0xF0};
	BYTE	keyDClkMin[] = {0x7F, 0x10, 0x7E, 0x15, 0x7D, 0x03, 0x7C, 0x00};	// 202000

	nPosSyncWidthCheck = FindKey(keyHSyncWidthCheck, 9);
	if (nPosSyncWidthCheck < 0) {
		fprintf(stderr, "keyHSyncWidthCheck not find\n");
		goto L_RET;
	}

	nPosVHeightCheck2 = -1;
	if (model == P2314H) {	// P2214H/P2314Hはこちら
		BYTE	keyVHeightCheck1[] = {0xC3, 0xE5, 0x5A, 0x94, 0xF0};
		BYTE	keyVHeightCheck2[] = {0xC3, 0xE5, 0x56, 0x94, 0xEF};
		nPosVHeightCheck = FindKey(keyVHeightCheck1, 5);
		if (nPosVHeightCheck < 0) {
			fprintf(stderr, "keyVHeightCheck not find\n");
			goto L_RET;
		}
		nPosVHeightCheck2 = FindKey(keyVHeightCheck2, 5);
		if (nPosVHeightCheck2 < 0) {
			fprintf(stderr, "keyVHeightCheck2 not find\n");
			goto L_RET;
		}
		nOfsVHeightCheck = 4;
	}
	else {
		nPosVHeightCheck = FindKey(keyVHeightCheck, 6);
		if (nPosVHeightCheck < 0) {
			fprintf(stderr, "keyVHeightCheck not find\n");
			goto L_RET;
		}
		nOfsVHeightCheck = 5;
	}

	nPosDClkMin = - 1;
	if (model == LHRD56_IPAD97 || model == V_M56VDA_IPAD97) {
		nPosDClkMin = FindKey(keyDClkMin, 8);
		if (nPosDClkMin < 0) {
			fprintf(stderr, "keyDClkMin not find\n");
			goto L_RET;
		}
	}

	buf[nPosSyncWidthCheck+8] = 0x01;	// HSyncWidth*7 < HTotalのチェックを*1にして無効化
	printf("Disable sync width check buf[%08X]=%X\n", nPosSyncWidthCheck+8, buf[nPosSyncWidthCheck+8]);
	buf[nPosVHeightCheck+nOfsVHeightCheck] = 0xC8;		// VTotalHeightの下限を240->200に緩和
	printf("Disalbe vheight check buf[%08X]=%X\n", nPosVHeightCheck+nOfsVHeightCheck, buf[nPosVHeightCheck+nOfsVHeightCheck]);
	if (0 <= nPosVHeightCheck2) {
		buf[nPosVHeightCheck2+nOfsVHeightCheck] = 0xC7;		// VTotalHeightの下限を240->200に緩和
		printf("Disalbe vheight chekc2 buf[%08X]=%X\n", nPosVHeightCheck2+nOfsVHeightCheck, buf[nPosVHeightCheck2+nOfsVHeightCheck]);
	}
	if (model == LHRD56_IPAD97 || model == V_M56VDA_IPAD97) {
		buf[nPosDClkMin+5] = 0x02;		// DClkMinを202000->136464に(50Hzだと170500くらいになるので)
		printf("Change dcl min buf[%08X]=%X\n", nPosDClkMin+5, buf[nPosDClkMin+5]);
	}

	bRet = true;

L_RET:
	return bRet;
}

int RTD2662ModeTableDump(
const char	*szPath,	//!< i	:
int			nMode		//!< i	:0=Dump 1=Modify -1=CheckOnly
)
{
	int i, ret, nModeTableCount, nOffset;
	int	nIdxNo[MAX_INDEX] = {-1};
	FILE *fp = NULL;
	FILE *fpCsv = NULL;
	FILE *fpOut = NULL;
	enModel	model = UNKNOWN;
	struct stat st;
	char szFilePath[4096];
	bool bModify;

	strcpy(szFilePath, szPath);

	fp = fopen(szFilePath, "rb");
	if (!fp) {
		fprintf(stderr, "can't open %s\n", szFilePath);
		ret = 1;
		goto L_RET;
	}

	stat(szFilePath, &st);
	nFileLen = st.st_size;
	buf = (BYTE *)malloc(nFileLen);
	if (!buf) {
		fprintf(stderr, "can't alloc memory %d\n", nFileLen);
		ret = 3;
		goto L_CLOSE_CSV;
	}

	ret = fread(buf, nFileLen, 1, fp);
	if (!ret) {
		fprintf(stderr, "can't read %s\n", szFilePath);
		ret = 4;
		goto L_FREE;
	}

	/* P2314の途中にある解像度テーブル？
	int ofs = 0x32344;
	for (i=0; i<77; i++) {
		printf("%d ofs(%x)=%d\n", i, 0x134cc+i*2, buf[0x134cc+i*2]*256+buf[0x134cc+i*2+1]);
	}

	for (i=0; i<256; i++) {
		printf("%d ofs(%x)=%d\n", i, ofs+i*2, buf[ofs+i*2]*256+buf[ofs+i*2+1]);
	}
	return 0;
	*/

	nModeTableStart = FindModeTable<T_Info>(nFileLen, nModeTableCount);
	if (nModeTableStart < 0) {
		nModeTableStart = FindModeTable<T_Info_23>(nFileLen, nModeTableCount);
		if (nModeTableStart < 0) {
			fprintf(stderr, "can't find mode table\n");
			ret = 5;
			goto L_FREE;
		}
		model = RTD2668;
	}

	// P2314Hの画面ﾓｰﾄﾞと使用ﾌﾟﾘｾｯﾄﾃｰﾌﾞﾙNoの紐づけ 
	memset(nIdxNo, -1, sizeof(nIdxNo));
	nIdxNo[X68_15K_I] = 0;		//  0:640x350 31.5KHz/70Hz
	nIdxNo[X68_15K_P] = 139;//1;		//  1;640x350 31.5KHz/70Hz
	nIdxNo[X68_24K_I] = 4;		//  4:720x400 31.5KHz/70Hz
	nIdxNo[X68_24K_P] = 5;		//  5:640x400 31.5KHz/70Hz
	nIdxNo[X68_31K] = 6;		//  6:720x400 38.0KHz/85Hz
	nIdxNo[X68_Memtest] = 7;	//  7:640x400 38.0KHz/85Hz
	nIdxNo[X68_Dash] = 14;		// 14:720x480 29.8KHz/59.9Hz
	nIdxNo[X68_FZ24K] = 16;		// 16:720x576 35.8KHz/60Hz
	nIdxNo[X68_Druaga] = 17;	// 17:720x576 45.5KHz/75.6Hz
	nIdxNo[FMT_Raiden] = 24;	// 24:832x624 49.7KHz/74.5Hz
	nIdxNo[FMT_SRMP2PS] = 25;	// 25:848x480 31.0KHz/60Hz
	nIdxNo[M72_RTYPE] = 26;		// 26:848x480 35.0KHz/70Hz
	nIdxNo[MVS] = 27;			// 27:848x480 36.0KHz/72Hz
	nIdxNo[GEN_15K_P] = 87;		// 87:1440x240 15.7KHz/60Hz

	if (model == UNKNOWN) {
		// ﾓﾃﾞﾙ自動判定 ModeTable開始位置から判定 中華液晶基板ではﾌｧｰﾑｳｪｱが頻繁に変わるからあまり意味なし
		switch (nModeTableStart) {
		case 0x200A:	// P2214H/P2314H
			printf("DELL P2214H/P2314H\n");
			model = P2314H;
			break;
		case 0x32A74:	// 252B9
			printf("PHILIPS 252B9/11\n");
			model = PHI_252B9;
			// プリセットテーブルはP2314Hと同じ
			break;
		case 0x5819:	// LH-RD56(V+H)-01 例のiPad9.7型液晶を使用した15KHzモニタ用 2048x1536.bin
			printf("LH-RD56(V+H) Light Blue Jack iPad 9.7\n");
			model = LHRD56_IPAD97;
			nIdxNo[X68_Dash] = 15;		// 15:720x480 29.8KHz/59.9Hz
			nIdxNo[X68_FZ24K] = 17;		// 17:720x576 35.8KHz/60Hz
			nIdxNo[X68_Druaga] = 18;	// 18:720x576 45.5KHz/75.6Hz
			nIdxNo[FMT_Raiden] = 25;	// 25:832x624 49.7KHz/74.5Hz
			nIdxNo[FMT_SRMP2PS] = 26;	// 26:848x480 31.0KHz/60Hz
			nIdxNo[M72_RTYPE] = 27;		// 27:848x480 35.0KHz/70Hzs
			nIdxNo[MVS] = 37;			// 37:1152x864 53.7KHz/60Hz
			nIdxNo[GEN_15K_P] = 86;		// 86:1440x240 15.7KHz/60Hz
			break;
		case 0xD97E:	// 同上 1366x768 UIは黒ジャックと同じ？
			printf("LH-RD56(V+H) Light Blue Jack 1366x768\n");
			model = LHRD56_1366x768;
			// プリセットテーブルはP2314Hとほぼ同じ
			break;
		case 0x4803C:	// M.RT2556 黒ジャック 1920x1080
			printf("M.RT2556 Black Jack 1920x1080\n");
			model = M_RT2556_FHD;
			// プリセットテーブルはP2314Hとほぼ同じ
			break;
		case 0x22129:	// okaz氏iPad基板 V.M56VDA
			printf("V.M56VDA Black Jack iPad 9.7\n");
			model = V_M56VDA_IPAD97;
			// プリセットテーブルはLH-RD56(V+H) iPad 9.7と同じ
			nIdxNo[X68_Dash] = 15;		// 15:720x480 29.8KHz/59.9Hz
			nIdxNo[X68_FZ24K] = 17;		// 17:720x576 35.8KHz/60Hz
			nIdxNo[X68_Druaga] = 18;	// 18:720x576 45.5KHz/75.6Hz
			nIdxNo[FMT_Raiden] = 25;	// 25:832x624 49.7KHz/74.5Hz
			nIdxNo[FMT_SRMP2PS] = 26;	// 26:848x480 31.0KHz/60Hz
			nIdxNo[M72_RTYPE] = 27;		// 27:848x480 35.0KHz/70Hzs
			nIdxNo[MVS] = 37;			// 37:1152x864 53.7KHz/60Hz
			nIdxNo[GEN_15K_P] = 86;		// 86:1440x240 15.7KHz/60Hz
			break;
		case 0x32000:	// JG2555TC
			printf("JG2555TC Balck Jack iPad 9.7\n");
			model = JG2555TC_IPAD97;
			// プリセットテーブルはP2314Hとほぼ同じ
			break;
		case 0x39c7:	// PCB800099(RTD2660/RTD2662)
			model = PCB800099;
			nIdxNo[X68_15K_I] = 0;
			nIdxNo[X68_15K_P] = 3;
			nIdxNo[X68_24K_I] = 4;
			nIdxNo[X68_24K_P] = 13;
			nIdxNo[X68_31K] = 18;
			nIdxNo[X68_Memtest] = 19;
			nIdxNo[X68_Dash] = 20;
			nIdxNo[X68_FZ24K] = 21;
			nIdxNo[X68_Druaga] = 22;
			nIdxNo[FMT_Raiden] = 23;
			nIdxNo[FMT_SRMP2PS] = 24;
			nIdxNo[M72_RTYPE] = 25;
			nIdxNo[MVS] = 26;
			break;
		}
	}
	if (model == UNKNOWN) {
		fprintf(stderr, "unknown firmware\n");
	}
	
	if (nMode == 1 && model != UNKNOWN && model != RTD2668) {

#if 1
		if (model != JG2555TC_IPAD97) {
			bModify = ModifyFirmware(model);
			if (bModify) {
				printf("*** modify firmware success ***\n");
			}
			else {
				fprintf(stderr, "can't modify firmware\n");
			}
		}
#else
		bModify = false;
#endif

		//											Pol   Wid   Hei  HFrq VFrq HT VT HTot  VTot HSB  VSB
  		//SetParameter<T_Info>(nIdxNo[X68_15K_I],		0x1F,  512, 480, 159, 615, 5, 5,  608, 521,  78, 24);		// X68000  512x512 15KHz(interlace) -> 標準の480iﾌﾟﾘｾｯﾄが使用されるため無効
		SetParameter<T_Info>(nIdxNo[X68_15K_P],		0x0F, 1472, 240, 159, 615, 5, 5, 1716, 262, 238, 20);		// X68000  512x240 15KHz
		SetParameter<T_Info>(nIdxNo[X68_24K_I],		0x1F, 1024, 848, 246, 532, 5, 5, 1408, 931, 282, 46);		// X68000 1024x848 24KHz(interlace) 偶数・奇数ライン逆？
		SetParameter<T_Info>(nIdxNo[X68_24K_P],		0x0F, 1024, 424, 246, 532, 5, 5, 1408, 465, 282, 23);		// X68000 1024x424 24KHz
		SetParameter<T_Info>(nIdxNo[X68_31K],		0x0F,  768, 512, 314, 554, 5, 5, 1104, 568, 261, 32);		// X68000  768x512 31KHz
		SetParameter<T_Info>(nIdxNo[X68_Memtest],	0x0F,  768, 512, 340, 554, 5, 5, 1130, 613, 320, 41);		// X68000 memtest 31KHz
		SetParameter<T_Info>(nIdxNo[X68_Dash],		0x0F,  768, 536, 315, 543, 5, 5, 1176, 580, 308, 38);		// X68000 ダッシュ野郎
		SetParameter<T_Info>(nIdxNo[FMT_Raiden],	0x0F,  768, 512, 323, 603, 3, 3, 1104, 536, 240, 19);		// TOWNS 雷電伝説
		SetParameter<T_Info>(nIdxNo[M72_RTYPE],		0x0F,  768, 256, 157, 550, 5, 5, 1024, 284, 156, 24);		// R-TYPE基板 15.7KHz/55Hz KAPPY.さん提供
		if (bModify) {
			// 水平同期信号幅のﾁｪｯｸを外さないと下記ﾌﾟﾘｾｯﾄは映らない
			SetParameter<T_Info>(nIdxNo[X68_FZ24K],		0x0F,  640, 448, 245, 524, 5, 5,  944, 469,  64, 10);		// X68000 Fantasy Zone 24KHz		※ModifyFirmwareが通用した場合のみ対応
			SetParameter<T_Info>(nIdxNo[X68_Druaga],	0x0F,  672, 560, 315, 530, 5, 5, 1104, 595, 108, 31);		// X68000 Druaga 31KHz				※ModifyFirmwareが通用した場合のみ対応
			SetParameter<T_Info>(nIdxNo[FMT_SRMP2PS],	0x0F,  736, 480, 320, 609, 3, 3,  896, 525, 144,  4);		// TOWNS スーパーリアル麻雀P2&P3	※ModifyFirmwareが通用した場合のみ対応
			//縦像度240未満の定義はVTotalHeight下限ﾁｪｯｸを除去しないと範囲外ｴﾗｰ表示で映らない
			SetParameter<T_Info>(nIdxNo[GEN_15K_P], 0x00, 0, 0, 0, 0, 5, 5, 0, 0, 0, 0);						// 元からある15.7KHz/Progressive/1440x240pの許容誤差を10->5に変更
			if (model == LHRD56_IPAD97 || model == V_M56VDA_IPAD97) {	// VTotal 232以下ﾀﾞﾒっぽい
				//buf[0x3E040] = 0xE4;	// DEBUG MODE ENABLE
				//buf[0x4A950] = 0xC3;	// D3->C3 UserInterfaceGetDclkNoSupportStatusを常にreturn FALSEに
				SetParameter<T_Info>(nIdxNo[MVS], 0x0F, 576, 232, 157, 591, 3, 3, 768, 263, 120, 24);				// MVS基板 15.7KHz/59.1Hz KAPPY.さん提供
				SetParameter<T_Info>(136, 0x00, 0, 0, 0, 0, 5, 5, 0, 0, 0, 0);						// 元からある15.7KHz/Progressive/1440x240pの許容誤差を10->5に変更
				SetParameter<T_Info>(148, 0x00, 0, 0, 0, 0, 5, 5, 0, 0, 0, 0);						// 元からある15.7KHz/Progressive/720x240pの許容誤差を10->5に変更
			}
			else {
				SetParameter<T_Info>(nIdxNo[MVS], 0x0F, 576, 224, 157, 591, 3, 3, 768, 263, 120, 24);				// MVS基板 15.7KHz/59.1Hz KAPPY.さん提供
			}
		}

		strcpy(szFilePath, MakePath(szFilePath, "_mod"));
		fpOut = fopen(szFilePath, "wb");
		if (fpOut) {
			ret = fwrite(buf, nFileLen, 1, fpOut);
			if (ret) {
				printf("modify done\n");
			}
			else {
				fprintf(stderr, "can't write %s\n", szFilePath);
			}
			fclose(fpOut);
		}
		else {
			fprintf(stderr, "can't open %s\n", szFilePath);
		}
	}
	else if (nMode == 0) {
		strcpy(szFilePath, MakePath(szFilePath, "", ".csv"));
		fpCsv = fopen(szFilePath, "wt");
		if (!fpCsv) {
			fprintf(stderr, "can't open %s\n", szFilePath);
			ret = 2;
			goto L_CLOSE;
		}
		nOffset = nModeTableStart;
		if (model == RTD2668) {
							//   0	        1         2         3         4         5         6         7         8        
							//   123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890
			fprintf(fpCsv, "Idx,Offset,No  ,Type,PF  ,W   ,H   ,HFrq,VFrq,HTl,VTl,HTot,VTot,HSta,VSta,IVCo\n");
		}
		else {
							//   0	        1         2         3         4         5         6         7         8        
							//   123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890
			fprintf(fpCsv, "No ,Offset,PF  ,W   ,H   ,HFrq,VFrq,HTl,VTl,HTot,VTot,HSta,VSta\n");
		}
		for (i=0; i<nModeTableCount; i++) {
			if (model == RTD2668) {
				struct T_Info_23 *pInfo = (T_Info_23 *)&buf[nOffset];
				DumpModeTableRecord(fpCsv, pInfo, i, nOffset);
				nOffset += sizeof(T_Info_23);
			}
			else {
				struct T_Info *pInfo = (T_Info *)&buf[nOffset];
				DumpModeTableRecord(fpCsv, pInfo, i, nOffset);
				nOffset += sizeof(T_Info);
			}
		}
	}

	ret = (model == UNKNOWN) ? -1 : 0;

L_FREE:
	free(buf);
L_CLOSE_CSV:
	if (fpCsv) fclose(fpCsv);
L_CLOSE:
	fclose(fp);
L_RET:
	return ret;
}


