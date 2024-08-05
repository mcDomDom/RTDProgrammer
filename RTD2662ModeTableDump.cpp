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
	struct T_Info {		// 19Byte/Rec
		BYTE	polarity;	// Bit0-3:Polarity Flag? Bit6:Interlace?
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
	struct T_Info_23 {	// 23Byte/Rec
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

enum enIndex
{
//	X68_15K_I,		// X68000 15KHz Interlace
	X68_15K_P,		// X68000 15KHz Progressive
//	X68_24K_I,		// X68000 24KHz Interlace
	X68_24K_P,		// X68000 24KHz Progressive
	X68_31K,		// X68000 31KHz
	X68_Memtest,	// X68000 memtest68K 31KHz
	X68_Dash,		// X68000 ダッシュ野郎 31KHz
	X68_FZ24K,		// X68000 Fantasy Zone 24KHz
	X68_Druaga,		// X68000 Druaga 31KHz
	FMT_Raiden,		// FM TOWNS 雷電伝説 31KHz
	FMT_SRMP2PS,	// FM TOWNS スーパーリアル麻雀P2&P3 31KHz
	FMT_LINUX,		// FM TOWNS Linuxコンソール 31KHz
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
		short nHFreq = ntohs(pInfo1->hfreq);
		short nVFreq = ntohs(pInfo1->vfreq);
		if (512 <= nWidth && nWidth <= 4096 &&
			200 <= nHeight && nHeight <= 2048 &&
			nHStart < nWidth && nVStart < nHeight && 
			150 < nHFreq && nHFreq < 1500 && 
			200 < nVFreq && nVFreq < 1500 && 
			nWidth%2 == 0 && nVStart < 100 &&
			5 <= pInfo1->htolerance && pInfo1->htolerance <= 12 &&
			5 <= pInfo1->vtolerance && pInfo1->vtolerance <= 12 &&
			5 <= pInfo2->htolerance && pInfo2->htolerance <= 12 &&
			5 <= pInfo2->vtolerance && pInfo2->vtolerance <= 12) {
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
	else if (model == JG2555TC_IPAD97) {
		printf("Skip VHeightCheck\n");
		nPosVHeightCheck = -1;
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
	if (0 <= nPosVHeightCheck) {
		buf[nPosVHeightCheck+nOfsVHeightCheck] = 0xC8;		// VTotalHeightの下限を240->200に緩和
		printf("Disalbe vheight check buf[%08X]=%X\n", nPosVHeightCheck+nOfsVHeightCheck, buf[nPosVHeightCheck+nOfsVHeightCheck]);
	}
	if (0 <= nPosVHeightCheck2) {
		buf[nPosVHeightCheck2+nOfsVHeightCheck] = 0xC7;		// VTotalHeightの下限を240->200に緩和
		printf("Disalbe vheight chekc2 buf[%08X]=%X\n", nPosVHeightCheck2+nOfsVHeightCheck, buf[nPosVHeightCheck2+nOfsVHeightCheck]);
	}
	if (model == LHRD56_IPAD97 || model == V_M56VDA_IPAD97 || model == V_M56VDA_IPAD97_2|| model == LHRD56_IPAD97_POO ) {
		buf[nPosDClkMin+5] = 0x02;		// DClkMinを202000->136464に(50Hzだと170500くらいになるので)
		printf("Change dcl min buf[%08X]=%X\n", nPosDClkMin+5, buf[nPosDClkMin+5]);
	}

	bRet = true;

L_RET:
	return bRet;
}

#if 0
void SplitOut()
{
	char szBuf[4096];
	int i, j;
	for (i=0; i<6; i++) {
		sprintf(szBuf, "d:\\EK241Y_%d.bin", i);
		FILE *fp = fopen(szBuf, "wb");
		fwrite(&buf[i*0x10000], 0x10000, 1, fp);
		fclose(fp);
	}
}

void SearchAspect()
{					//00   01   02   03   04   05   06   07   08   09   0a   0b   0c   0d   0e   0f
	BYTE szTable[] = {' ', '\'','(', ')', ',', '-', '.', '/', '0', '1', '2', '3', '4', '5', '6', '7', 
					  '8', '9', '0', ':', ';', '?', '@', '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 
   					  'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'W', 
					  'X', 'Y', 'Z', 'ｱ', 'ｲ', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 
					  'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 'ｶ', 
				      'ｷ', 'ｸ', 'ｹ', 'ｺ', 'ｻ', 'ｼ', 'ｽ', 'ｾ', 'ｿ', 'ﾀ', 'ﾁ', 'ﾂ', 'ﾃ', 'ﾄ', 'ﾅ', 'ﾆ', 
					  'ﾇ', 'ﾈ', 'ﾉ', ' ', 'ﾊ', 'ﾋ', 'ﾌ', 'ﾍ', 'ﾎ', 'ﾏ', 'ﾐ', 'ﾑ', 'ﾒ', 'ﾓ', 'ﾔ', 'ﾕ', 
					  'ﾖ', 'ﾜ', 'ｦ', 'ﾝ'};
	char szBuf[4096];
	int i, j;
	int size = sizeof(szTable);
	for (i=0; i<nFileLen; i++) {
		for (j=0; j<4096 ; j++) {
			if (0 <= buf[i+j] && buf[i+j] < size) {
				szBuf[j] = szTable[buf[i+j]];
			}
			else {
				break;
			}
		}
		szBuf[j] = '\0';
		if (3 <= j) {
			printf("[%05X]:%s\n", i, szBuf);
			i += j;
		}
	}
/*
	BYTE key[] = {"spect"};
	for (int i=0; i<nFileLen-6; i++) {
		for (int c=-0x61; c<128; c++) {
			BYTE key2[5];
			memcpy(key2, key, 5);
			for (int j=0; j<5; j++) {
				key2[j] += c;
			}
			if (memcmp(&buf[i], key2, 5) == 0) {
				printf("Find %X c=%d\n", i, c);
				break;
			}
		}
	}
*/
}
#endif

bool DisableAcerAspectChangeCheck(enModel model)
{
	bool bRet = false;
	int nOffset[2] = {-1, -1};

	printf("Disable acer aspect change check : ");

	// Ignore Aspect Change Disable
	if (model == EK241YEbmix) {
		nOffset[0] = 0x5cbe9;
		nOffset[1] = 0x5cc25;
	}
	else if (model == EK271Ebmix) {
		nOffset[0] = 0x4b079;
		nOffset[1] = 0x4b0b5;
	}
	else if (model == QG221QHbmiix) {
		nOffset[0] = 0x6a191;
		nOffset[1] = 0x6a1d2;
	}
	else if (model == C24M2020DJP) {
		nOffset[0] = 0x3d58a;
		nOffset[1] = 0x3d5cb;
	}
	else if (model == KA222Q) {
		nOffset[0] = 0x1d693;
		nOffset[1] = 0x1d6d4;
	}
	else {
		printf("Invlid model\n");
		goto L_RET;
	}
	if (buf[nOffset[0]] == 0xC2 && buf[nOffset[1]] == 0xC2) {
		buf[nOffset[0]] = 0xD2;
		buf[nOffset[1]] = 0xD2;
		printf("Done\n");
		bRet = true;
	}
	else {
		printf("Faile buf[%05X]=%02X buf[%05X]=%02X\n", nOffset[0], buf[nOffset[0]], nOffset[1], buf[nOffset[1]] );
	}

L_RET:
	return bRet;
}

void OutputAcerISTPTR(enModel model, int &nOffset)
{
	if (model == EK241YEbmix) {
		buf[nOffset++] = 0x12;	buf[nOffset++] = 0x17;	buf[nOffset++] = 0xBA;
	}
	else if (model == EK271Ebmix) {
		buf[nOffset++] = 0x12;	buf[nOffset++] = 0x17;	buf[nOffset++] = 0x66;
	}
	else if (model == QG221QHbmiix) {
		buf[nOffset++] = 0x12;	buf[nOffset++] = 0x18;	buf[nOffset++] = 0x51;
	}
	else if (model == C24M2020DJP) {
		buf[nOffset++] = 0x12;	buf[nOffset++] = 0x15;	buf[nOffset++] = 0xD9;
	}
	else if (model == KA222Q) {
		buf[nOffset++] = 0x12;	buf[nOffset++] = 0x12;	buf[nOffset++] = 0xCF;
	}
}

int GetAspectFunctionOffset(enModel model, int &nOffsetRet)
{
	int nOffset = -1;
	nOffsetRet = -1;
	if (model == EK241YEbmix) {
		nOffset = 0x2f225;
		nOffsetRet = 0x2f26c;
	}
	else if (model == EK271Ebmix) {
		nOffset = 0x2f486;
		nOffsetRet = 0x2f4cd;
	}
	else if (model == QG221QHbmiix) {
		nOffset = 0x2f156;
		nOffsetRet = 0x2f19f;
	}
	else if (model == C24M2020DJP) {
		nOffset = 0x5e746;
		nOffsetRet = 0x5e78f;
	}
	else if (model == KA222Q) {
		nOffset = 0x1eff8;
		nOffsetRet = 0x1f041;
	}
	else {
		printf("Invlid model\n");
	}
	return nOffset;
}

// Force 4:3(640x480)->未使用
void ModifyAcerAspectFunctionForce4x3(enModel model)
{
	int nOffsetRet;
	int nOffset = GetAspectFunctionOffset(model, nOffsetRet);
	if (nOffset < 0) {
		return;
	}
	// Aspect H 640
	buf[nOffset++] = 0x74;	buf[nOffset++] = 0x02;	// MOV A, #0x02
	buf[nOffset++] = 0x75;	buf[nOffset++] = 0xF0;	buf[nOffset++] = 0x80;	// MOV B, #0x80
	// LCALL ISTPTR
	OutputAcerISTPTR(model, nOffset);
	// Aspect V 480
	buf[nOffset++] = 0x74;	buf[nOffset++] = 0xE0;	// MOV A, #0x40
	buf[nOffset++] = 0xFF;							// MOV R7, A
	buf[nOffset++] = 0x74;	buf[nOffset++] = 0x01;	// MOV A, #0x01
	buf[nOffset++] = 0xF9;							// MOV R1, A
	// Restore param2 R2/R3
	if (model == EK241YEbmix) {
		buf[nOffset++] = 0x90;	buf[nOffset++] = 0xE0;	buf[nOffset++] = 0x88;	// MOV DPTR, #0xE088
		buf[nOffset++] = 0x12;	buf[nOffset++] = 0xE1;	buf[nOffset++] = 0x1B;	// LCALL restore R2/R3
	}
	else if (model == EK271Ebmix) {
		buf[nOffset++] = 0x90;	buf[nOffset++] = 0xE0;	buf[nOffset++] = 0x88;	// MOV DPTR, #0xE088
		buf[nOffset++] = 0x12;	buf[nOffset++] = 0xE7;	buf[nOffset++] = 0xC1;	// LCALL restore R2/R3
	}
	else if (model == QG221QHbmiix) {
		buf[nOffset++] = 0x90;	buf[nOffset++] = 0xE3;	buf[nOffset++] = 0x08;	// MOV DPTR, #0xE308
		buf[nOffset++] = 0x12;	buf[nOffset++] = 0xE9;	buf[nOffset++] = 0x88;	// LCALL restore R2/R3
	}
	else if (model == C24M2020DJP) {
		buf[nOffset++] = 0x90;	buf[nOffset++] = 0xE3;	buf[nOffset++] = 0x35;	// MOV DPTR, #0xE335
		buf[nOffset++] = 0x12;	buf[nOffset++] = 0xAB;	buf[nOffset++] = 0xF3;	// LCALL
																				//     restore R2/R3
																				//     XCH A,R1
																				//     MOV B,R7
																				//     CALL ISTPTR
		goto L_RET;
	}
	else if (model == KA222Q) {
		buf[nOffset++] = 0x80;	buf[nOffset++] = 0x39;	// SJMP 0xF041
		goto L_RET;
	}
	buf[nOffset++] = 0xC9;							// XCH A,R1
	buf[nOffset++] = 0x8F;	buf[nOffset++] = 0xF0;	// MOV B,R7
	// LCALL ISTPTR
	OutputAcerISTPTR(model, nOffset);
L_RET:
	buf[nOffset++] = 0x22;	// RET

	printf("Modified acer aspect function force 4:3 ratio\n");
}

// AcerのWide ModeメニューでAspect選択時の挙動を改造
// mode==ModeModifyの場合は常に4:3
// mode==ModeModify2の場合、縦解像度が350未満の場合のみ強制4:3 それ以外は元の解像度比率を使用
//       X68000の1024x848/1024x424モードでNot Supportになる不具合あり
//       他にも縦横比が異常なプリセットで影響出る可能性あり
bool ModifyAcerWideModeFunction(enMode mode, enModel model)
{
	// Height < 350 Force 4:3(640x480)
	int nOffsetRet;
	int nOffset = GetAspectFunctionOffset(model, nOffsetRet);
	if (nOffset < 0) {
		return false;
	}

	// 
	buf[nOffset++] = 0xE9;														// MOV A,R1
	buf[nOffset++] = 0xFC;														// MOV R4,A
	buf[nOffset++] = 0xEA;														// MOV A,R2
	buf[nOffset++] = 0xFD;														// MOV R5,A

	// Input Width
	if (model == EK241YEbmix || model == EK271Ebmix) {
		buf[nOffset++] = 0x90;	buf[nOffset++] = 0xE3;	buf[nOffset++] = 0xD1;	// MOV DPTR,#E3D1 
	}
	else if (model == QG221QHbmiix) {
		buf[nOffset++] = 0x90;	buf[nOffset++] = 0xE4;	buf[nOffset++] = 0xB8;	// MOV DPTR,#E4B8
	}
	else if (model == C24M2020DJP) {
		buf[nOffset++] = 0x90;	buf[nOffset++] = 0xE5;	buf[nOffset++] = 0x69;	// MOV DPTR,#E569
	}
	else if (model == KA222Q) {
		buf[nOffset++] = 0x90;	buf[nOffset++] = 0xE4;	buf[nOffset++] = 0x14;	// MOV DPTR,#E414
	}
	else {
		printf("code not implemented\n");
		return false;
	}
	// Set Input Width to HAspect
	buf[nOffset++] = 0xE0;														// MOVX A,@DPTR
	buf[nOffset++] = 0xFF;														// MOV R7,A
	buf[nOffset++] = 0xA3;														// INC DPTR
	buf[nOffset++] = 0xE0;														// MOVX A,@DPT
	buf[nOffset++] = 0xCF;														// XCH A,R7
	buf[nOffset++] = 0x8F;	buf[nOffset++] = 0xF0;								// MOV B,R7
	OutputAcerISTPTR(model, nOffset);											// LCALL ISTPTR

	// Input Height
	if (model == EK241YEbmix || model == EK271Ebmix) {
		buf[nOffset++] = 0x90;	buf[nOffset++] = 0xE3;	buf[nOffset++] = 0xDD;	// MOV DPTR,#E3DD
	}
	else if (model == QG221QHbmiix) {
		buf[nOffset++] = 0x90;	buf[nOffset++] = 0xE4;	buf[nOffset++] = 0xC4;	// MOV DPTR,#E4C4
	}
	else if (model == C24M2020DJP) {
		buf[nOffset++] = 0x90;	buf[nOffset++] = 0xE5;	buf[nOffset++] = 0x75;	// MOV DPTR,#E575
	}
	else if (model == KA222Q) {
		buf[nOffset++] = 0x90;	buf[nOffset++] = 0xE4;	buf[nOffset++] = 0x1E;	// MOV DPTR,#E41E
	}
	else {
		printf("code not implemented\n");
		return false;
	}
	// Set Input Height to R1/R7
	buf[nOffset++] = 0xE0;														// MOVX A,@DPTR
	buf[nOffset++] = 0xF9;														// MOV R1,A
	buf[nOffset++] = 0xA3;														// INC DPTR
	buf[nOffset++] = 0xE0;														// MOVX A,@DPTR
	buf[nOffset++] = 0xFF;														// MOV R7,A

	// Check Interlace Flag?
	if (model == EK241YEbmix || model == EK271Ebmix) {
		buf[nOffset++] = 0x90;	buf[nOffset++] = 0xE3;	buf[nOffset++] = 0x65;	// MOV DPTR,#E365
	}
	else  if (model == QG221QHbmiix) {
		buf[nOffset++] = 0x90;	buf[nOffset++] = 0xE4;	buf[nOffset++] = 0x4C;	// MOV DPTR,#E44C
	}
	else  if (model == C24M2020DJP) {
		buf[nOffset++] = 0x90;	buf[nOffset++] = 0xE5;	buf[nOffset++] = 0x35;	// MOV DPTR,#E535
	}
	else  if (model == KA222Q) {
		buf[nOffset++] = 0x90;	buf[nOffset++] = 0xE3;	buf[nOffset++] = 0xE6;	// MOV DPTR,#E3E6
	}
	else {
		printf("code not implemented\n");
		return false;
	}
	buf[nOffset++] = 0xE0;														// MOVX A,@DPTR
	if (model == EK241YEbmix) {
		buf[nOffset++] = 0x12;	buf[nOffset++] = 0xE0;	buf[nOffset++] = 0xF5;	// LCALL 0xE0F5
	}
	else if (model == EK271Ebmix) {
		buf[nOffset++] = 0x12;	buf[nOffset++] = 0xE7;	buf[nOffset++] = 0x9B;	// LCALL 0xE79B
	}
	else if (model == QG221QHbmiix || model == C24M2020DJP || model == KA222Q) {
		// 関数の中身が展開されている
		buf[nOffset++] = 0x13;													// RRC
		buf[nOffset++] = 0x13;													// RRC
		buf[nOffset++] = 0x54;	buf[nOffset++] = 0x3F;							// ANL A,#0x3F
		buf[nOffset++] = 0x30;	buf[nOffset++] = 0xE0;	buf[nOffset++] = 0x07;	// JNB Check VHeight < 350
		goto L_x2Height;
	}
	else {
		printf("code not implemented\n");
		return false;
	}
	buf[nOffset++] = 0x70;	buf[nOffset++] = 0x07;								// JNZ Check VHeight < 350

L_x2Height:
	// x2 VHeight
	buf[nOffset++] = 0xEF;														// MOV A,R7
	buf[nOffset++] = 0x25;	buf[nOffset++] = 0xE0;								// ADD A,A
	buf[nOffset++] = 0xFF;														// MOV R7,A
	buf[nOffset++] = 0xE9;														// MOV A,R1
	buf[nOffset++] = 0x33;														// RLC A
	buf[nOffset++] = 0xF9;														// MOV R1,A

	// Check VHeight < 350
	buf[nOffset++] = 0xC3;														// CLR CY
	buf[nOffset++] = 0xEF;														// MOV A,R7
	buf[nOffset++] = 0x94;	buf[nOffset++] = 0x5E;								// SUBB A,#0x5E
	buf[nOffset++] = 0xE9;														// MOV A,R1
	buf[nOffset++] = 0x94;	buf[nOffset++] = 0x01;								// SUBB A,#0x01 
	if (mode == ModeModify2) {
		// Use Original Aspect Ratio
		buf[nOffset++] = 0x50;	buf[nOffset++] = nOffsetRet-nOffset-1;			// JNC nOffsetRet
	}
	else {
		// Force 4:3 Aspect Ratio
		buf[nOffset++] = 0x00;													// NOP
		buf[nOffset++] = 0x00;													// NOP 
	}
	// Set Aspect 640x480(4:3)
	buf[nOffset++] = 0xEC;														// MOV A,R4
	buf[nOffset++] = 0xF9;														// MOV R1,A
	buf[nOffset++] = 0xED;														// MOV A,R5
	buf[nOffset++] = 0xFA;														// MOV R2,A
	buf[nOffset++] = 0x74;	buf[nOffset++] = 0x02;								// MOV A,#0x02
	buf[nOffset++] = 0x75;	buf[nOffset++] = 0xF0;	buf[nOffset++] = 0x80;		// MOV B,#0x80
	OutputAcerISTPTR(model, nOffset);											// LCALL ISTPTR
	buf[nOffset++] = 0x74;	buf[nOffset++] = 0xE0;								// MOV A,#0xE0
	buf[nOffset++] = 0xFF;														// MOV R7,A
	buf[nOffset++] = 0x74;	buf[nOffset++] = 0x01;								// MOV A,#0x01
	buf[nOffset++] = 0xF9;														// MOV R1,A

	for ( ; nOffset<nOffsetRet; nOffset++) buf[nOffset] = 0x00;					// NOP

	printf("Modified acer wide mode function %s aspect ration\n", mode == ModeModify2 ? "original" : "force 4:3");

	return true;
}


/**
		同一周波数&VTotalのインタレースとプログレッシブを別々のプリセットで識別する試み
		->同一周波数&VTotalで連続してインタレースとプログレッシブが切り替わってもコントローラが認識してくれない
		  電源を入れなおすと正しく認識するが、使いどころ微妙
*/
void ModifyAcerEK2xxYCompareInterlace(enModel model)
{
	int nOffset;
	if (model == EK241YEbmix) {
		nOffset = 0x4a304;
	}
	else if (model == EK271Ebmix) {
		nOffset = 0x5c832;
	}
	else {
		printf("Invlid model\n");
		return;
	}

	buf[nOffset++]=0x90; buf[nOffset++]=0xe0; buf[nOffset++]=0x93;
	buf[nOffset++]=0xef;
	buf[nOffset++]=0xf0;
	buf[nOffset++]=0x75; buf[nOffset++]=0xf0; buf[nOffset++]=0x13;
	buf[nOffset++]=0xed;
	buf[nOffset++]=0xa4;
	buf[nOffset++]=0x24; buf[nOffset++]=0x05;
	buf[nOffset++]=0xf5; buf[nOffset++]=0x82;
	buf[nOffset++]=0xe5; buf[nOffset++]=0xf0;
	buf[nOffset++]=0x34; buf[nOffset++]=0x26;
	buf[nOffset++]=0xf5; buf[nOffset++]=0x83;
	buf[nOffset++]=0xe4;
	buf[nOffset++]=0x93;
	buf[nOffset++]=0x54; buf[nOffset++]=0x40;
	buf[nOffset++]=0xc4;
	buf[nOffset++]=0x13;
	buf[nOffset++]=0x54; buf[nOffset++]=0x07;
	buf[nOffset++]=0x24; buf[nOffset++]=0xff;
	buf[nOffset++]=0x92; buf[nOffset++]=0xf7;
	buf[nOffset++]=0x90; buf[nOffset++]=0xe3; buf[nOffset++]=0xcc;	//
	buf[nOffset++]=0xe0;
	buf[nOffset++]=0x13;
	buf[nOffset++]=0x13;
	buf[nOffset++]=0x54; buf[nOffset++]=0x3f;
	buf[nOffset++]=0x13;
	buf[nOffset++]=0x30; buf[nOffset++]=0xf7; buf[nOffset++]=0x01;
	buf[nOffset++]=0xb3;
	buf[nOffset++]=0x40; buf[nOffset++]=0x06;
	buf[nOffset++]=0xed;
	if (model == EK241YEbmix) {
		buf[nOffset++]=0x12; buf[nOffset++]=0x16; buf[nOffset++]=0x23;	// CSTPTR
	}
	else if (model == EK271Ebmix) {
		buf[nOffset++]=0x12; buf[nOffset++]=0x15; buf[nOffset++]=0xCF;	// CSTPTR
	}
	buf[nOffset++]=0xd3;
	buf[nOffset++]=0x22;
	buf[nOffset++]=0xc3;
	buf[nOffset++]=0x22;

	// CSTPTR
	// BB 01 06 89 82 8A 83 F0 22 50 02 F7 22 BB FE 01 F3 22

	printf("Compare interlace flag enable\n");
}


int RTD2662ModeTableDump(
const char	*szPath,	//!< i	:
enMode		nMode		//!< i	:0=Dump 1=Modify 2=Modify2 -1=CheckOnly
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
		ret = -1;
		goto L_RET;
	}

	stat(szFilePath, &st);
	nFileLen = st.st_size;
	buf = (BYTE *)malloc(nFileLen);
	if (!buf) {
		fprintf(stderr, "can't alloc memory %d\n", nFileLen);
		ret = -3;
		goto L_CLOSE_CSV;
	}

	ret = fread(buf, nFileLen, 1, fp);
	if (!ret) {
		fprintf(stderr, "can't read %s\n", szFilePath);
		ret = -4;
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
			ret = -5;
			goto L_FREE;
		}
		model = RTD2668;
	}

	// P2314Hの画面ﾓｰﾄﾞと使用ﾌﾟﾘｾｯﾄﾃｰﾌﾞﾙNoの紐づけ 
	memset(nIdxNo, -1, sizeof(nIdxNo));
//ｲﾝﾀﾚｰｽ用の定義使われない事がわかったのでやめる 640x350/400等VTOWNSのBIOSで使ってるのでやめる
//	nIdxNo[X68_15K_I] = 0;		//  0:640x350 31.5KHz/70Hz
//	nIdxNo[X68_24K_I] = 4;		//  4:720x400 31.5KHz/70Hz
	nIdxNo[X68_15K_P] = 16;		// 16:720x576 35.8KHz/60Hz
	nIdxNo[X68_24K_P] = 17;		// 17:720x576 45.5KHz/75.6Hz
	nIdxNo[X68_31K] = 24;		// 24:832x624 49.7KHz/74.5Hz
	nIdxNo[X68_Memtest] = 25;	// 25:848x480 31.0KHz/60Hz
	nIdxNo[X68_Dash] = 26;		// 26:848x480 35.0KHz/70Hz
	nIdxNo[X68_FZ24K] = 27;		// 27:848x480 36.0KHz/72Hz
	nIdxNo[X68_Druaga] = 28;	// 28:848x480 37.6KHz/75Hz
	nIdxNo[FMT_Raiden] = 38;	// 38:1152x864 53.7KHz/60Hz
	nIdxNo[FMT_SRMP2PS] = 39;	// 39:1152x864 64.2KHz/70.2Hz
	nIdxNo[M72_RTYPE] = 40;		// 40:1152x864 67.5KHz/75Hz
	nIdxNo[MVS] = 41;			// 41:1152x864 67.0KHz/85Hz
	nIdxNo[FMT_LINUX] = 42;		// 42:1152x870 68.7KHz/75Hz
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
		case 0x32A76:	// Mig9氏 252B9
			printf("PHILIPS 252B9/11(Mig9)\n");
			model = PHI_252B9;
			// プリセットテーブルはP2314Hと同じ
			break;
		case 0x5819:	// LH-RD56(V+H)-01 例のiPad9.7型液晶を使用した15KHzモニタ用 2048x1536.bin
			printf("LH-RD56(V+H) Light Blue Jack iPad 9.7\n");
			model = LHRD56_IPAD97;
			nIdxNo[X68_15K_P] = 17;		// 17:720x576 35.8KHz/60Hz
			nIdxNo[X68_24K_P] = 18;		// 18:720x576 45.5KHz/75.6Hz
			nIdxNo[X68_31K] = 25;		// 25:832x624 49.7KHz/74.5Hz
			nIdxNo[X68_Memtest] = 26;	// 26:848x480 31.0KHz/60Hz
			nIdxNo[X68_Dash] = 27;		// 27:848x480 35.0KHz/70Hz
			nIdxNo[X68_FZ24K] = 42;		// 42:1152x900 61.8KHz/66Hz
			nIdxNo[X68_Druaga] = 43;	// 43:1152x900 61.8KHz/66Hz
			nIdxNo[FMT_Raiden] = 37;	// 37:1152x864 53.7KHz/60Hz
			nIdxNo[FMT_SRMP2PS] = 38;	// 38:1152x864 64.2KHz/70.2Hz
			nIdxNo[M72_RTYPE] = 39;		// 39:1152x864 67.5KHz/75Hz
			nIdxNo[MVS] = 40;			// 40:1152x864 67.0KHz/85Hz
			nIdxNo[FMT_LINUX] = 41;		// 41:1152x870 68.7KHz/75Hz
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
			nIdxNo[X68_15K_P] = 17;		// 17:720x576 35.8KHz/60Hz
			nIdxNo[X68_24K_P] = 18;		// 18:720x576 45.5KHz/75.6Hz
			nIdxNo[X68_31K] = 25;		// 25:832x624 49.7KHz/74.5Hz
			nIdxNo[X68_Memtest] = 26;	// 26:848x480 31.0KHz/60Hz
			nIdxNo[X68_Dash] = 27;		// 27:848x480 35.0KHz/70Hz
			nIdxNo[X68_FZ24K] = 42;		// 42:1152x900 61.8KHz/66Hz
			nIdxNo[X68_Druaga] = 43;	// 43:1152x900 61.8KHz/66Hz
			nIdxNo[FMT_Raiden] = 37;	// 37:1152x864 53.7KHz/60Hz
			nIdxNo[FMT_SRMP2PS] = 38;	// 38:1152x864 64.2KHz/70.2Hz
			nIdxNo[M72_RTYPE] = 39;		// 39:1152x864 67.5KHz/75Hz
			nIdxNo[MVS] = 40;			// 40:1152x864 67.0KHz/85Hz
			nIdxNo[FMT_LINUX] = 41;		// 41:1152x870 68.7KHz/75Hz
			nIdxNo[GEN_15K_P] = 86;		// 86:1440x240 15.7KHz/60Hz
			break;
		case 0x221A3:	// taobao 広州四維液晶貿易有限公司 V.M56VDA iPad
			printf("V.M56VDA Black Jack iPad 9.7(taobao)\n");
			model = V_M56VDA_IPAD97_2;
			nIdxNo[X68_15K_P] = 17;		// 17:720x576 35.8KHz/60Hz
			nIdxNo[X68_24K_P] = 18;		// 18:720x576 45.5KHz/75.6Hz
			nIdxNo[X68_31K] = 25;		// 25:832x624 49.7KHz/74.5Hz
			nIdxNo[X68_Memtest] = 26;	// 26:848x480 31.0KHz/60Hz
			nIdxNo[X68_Dash] = 27;		// 27:848x480 35.0KHz/70Hz
			nIdxNo[X68_FZ24K] = 42;		// 42:1152x900 61.8KHz/66Hz
			nIdxNo[X68_Druaga] = 43;	// 43:1152x900 61.8KHz/66Hz
			nIdxNo[FMT_Raiden] = 37;	// 37:1152x864 53.7KHz/60Hz
			nIdxNo[FMT_SRMP2PS] = 38;	// 38:1152x864 64.2KHz/70.2Hz
			nIdxNo[M72_RTYPE] = 39;		// 39:1152x864 67.5KHz/75Hz
			nIdxNo[MVS] = 40;			// 40:1152x864 67.0KHz/85Hz
			nIdxNo[FMT_LINUX] = 41;		// 41:1152x870 68.7KHz/75Hz
			nIdxNo[GEN_15K_P] = 86;		// 86:1440x240 15.7KHz/60Hz
			break;
		case 0x22386:
			printf("LH-RD56(V+H) Light Blue Jack iPad 9.7(poo)\n");
			model = LHRD56_IPAD97_POO;
			// プリセットテーブルはP2314Hとほぼ同じ
			break;
		case 0x32000:	// JG2555TC
			printf("JG2555TC Balck Jack iPad 9.7\n");
			model = JG2555TC_IPAD97;
			// プリセットテーブルはP2314Hとほぼ同じ
			break;
		case 0x39c7:	// PCB800099(RTD2660/RTD2662)
			printf("PCB800099\n");
			model = PCB800099;
//			nIdxNo[X68_15K_I] = 0;		
			nIdxNo[X68_15K_P] = 3;		
//			nIdxNo[X68_24K_I] = 4;		
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
		case 0x3841:	// PCB800099(Newman BluePCB)
			printf("PCB800099(Newman BluePCB)\n");
			model = PCB800099;
//			nIdxNo[X68_15K_I] = 0;		// 640x350
			nIdxNo[X68_15K_P] = 3;		// 720x400
//			nIdxNo[X68_24K_I] = 4;		// 720x400
			nIdxNo[X68_24K_P] = 6;		// 720x400
			nIdxNo[X68_31K] = 12;		// 800x480
			nIdxNo[X68_Memtest] = 13;	// 800x480
			nIdxNo[X68_Dash] = 19;		// 832x624
			nIdxNo[X68_FZ24K] = 20;		// 1024x600
			nIdxNo[X68_Druaga] = 27;	// 1024x800
			nIdxNo[FMT_Raiden] = 28;	// 1024x800
			nIdxNo[FMT_SRMP2PS] = 29;	// 1152x864
			nIdxNo[M72_RTYPE] = 30;		// 1152x864
			nIdxNo[MVS] = 31;			// 1152x864
			break;
		case 0x52A4A:	// Acer EK271Ebmix KAPPY.氏
			printf("Acer EK271Ebmix\n");
			model = EK271Ebmix;
			// プリセットテーブルはP2314Hとほぼ同じ
			break;
		case 0x42A55:	// Acer EK241YEbmix KAPPY.氏
			printf("Acer EK241YEbmix\n");
			model = EK241YEbmix;
			// プリセットテーブルはP2314Hとほぼ同じ
			break;
		case 0x52A56:	// Acer QG221QHbmiix KAPPY.氏
			printf("Acer QG221QHbmiix\n");
			model = QG221QHbmiix;
			// プリセットテーブルはP2314Hとほぼ同じ
			break;
		case 0x227B6:	// Amazon C24M2020DJP xbeeing氏
			printf("Amazon C24M2020DJP\n");
			model = C24M2020DJP;
			// プリセットテーブルはP2314Hとほぼ同じ
			break;
		case 0x134DB:	// Cocoper CZ-617Ph zinfyk氏
			printf("Cocoper CZ-617Ph\n");
			model = CZ617Ph;
			// プリセットテーブルはP2314Hとほぼ同じ
			break;
		case 0x320CC:	// Acer KA222Q CAT-2氏
			printf("Acer KA222Q\n");
			model = KA222Q;
			// プリセットテーブルはP2314Hとほぼ同じ
			break;
		}
	}
	if (model == UNKNOWN) {
		fprintf(stderr, "unknown firmware nModeTableStart=%X\n", nModeTableStart);
	}
	
	if ((nMode == ModeModify || nMode == ModeModify2) && model != UNKNOWN && model != RTD2668) {

#if 1
		if (model == EK271Ebmix || model == EK241YEbmix || model == QG221QHbmiix || 
			model == C24M2020DJP || model == KA222Q) {
			if (DisableAcerAspectChangeCheck(model)) {
				ModifyAcerWideModeFunction(nMode, model);
			}
			else {
				fprintf(stderr, "can't modify acer aspect function\n");
			}
		}

		if (model != PCB800099) {
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
		SetParameter<T_Info>(nIdxNo[X68_15K_P],		0x0F,  640, 240, 159, 615, 5, 5,  760, 262,  98, 20);		// X68000  512x240 15KHz
		SetParameter<T_Info>(nIdxNo[X68_24K_P],		0x0F, 1024, 424, 246, 532, 5, 5, 1408, 465, 282, 23);		// X68000 1024x424 24KHz
		SetParameter<T_Info>(nIdxNo[X68_31K],		0x0F,  768, 512, 314, 554, 5, 5, 1104, 568, 261, 32);		// X68000  768x512 31KHz
		SetParameter<T_Info>(nIdxNo[X68_Memtest],	0x0F,  768, 512, 340, 554, 5, 5, 1130, 613, 320, 41);		// X68000 memtest 31KHz
		SetParameter<T_Info>(nIdxNo[X68_Dash],		0x0F,  768, 536, 315, 543, 5, 5, 1176, 580, 308, 38);		// X68000 ダッシュ野郎
		SetParameter<T_Info>(nIdxNo[FMT_Raiden],	0x0F,  768, 512, 323, 603, 3, 3, 1104, 536, 240, 19);		// TOWNS 雷電伝説
		SetParameter<T_Info>(nIdxNo[M72_RTYPE],		0x0F,  768, 256, 157, 550, 5, 5, 1024, 284, 156, 24);		// R-TYPE基板 15.7KHz/55Hz KAPPY.さん提供
		SetParameter<T_Info>(nIdxNo[FMT_LINUX],		0x0F,  768, 512, 311, 579, 3, 3,  920, 538, 138, 26);		// TOWNS LINUXコンソール プーさん提供
		if (bModify || model == PCB800099 ) {
			// PCB800099(RTD2660/2662)以外は水平同期信号幅のﾁｪｯｸを外さないと下記ﾌﾟﾘｾｯﾄは映らない
			SetParameter<T_Info>(nIdxNo[X68_FZ24K],		0x0F,  640, 448, 245, 524, 5, 5,  944, 469,  64, 10);		// X68000 Fantasy Zone 24KHz		※ModifyFirmwareが通用した場合のみ対応
			SetParameter<T_Info>(nIdxNo[X68_Druaga],	0x0F,  672, 560, 315, 530, 5, 5, 1104, 595, 108, 31);		// X68000 Druaga 31KHz				※ModifyFirmwareが通用した場合のみ対応
			SetParameter<T_Info>(nIdxNo[FMT_SRMP2PS],	0x0F,  736, 480, 320, 609, 3, 3,  896, 525, 144,  4);		// TOWNS スーパーリアル麻雀P2&P3	※ModifyFirmwareが通用した場合のみ対応
			//縦像度240未満の定義はVTotalHeight下限ﾁｪｯｸを除去しないと範囲外ｴﾗｰ表示で映らない
			SetParameter<T_Info>(nIdxNo[GEN_15K_P], 0x00, 0, 0, 0, 0, 5, 5, 0, 0, 0, 0);						// 元からある15.7KHz/Progressive/1440x240pの許容誤差を10->5に変更
			if (model == LHRD56_IPAD97) {	// VTotal 232以下ﾀﾞﾒっぽい
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
				printf("Modified firmware %s writ ok\n", szFilePath);
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
	else if (nMode == ModeDump) {
		strcpy(szFilePath, MakePath(szFilePath, "", ".csv"));
		fpCsv = fopen(szFilePath, "wt");
		if (!fpCsv) {
			fprintf(stderr, "can't open %s\n", szFilePath);
			ret = -2;
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

	ret = model;

L_FREE:
	free(buf);
L_CLOSE_CSV:
	if (fpCsv) fclose(fpCsv);
L_CLOSE:
	fclose(fp);
L_RET:
	return ret;
}


