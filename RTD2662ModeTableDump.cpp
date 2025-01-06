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
	X68_FZ31K,		// X68000 Fantasy Zone 31KHz
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

bool ModifyFirmware(enModel model);
bool DisableAcerAspectChangeCheck(enModel model);
bool ModifyAcerWideModeFunction(enMode mode, enModel model);
bool AddAspectMode(enMode mode, enModel model);
bool AddAspectModeForAcer(enMode mode, enModel model);
bool AddAspectModeForDell(enMode mode, enModel model);

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

void DellCharConv(const char *path)
{
	const char CharTbl[256] = {
#if 0	// RTD2556 LCD Controller Board
	//  0    1    2    3    4    5    6    7    8    9    a    b    c    d    e    f
		0  , '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E',	// 00
		'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U',	// 10
		'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k',	// 20
		'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 0  ,	// 30
		0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  ,	// 40
		0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , '-', 0  , 0  , 0  , 0  ,	// 50
		0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  ,	// 60
		0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  ,	// 70
#else	// DELL P2314H
	//  0    1    2    3    4    5    6    7    8    9    a    b    c    d    e    f
		0  , '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', ' ', 'C', 'D', 'E',	// 00
		'F', '-', '.', 'I', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 0  , 0  ,	// 10
		0  , 0  , 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',	// 20
		'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd',	// 30
		'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',	// 40
		'u', 'v', 'w', 'x', 'y', 'z', 0  , 0  , 0  , 0  , 0  , '-', 0  , 0  , 0  , 0  ,	// 50
		0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  ,	// 60
		0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  ,	// 70
#endif
	};
	char	RevCharTbl[256];
	int i, nStart = 0, nOffset = 0, nSize, nStrLen;
	bool bHit = false;
	char pattern[16];

	/*
	for (i=0; i<256; i++) RevCharTbl[CharTbl[i]] = i;


	memset(pattern, 0x00, sizeof(pattern));
	nStrLen = strlen(str);
	if (sizeof(pattern) < nStrLen) return;
	for (i=0; i<nStrLen; i++) pattern[i] = RevCharTbl[str[i]];
	for (i=0; i<nFileLen-nStrLen; i++) {
		if (memcmp(&buf[i], pattern, nStrLen) == 0) {
		}
	}
	//nStart = 0x345DA;
	nStart = 0x4D98F;	// P2314H Model Name & Firmware Code
	nSize = 32;
	//nStart = 0x42000;	// P2314H Language
	//nSize = 0x4400;
	for (int i=nStart; i<nStart+nSize; i++) {
		//int nChar = 'A'+buf[i]-nOffset;
		int nChar = CharTbl[buf[i]];
		if (isascii(nChar)) printf("%c", nChar);
	}
	*/
#ifdef WIN32
	char *outbuf = (char *)malloc(nFileLen);
	for (i=0; i<nFileLen; i++) outbuf[i] = CharTbl[buf[i]];
	char outpath[1024], szDrive[256], szDir[256], szFName[256], szExt[256];
	_splitpath(path, szDrive, szDir, szFName, szExt);
	strcat(szFName, "_charconv");
	_makepath(outpath, szDrive, szDir, szFName, szExt);
	FILE *fp = fopen(outpath, "wb");
	fwrite(outbuf, nFileLen, 1, fp);
	fclose(fp);
	free(outbuf);
#endif
}

//! Dell 機種判別
enModel JudgeDellModel()
{
	BYTE	patE1715S_xxxxx[] = {0x25, 0xE0, 0x24, 0x66, 0xFF, 0x12, 0xDC, 0x9A, 0x7B, 0x12, 0x7D, 0x15, 0x12, 0x06, 0xA7, 0x12, 0xDC, 0xA8, 0x80, 0xBD, 0xE4, 0x90, 0xF9, 0x08, 0xF0, 0x12, 0xDC, 0xCD, 0x94, 0x06, 0x40, 0x03, 0x02};
	BYTE	patP2214H_P72WF[] = {0x25, 0x40, 0x47, 0x47, 0x0C, 0x31, 0x16, 0x16, 0x15, 0x18, 0x29, 0xFF, 0x2E, 0x17, 0x23, 0x15, 0x14, 0x15, 0xFF, 0x1D, 0x20, 0x18, 0x00, 0x08, 0x09, 0x08, 0xFF, 0x39, 0x39, 0xFF, 0x39, 0x39, 0xFF};
	BYTE	patP2314H_79H3D[] = {0x25, 0x40, 0x47, 0x47, 0x0C, 0x31, 0x16, 0x17, 0x15, 0x18, 0x29, 0xFF, 0x2E, 0x17, 0x35, 0x15, 0x14, 0x15, 0xFF, 0x34, 0x37, 0x2F, 0x0C, 0x1B, 0x1B, 0x19, 0xFF, 0x1B, 0x1D, 0x29, 0x17, 0x25, 0xFF};
	BYTE	patP2314H_48H1R[] = {0x25, 0x40, 0x47, 0x47, 0x0C, 0x31, 0x16, 0x17, 0x15, 0x18, 0x29, 0xFF, 0x2E, 0x17, 0x35, 0x15, 0x14, 0x15, 0xFF, 0x34, 0x37, 0x2F, 0x0C, 0x1B, 0x1B, 0x19, 0xFF, 0x18, 0x1C, 0x29, 0x15, 0x33, 0xFF};
	BYTE	patP2314H_9R54N[] = {0x25, 0x40, 0x47, 0x47, 0x0C, 0x31, 0x16, 0x17, 0x15, 0x18, 0x29, 0xFF, 0x2E, 0x17, 0x35, 0x15, 0x14, 0x15, 0xFF, 0x34, 0x37, 0x2F, 0x0C, 0x1B, 0x1B, 0x19, 0xFF, 0x1D, 0x33, 0x19, 0x18, 0x2F, 0xFF};

	int nOffset = 0x4D98F;
	enModel nModel = UNKNOWN;
	if (memcmp(&buf[nOffset], patE1715S_xxxxx, sizeof(patE1715S_xxxxx)) ==0) {
		nModel = E1715S;
		printf("DELL E1715S\n");
	}
	else if (memcmp(&buf[nOffset], patP2214H_P72WF, sizeof(patP2214H_P72WF)) ==0) {
		nModel = P2214H_P72WF;
		printf("DELL P2214H P72WF\n");
	}
	else if (memcmp(&buf[nOffset], patP2314H_48H1R, sizeof(patP2314H_48H1R)) ==0) {
		nModel = P2314H_48H1R;
		printf("DELL P2314H 48H1R\n");
	}
	else if (memcmp(&buf[nOffset], patP2314H_79H3D, sizeof(patP2314H_79H3D)) ==0) {
		nModel = P2314H_79H3D;
		printf("DELL P2314H 79H3D\n");
	}
	else if (memcmp(&buf[nOffset], patP2314H_9R54N, sizeof(patP2314H_9R54N)) ==0) {
		nModel = P2314H_9R54N;
		printf("DELL P2314H 9R54N\n");
	}
	if (nModel == UNKNOWN) {
		nOffset = 0x4DB8C;
		if (memcmp(&buf[nOffset], patP2314H_48H1R, sizeof(patP2314H_48H1R)) ==0) {
			nModel = P2314H_48H1R_A01;
			printf("DELL P2314H 48H1R A01\n");
		}
		else if (memcmp(&buf[nOffset], patP2314H_79H3D, sizeof(patP2314H_79H3D)) ==0) {
			nModel = P2314H_79H3D_B01;
			printf("DELL P2314H 79H3D B01\n");
		}
	}

	return nModel;
}

bool IsAcerModel(
enMode		nMode, 
enModel		model
)
{
	bool bRet = false;

	if (nMode == ModeModify && 
			(model == EK271Ebmix || model == EK271Ebmix_2 || model == EK271Ebmix_3 || 
			 model == EK241YEbmix || model == EK241YEbmix_2 || 
			 model == QG221QHbmiix || model == QG271Ebmiix || 
			 model == C24M2020DJP || model == C27M2020DJP || 
			 model == KA222Q || model == KA222Q_2 || 
			 model == EK221QE3bi)) {
		bRet = true;
	}
	else if (nMode == ModeModifyExp && 
				 (model == EK271Ebmix || model == EK271Ebmix_2 || model == EK271Ebmix_3 || 
				  model == EK241YEbmix || model == EK241YEbmix_2 || 
				  model == QG221QHbmiix || model == QG271Ebmiix ||
	 			  model == C24M2020DJP || model == C27M2020DJP || 
				  model == KA222Q || model == KA222Q_2 || 
				  model == EK221QE3bi || model == CB242YEbmiprx || model == CB272Ebmiprx)) {
		bRet = true;
	}
	
	return bRet;
}


bool IsDellP2x14H(
enModel		model
)
{
	bool bRet = false;

	if (model == P2214H_P72WF || 
		model == P2314H_48H1R || model == P2314H_48H1R_A01 || 
		model == P2314H_79H3D || model == P2314H_79H3D_B01 || model == P2314H_9R54N) {
		bRet = true;
	}
	
	return bRet;
}

int RTD2662ModeTableDump(
const char	*szPath,	//!< i	:
enMode		nMode		//!< i	:0=Dump 1=Modify 2=Modify4x3 -1=CheckOnly
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

	ret = 0;

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
	nIdxNo[X68_FZ31K] = 43;		// 43:1152x900 61.8KHz/66Hz
	nIdxNo[GEN_15K_P] = 87;		// 87:1440x240 15.7KHz/60Hz

	if (model == UNKNOWN) {
		// ﾓﾃﾞﾙ自動判定 ModeTable開始位置から判定 中華液晶基板ではﾌｧｰﾑｳｪｱが頻繁に変わるからあまり意味なし
		switch (nModeTableStart) {
		case 0x200A:	// P2214H/P2314H/E1715S
			model = JudgeDellModel();
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
			nIdxNo[X68_FZ31K] = 42;		// 42:1152x900 61.8KHz/66Hz
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
			nIdxNo[X68_FZ31K] = 42;		// 42:1152x900 61.8KHz/66Hz
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
			nIdxNo[X68_FZ31K] = 42;		// 42:1152x900 61.8KHz/66Hz
			nIdxNo[GEN_15K_P] = 86;		// 86:1440x240 15.7KHz/60Hz
			break;
		case 0x22386:
			printf("LH-RD56(V+H) Light Blue Jack iPad 9.7(poo)\n");
			model = LHRD56_IPAD97_POO;
			// プリセットテーブルはP2314Hとほぼ同じ
			break;
		case 0x23230:
			printf("RTD2513A (ponzu)\n");
			model = RTD2513A;
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
		case 0x42734:	// WIMAXIT FW:V001  楊手令 氏
			printf("WIMAXIT FW:V001\n");
			model = WIMAXIT;
			// プリセットテーブルはP2314Hとほぼ同じ
			break;
		case 0x52A4A:	// Acer EK271Ebmix KAPPY.氏
			printf("Acer EK271Ebmix\n");
			model = EK271Ebmix;
			// プリセットテーブルはP2314Hとほぼ同じ
			break;
		case 0x52C48:	// Acer EK271Ebmix_2 しげ氏
			printf("Acer EK271Ebmix_2\n");
			model = EK271Ebmix_2;
			break;
		case 0x52A15:	// Acer EK271Ebmix_3 Shuripon 氏
			printf("Acer EK271Ebmix_3\n");
			model = EK271Ebmix_3;
			break;
		case 0x42A55:	// Acer EK241YEbmix KAPPY.氏
			printf("Acer EK241YEbmix\n");
			model = EK241YEbmix;
			// プリセットテーブルはP2314Hとほぼ同じ
			break;
		case 0x32356:	// Acer EK241YEbmix_2 しげ氏
			printf("Acer EK241YEbmix_2\n");
			model = EK241YEbmix_2;
			// プリセットテーブルはP2314Hとほぼ同じ
			break;
		case 0x45DCD:	// Acer QG271Ebmiix てまりあ氏
			model = QG271Ebmiix;
			printf("Acer QG271Ebmiix\n");
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
		case 0x227BC:	// Amazon C27M2020DJP みゆ氏
			printf("Amazon C27M2020DJP\n");
			model = C27M2020DJP;
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
		case 0x33F10:	// BenQ BL912 Higgy69氏
			printf("BenQ BL912\n");
			model = BL912;
			// プリセットテーブルはP2314Hとほぼ同じ
			break;
		case 0x4239D:	// Acer KA222Q_2 SATOSHI氏
			printf("Acer KA222Q_2\n");
			model = KA222Q_2;
			// プリセットテーブルはP2314Hとほぼ同じ
			break;
		case 0x528EC:	// Acer EK221QE3bi tomo_retro氏
			printf("Acer EK221QE3bi\n");
			model = EK221QE3bi;
			// プリセットテーブルはP2314Hとほぼ同じ
			break;
		case 0x2313C:	// Cocoper 13.3 イカ先生氏
			printf("Cocoper 13.3\n");
			model = Cocoper133;
			break;
		case 0x00050:	// iiyama X2377HS KAPPY.氏
			printf("iiyama X2377HS\n");
			model = X2377HS;
			nIdxNo[X68_FZ24K] = 43;		// 43:1152x900 61.8KHz/66Hz
			break;
		case 0x527F6:	// Acer CB242YEbmiprx
			printf("Acer CB242YEbmiprx\n");
			model = CB242YEbmiprx;
			// プリセットテーブルはP2314Hとほぼ同じ
			break;
		case 0x527E2:	// Acer CB272Ebmiprx AKT氏
			printf("Acer CB272Ebmiprx\n");
			model = CB272Ebmiprx;
			// プリセットテーブルはP2314Hとほぼ同じ
			break;
		}
	}
	if (model == UNKNOWN) {
		fprintf(stderr, "unknown firmware nModeTableStart=%X\n", nModeTableStart);
		ret = -10;

#ifdef _DEBUG
		DellCharConv(szPath);
#endif
	}
	
	if ((nMode == ModeModify || nMode == ModeModify4x3 || nMode == ModeModifyExp) && 
		model != UNKNOWN && model != RTD2668) {

#if 1
		if (nMode == ModeModify && IsAcerModel(nMode, model)) {
			if (DisableAcerAspectChangeCheck(model)) {
				if (!ModifyAcerWideModeFunction(nMode, model)) {
					fprintf(stderr, "Fail modify acer wide mode function\n");
					ret = -11;
					goto L_FREE;
				}
			}
			else {
				fprintf(stderr, "Can't modify acer aspect function\n");
				ret = -12;
			}
		}
		else if (nMode == ModeModifyExp && model == LHRD56_IPAD97) {
			if (!AddAspectMode(nMode, model)) {
				fprintf(stderr, "Fail add aspect mode\n");
				ret = -13;
				goto L_FREE;
			}
		}
		else if (nMode == ModeModifyExp && 
				 (IsDellP2x14H(model) || model == X2377HS)) {
			if (!AddAspectModeForDell(nMode, model)) {
				fprintf(stderr, "Fail add aspect mode for dell\n");
				ret = -14;
				goto L_FREE;
			}
		}
		else if (nMode == ModeModifyExp && IsAcerModel(nMode, model)) {
			if (DisableAcerAspectChangeCheck(model)) {
				if (!AddAspectModeForAcer(nMode, model)) {
					fprintf(stderr, "Fail add aspect mode for acer\n");
					ret = -15;
					goto L_FREE;
				}
			}
			else {
				fprintf(stderr, "Can't modify acer aspect function\n");
				ret = -16;
			}
		}

		if (model != PCB800099 && model != RTD2513A) {
			bModify = ModifyFirmware(model);
			if (bModify) {
				printf("*** modify firmware success ***\n");
			}
			else {
				fprintf(stderr, "can't modify firmware\n");
				ret = -17;
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
		//FantasyZone 31KHz 現状ダッシュ野郎のプリセットが適用されている？
		//SetParameter<T_Info>(nIdxNo[X68_FZ31K],		0x0F,  644, 448, 311, 547, 3, 3, 1100, 568, 261, 90);		// X68000 Fantasy Zone 31KHz
		SetParameter<T_Info>(nIdxNo[X68_FZ31K],		0x0F,  764, 512, 311, 547, 3, 3, 1104, 568, 261, 32);		// X68000  768x512 31KHz
		SetParameter<T_Info>(nIdxNo[FMT_Raiden],	0x0F,  768, 512, 323, 603, 3, 3, 1104, 536, 240, 19);		// TOWNS 雷電伝説
		SetParameter<T_Info>(nIdxNo[M72_RTYPE],		0x0F,  768, 256, 157, 550, 5, 5, 1024, 284, 156, 24);		// R-TYPE基板 15.7KHz/55Hz KAPPY.さん提供
		SetParameter<T_Info>(nIdxNo[FMT_LINUX],		0x0F,  768, 512, 311, 579, 3, 3,  920, 538, 138, 26);		// TOWNS LINUXコンソール プーさん提供

		if (nMode == ModeModifyExp && 
			((IsAcerModel(nMode, model) && model != QG271Ebmiix) || IsDellP2x14H(model) || model == X2377HS)) {	// Acer機/Dell機とX2377HSは1440x240 15kHz/60Hzのプリセットを640x240にしてAspect対応
			printf("H:15KHz/V:60Hz 240p/480i 1440x240->640x240 (Experimental)\n");
			SetParameter<T_Info>(87,		0x0F,  644, 240, 157, 600, 5, 5,  760, 262,  98, 20);		// Generic 240p/480i
			SetParameter<T_Info>(139,		0x0F,  640, 240, 157, 600, 5, 5,  760, 262,  98, 20);		// Generic 240p/480i PS2はこちらが使われるっぽい？
		}
		if (model == V_M56VDA_IPAD97) {	// ウォーリアブレードがどのプリセット使われるか調査用
			SetParameter<T_Info>(86 ,		0x0F,  644, 240, 157, 600,  5,  5,  760, 262,  98, 20);		// 1440x240 15.7KHz 60Hz 262(Generic 240p)
			SetParameter<T_Info>(136, 		0x0F,  648, 240, 157, 600,  5,  5,  760, 262,  98, 20);		// 1440x240 15.7KHz 60Hz 262
			SetParameter<T_Info>(148, 		0x0F,  652, 240, 157, 600,  5,  5,  760, 262,  98, 20);		// 720x240  15.7KHz 60Hz 262
			printf("V_M56VDA_IPAD97 Preset Test\n");
		}
		if (bModify || model == PCB800099 ) {

			// PCB800099(RTD2660/2662)以外は水平同期信号幅のﾁｪｯｸを外さないと下記ﾌﾟﾘｾｯﾄは映らない
			SetParameter<T_Info>(nIdxNo[X68_FZ24K],		0x0F,  640, 448, 245, 524, 5, 5,  944, 469,  64, 10);		// X68000 Fantasy Zone 24KHz		※ModifyFirmwareが通用した場合のみ対応
			SetParameter<T_Info>(nIdxNo[X68_Druaga],	0x0F,  672, 560, 315, 530, 5, 5, 1104, 595, 108, 31);		// X68000 Druaga 31KHz				※ModifyFirmwareが通用した場合のみ対応
			SetParameter<T_Info>(nIdxNo[FMT_SRMP2PS],	0x0F,  736, 480, 320, 609, 3, 3,  896, 525, 144,  4);		// TOWNS スーパーリアル麻雀P2&P3	※ModifyFirmwareが通用した場合のみ対応
			//縦像度240未満の定義はVTotalHeight下限ﾁｪｯｸを除去しないと範囲外ｴﾗｰ表示で映らない
			SetParameter<T_Info>(nIdxNo[GEN_15K_P], 0x00, 0, 0, 0, 0, 5, 5, 0, 0, 0, 0);						// 元からある15.7KHz/Progressive/1440x240pの許容誤差を10->5に変更
			if (model == LHRD56_IPAD97) {	// VTotal 232以下ﾀﾞﾒっぽい
				//buf[0x3E040] = 0xE4;	// DEBUG MODE ENABLE
				//buf[0x4A950] = 0xC3;	// D3->C3
				SetParameter<T_Info>(nIdxNo[MVS], 0x0F, 576, 232, 157, 591, 3, 3, 768, 263, 120, 24);				// MVS基板 15.7KHz/59.1Hz KAPPY.さん提供
				SetParameter<T_Info>(136, 0x00, 0, 0, 0, 0, 5, 5, 0, 0, 0, 0);						// 元からある15.7KHz/Progressive/1440x240pの許容誤差を10->5に変更
				SetParameter<T_Info>(148, 0x00, 0, 0, 0, 0, 5, 5, 0, 0, 0, 0);						// 元からある15.7KHz/Progressive/720x240pの許容誤差を10->5に変更
			}
			else if (model == V_M56VDA_IPAD97) {	// VTotal 240以下ﾀﾞﾒっぽい
				SetParameter<T_Info>(nIdxNo[MVS], 0x0F, 576, 240, 157, 591, 3, 3, 768, 263, 120, 24);				// MVS基板 15.7KHz/59.1Hz KAPPY.さん提供
				printf("V_M56VDA_IPAD97 MVS Setting\n");
			}
			else {
				SetParameter<T_Info>(nIdxNo[MVS], 0x0F, 576, 224, 157, 591, 3, 3, 768, 263, 120, 24);				// MVS基板 15.7KHz/59.1Hz KAPPY.さん提供
			}
		}

		strcpy(szFilePath, MakePath(szFilePath, (nMode == ModeModifyExp) ? "_modexp" : "_mod"));
		fpOut = fopen(szFilePath, "wb");
		if (fpOut) {
			if (fwrite(buf, nFileLen, 1, fpOut)) {
				printf("Modified firmware %s writ ok\n", szFilePath);
			}
			else {
				fprintf(stderr, "can't write %s\n", szFilePath);
				ret = -20;
			}
			fclose(fpOut);
		}
		else {
			fprintf(stderr, "can't open %s\n", szFilePath);
			ret = -21;
		}
	}
	else if (nMode == ModeModify && model == RTD2668) {
		printf("RTD2668 Modify\n");
		SetParameter<T_Info_23>(nIdxNo[X68_15K_P],		0x0F,  640, 240, 159, 615, 5, 5,  760, 262,  98, 20);		// X68000  512x240 15KHz
		SetParameter<T_Info_23>(nIdxNo[X68_24K_P],		0x0F, 1024, 424, 246, 532, 5, 5, 1408, 465, 282, 23);		// X68000 1024x424 24KHz
		SetParameter<T_Info_23>(nIdxNo[X68_31K],		0x0F,  768, 512, 314, 554, 5, 5, 1104, 568, 261, 32);		// X68000  768x512 31KHz
		SetParameter<T_Info_23>(nIdxNo[X68_Memtest],	0x0F,  768, 512, 340, 554, 5, 5, 1130, 613, 320, 41);		// X68000 memtest 31KHz
		SetParameter<T_Info_23>(nIdxNo[X68_Dash],		0x0F,  768, 536, 315, 543, 5, 5, 1176, 580, 308, 38);		// X68000 ダッシュ野郎
		//FantasyZone 31KHz 現状ダッシュ野郎のプリセットが適用されている？
		SetParameter<T_Info_23>(nIdxNo[X68_FZ31K],		0x0F,  644, 448, 311, 547, 3, 3, 1100, 568, 261, 90);		// X68000 Fantasy Zone 31KHz
		//SetParameter<T_Info_23>(nIdxNo[X68_FZ31K],		0x0F,  764, 512, 311, 547, 3, 3, 1104, 568, 261, 32);		// X68000  768x512 31KHz
		SetParameter<T_Info_23>(nIdxNo[FMT_Raiden],	0x0F,  768, 512, 323, 603, 3, 3, 1104, 536, 240, 19);		// TOWNS 雷電伝説
		SetParameter<T_Info_23>(nIdxNo[M72_RTYPE],		0x0F,  768, 256, 157, 550, 5, 5, 1024, 284, 156, 24);		// R-TYPE基板 15.7KHz/55Hz KAPPY.さん提供
		SetParameter<T_Info_23>(nIdxNo[FMT_LINUX],		0x0F,  768, 512, 311, 579, 3, 3,  920, 538, 138, 26);		// TOWNS LINUXコンソール プーさん提供
		SetParameter<T_Info_23>(nIdxNo[X68_FZ24K],		0x0F,  640, 448, 241, 516, 5, 5,  944, 469,  64, 10);		// X68000 Fantasy Zone 24KHz		※ModifyFirmwareが通用した場合のみ対応
		SetParameter<T_Info_23>(nIdxNo[X68_Druaga],	0x0F,  672, 560, 315, 530, 5, 5, 1104, 595, 108, 31);		// X68000 Druaga 31KHz				※ModifyFirmwareが通用した場合のみ対応
		SetParameter<T_Info_23>(nIdxNo[FMT_SRMP2PS],	0x0F,  736, 480, 320, 609, 3, 3,  896, 525, 144,  4);		// TOWNS スーパーリアル麻雀P2&P3	※ModifyFirmwareが通用した場合のみ対応

		strcpy(szFilePath, MakePath(szFilePath, (nMode == ModeModifyExp) ? "_modexp" : "_mod"));
		fpOut = fopen(szFilePath, "wb");
		if (fpOut) {
			if (fwrite(buf, nFileLen, 1, fpOut)) {
				printf("Modified firmware %s writ ok\n", szFilePath);
			}
			else {
				fprintf(stderr, "can't write %s\n", szFilePath);
				ret = -20;
			}
			fclose(fpOut);
		}
		else {
			fprintf(stderr, "can't open %s\n", szFilePath);
			ret = -21;
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

L_FREE:
	free(buf);
L_CLOSE_CSV:
	if (fpCsv) fclose(fpCsv);
L_CLOSE:
	fclose(fp);
L_RET:
	return ret;
}


