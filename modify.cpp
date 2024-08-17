#include "stdafx.h"
#include <stdint.h>

#ifndef BYTE
#define	BYTE unsigned char
#endif
#ifndef WORD
#define	WORD unsigned short
#endif

extern BYTE *buf;
extern int nFileLen;

int FindKey(BYTE key[], int nKeyLen, int nStartPos=0, int nEndPos=0)
{
	int nRet = -1;

	if (nEndPos == 0) nEndPos = nFileLen;

	for (int i=nStartPos; i<nEndPos-nKeyLen; i++) {
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
	if (model == P2214H || model == P2314H) {	// P2214H/P2314Hはこちら
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
	else if (model == E1715S) {
		BYTE	keyVHeightCheck[] = {0xC3, 0xE5, 0x59, 0x94, 0xF0};
		nPosVHeightCheck = FindKey(keyVHeightCheck, 5);
		if (nPosVHeightCheck < 0) {
			fprintf(stderr, "keyVHeightCheck not find\n");
			goto L_RET;
		}
		nOfsVHeightCheck = 4;
	}
	else if (model == X2377HS) {
		BYTE	keyVHeightCheck[] = {0xC3, 0xE5, 0x5A, 0x94, 0xF0};
		nPosVHeightCheck = FindKey(keyVHeightCheck, 5);
		if (nPosVHeightCheck < 0) {
			fprintf(stderr, "keyVHeightCheck not find\n");
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
	else if (model == C27M2020DJP) {
		nOffset[0] = 0x3d590;
		nOffset[1] = 0x3d5d1;
	}
	else if (model == KA222Q) {
		nOffset[0] = 0x1d693;
		nOffset[1] = 0x1d6d4;
	}
	else if (model == EK221QE3bi) {
		nOffset[0] = 0x5ce60;
		nOffset[1] = 0x5ce9c;
	}
	else if (model == CB272Ebmiprx) {
		nOffset[0] = 0x7b3e7;
		nOffset[1] = 0x7b428;
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

void OutputISTPTR(enModel model, int &nOffset)
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
	else if (model == C27M2020DJP) {
		buf[nOffset++] = 0x12;	buf[nOffset++] = 0x15;	buf[nOffset++] = 0xCD;
	}
	else if (model == KA222Q) {
		buf[nOffset++] = 0x12;	buf[nOffset++] = 0x12;	buf[nOffset++] = 0xCF;
	}
	else if (model == EK221QE3bi) {
		buf[nOffset++] = 0x12;	buf[nOffset++] = 0x16;	buf[nOffset++] = 0x5E;
	}
	else if (model == LHRD56_IPAD97) {
		buf[nOffset++] = 0x12;	buf[nOffset++] = 0x12;	buf[nOffset++] = 0xB8;
	}
	else if (model == CB272Ebmiprx) {
		buf[nOffset++] = 0x12;	buf[nOffset++] = 0x17;	buf[nOffset++] = 0x24;
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
	else if (model == C27M2020DJP) {
		nOffset = 0x5e6f2;
		nOffsetRet = 0x5e73b;
	}
	else if (model == KA222Q) {
		nOffset = 0x1eff8;
		nOffsetRet = 0x1f041;
	}
	else if (model == EK221QE3bi) {
		nOffset = 0x2f56c;
		nOffsetRet = 0x2f5b3;
	}
	else if (model == CB272Ebmiprx) {
		nOffset = 0x2f147;
		nOffsetRet = 0x2f17f;
	}
	else {
		printf("Invlid model\n");
	}
	return nOffset;
}

bool OutputMovDPTRInputVHeight(enModel model, int &nOffset)
{
	if (model == EK241YEbmix || model == EK271Ebmix || model == EK221QE3bi) {
		buf[nOffset++] = 0x90;	buf[nOffset++] = 0xE3;	buf[nOffset++] = 0xDD;	// MOV DPTR,Input Height
	}
	else if (model == QG221QHbmiix) {
		buf[nOffset++] = 0x90;	buf[nOffset++] = 0xE4;	buf[nOffset++] = 0xC4;	// MOV DPTR,Input Height
	}
	else if (model == C24M2020DJP || model == C27M2020DJP) {
		buf[nOffset++] = 0x90;	buf[nOffset++] = 0xE5;	buf[nOffset++] = 0x75;	// MOV DPTR,Input Height
	}
	else if (model == KA222Q) {
		buf[nOffset++] = 0x90;	buf[nOffset++] = 0xE4;	buf[nOffset++] = 0x1E;	// MOV DPTR,Input Height
	}
	else if (model == LHRD56_IPAD97) {
		buf[nOffset++] = 0x90;	buf[nOffset++] = 0xD8;	buf[nOffset++] = 0xBA;	// MOV DPTR,Input Height
	}
	else if (model == CB272Ebmiprx) {
		buf[nOffset++] = 0x90;	buf[nOffset++] = 0xE4;	buf[nOffset++] = 0x03;	// MOV DPTR,Input Height
	}
	else {
		printf("code not implemented\n");
		return false;
	}

	return true;
}

// 指定オフセット位置にAspect指定時の処理関数を設定
bool SetAcerWideModeFunction(enMode mode, enModel model, int &nOffset, int nOffsetRet, bool bBackupR1R2)
{
	int nJmpOffsetRetPos = -1, nJmpOffsetSetAspect640x480 = -1;

	if (bBackupR1R2) {
		// R1,R2を退避
		buf[nOffset++] = 0xE9;													// MOV A,R1
		buf[nOffset++] = 0xFC;													// MOV R4,A
		buf[nOffset++] = 0xEA;													// MOV A,R2
		buf[nOffset++] = 0xFD;													// MOV R5,A
	}

	// Input Width
	if (model == EK241YEbmix || model == EK271Ebmix || model == EK221QE3bi) {
		buf[nOffset++] = 0x90;	buf[nOffset++] = 0xE3;	buf[nOffset++] = 0xD1;	// MOV DPTR,Input Width
	}
	else if (model == QG221QHbmiix) {
		buf[nOffset++] = 0x90;	buf[nOffset++] = 0xE4;	buf[nOffset++] = 0xB8;	// MOV DPTR,Input Width
	}
	else if (model == C24M2020DJP || model == C27M2020DJP) {
		buf[nOffset++] = 0x90;	buf[nOffset++] = 0xE5;	buf[nOffset++] = 0x69;	// MOV DPTR,Input Width
	}
	else if (model == KA222Q) {
		buf[nOffset++] = 0x90;	buf[nOffset++] = 0xE4;	buf[nOffset++] = 0x14;	// MOV DPTR,Input Width
	}
	else if (model == LHRD56_IPAD97) {
		buf[nOffset++] = 0x90;	buf[nOffset++] = 0xD8;	buf[nOffset++] = 0xB0;	// MOV DPTR,Input Width
	}
	else if (model == CB272Ebmiprx) {
		buf[nOffset++] = 0x90;	buf[nOffset++] = 0xE3;	buf[nOffset++] = 0xF7;	// MOV DPTR,Input Width
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
	OutputISTPTR(model, nOffset);												// LCALL ISTPTR

	// Input Height
	if (!OutputMovDPTRInputVHeight(model, nOffset)) {							// MOV DPTR,Input Height
		return false;
	}
	// Set Input Height to R1/R7
	buf[nOffset++] = 0xE0;														// MOVX A,@DPTR
	buf[nOffset++] = 0xF9;														// MOV R1,A
	buf[nOffset++] = 0xA3;														// INC DPTR
	buf[nOffset++] = 0xE0;														// MOVX A,@DPTR
	buf[nOffset++] = 0xFF;														// MOV R7,A

	// AddAspectModeForAcerの場合のみ1024x424を4:3として処理
	if (mode == ModeModifyExp) {
		// Check VHeight 424
		buf[nOffset++] = 0xB9;	buf[nOffset++] = 0x01;	buf[nOffset++] = 0x05;	// CJNE	 R1,#0x01,0x05
		buf[nOffset++] = 0xBF;	buf[nOffset++] = 0xA8;	buf[nOffset++] = 0x02;	// CJNE	 R7,#0xA8,0x02
		buf[nOffset++] = 0x80;	buf[nOffset++] = 0x00;							// SJMP	 Set Aspect 640x480
		nJmpOffsetSetAspect640x480 = nOffset-1;
	}

	// Check Interlace Flag?
	if (model == EK241YEbmix || model == EK271Ebmix || model == EK221QE3bi) {
		buf[nOffset++] = 0x90;	buf[nOffset++] = 0xE3;	buf[nOffset++] = 0x65;	// MOV DPTR,Interlace Flag
	}
	else  if (model == QG221QHbmiix) {
		buf[nOffset++] = 0x90;	buf[nOffset++] = 0xE4;	buf[nOffset++] = 0x4C;	// MOV DPTR,Interlace Flag
	}
	else  if (model == C24M2020DJP || model == C27M2020DJP) {
		buf[nOffset++] = 0x90;	buf[nOffset++] = 0xE5;	buf[nOffset++] = 0x35;	// MOV DPTR,Interlace Flag
	}
	else  if (model == KA222Q) {
		buf[nOffset++] = 0x90;	buf[nOffset++] = 0xE3;	buf[nOffset++] = 0xE6;	// MOV DPTR,Interlace Flag
	}
	else  if (model == LHRD56_IPAD97) {
		buf[nOffset++] = 0x90;	buf[nOffset++] = 0xD8;	buf[nOffset++] = 0xAB;	// MOV DPTR,Interlace Flag
	}
	else  if (model == CB272Ebmiprx) {
		buf[nOffset++] = 0x90;	buf[nOffset++] = 0xE3;	buf[nOffset++] = 0x8D;	// MOV DPTR,Interlace Flag
	}
	else {
		printf("code not implemented\n");
		return false;
	}
	buf[nOffset++] = 0xE0;														// MOVX A,@DPTR
	if (mode == ModeModifyExp || 
		(model == QG221QHbmiix || model == C24M2020DJP || model == C27M2020DJP || model == KA222Q || model == LHRD56_IPAD97 || model == CB272Ebmiprx)) {
		// CheckInterlaceFlagが展開されている
		buf[nOffset++] = 0x13;													// RRC
		buf[nOffset++] = 0x13;													// RRC
		buf[nOffset++] = 0x54;	buf[nOffset++] = 0x3F;							// ANL A,#0x3F
		buf[nOffset++] = 0x30;	buf[nOffset++] = 0xE0;	buf[nOffset++] = 0x07;	// JNB Check VHeight < 350
		goto L_x2Height;
	}
	else if (model == EK241YEbmix) {
		buf[nOffset++] = 0x12;	buf[nOffset++] = 0xE0;	buf[nOffset++] = 0xF5;	// LCALL CheckInterlaceFlag
	}
	else if (model == EK271Ebmix) {
		buf[nOffset++] = 0x12;	buf[nOffset++] = 0xE7;	buf[nOffset++] = 0x9B;	// LCALL CheckInterlaceFlag
	}
	else if (model == EK221QE3bi) {
		buf[nOffset++] = 0x12;	buf[nOffset++] = 0xE9;	buf[nOffset++] = 0x31;	// LCALL CheckInterlaceFlag
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
	if (mode == ModeModify4x3) {
		// Force 4:3 Aspect Ratio
		buf[nOffset++] = 0x00;													// NOP
		buf[nOffset++] = 0x00;													// NOP 
	}
	else {
		// Use Original Aspect Ratio
		buf[nOffset++] = 0x50;	buf[nOffset++] = nOffsetRet-nOffset-1;			// JNC nOffsetRet
		nJmpOffsetRetPos = nOffset-1;	// nOffsetRetが0の場合、最後に設定する
	}

	// Set Aspect 640x480(4:3)
	if (0 < nJmpOffsetSetAspect640x480) {
		buf[nJmpOffsetSetAspect640x480] = nOffset - nJmpOffsetSetAspect640x480 - 1;
	}
	buf[nOffset++] = 0xEC;														// MOV A,R4
	buf[nOffset++] = 0xF9;														// MOV R1,A
	buf[nOffset++] = 0xED;														// MOV A,R5
	buf[nOffset++] = 0xFA;														// MOV R2,A
	buf[nOffset++] = 0x74;	buf[nOffset++] = 0x02;								// MOV A,#0x02
	buf[nOffset++] = 0x75;	buf[nOffset++] = 0xF0;	buf[nOffset++] = 0x80;		// MOV B,#0x80
	OutputISTPTR(model, nOffset);												// LCALL ISTPTR
	buf[nOffset++] = 0x74;	buf[nOffset++] = 0xE0;								// MOV A,#0xE0
	buf[nOffset++] = 0xFF;														// MOV R7,A
	buf[nOffset++] = 0x74;	buf[nOffset++] = 0x01;								// MOV A,#0x01
	buf[nOffset++] = 0xF9;														// MOV R1,A

	if (0 < nJmpOffsetRetPos && nOffsetRet == 0) {
		buf[nJmpOffsetRetPos] = nOffset - nJmpOffsetRetPos - 1;
	}

	return true;
}

// AcerのWide ModeメニューでAspect選択時の挙動を改造
// mode==ModeModifyの場合、縦解像度が350未満の場合のみ強制4:3 それ以外は元の解像度比率を使用
//       X68000の1024x848/1024x424モードでNot Supportになる不具合あり
//       他にも縦横比が異常なプリセットで影響出る可能性あり
// mode==ModeModify4x3の場合は常に4:3
bool ModifyAcerWideModeFunction(enMode mode, enModel model)
{
	int nOffsetRet;
	int nOffset = GetAspectFunctionOffset(model, nOffsetRet);
	if (nOffset < 0) {
		return false;
	}

	if (!SetAcerWideModeFunction(mode, model, nOffset, nOffsetRet, true)) {
		return false;
	}

	for ( ; nOffset<nOffsetRet; nOffset++) buf[nOffset] = 0x00;					// NOP

	printf("Modified acer wide mode function %s aspect ratio\n", mode == ModeModify4x3 ? "force 4:3" : "original" );

	return true;
}

#ifndef WIN32
#define	_MAX_PATH	256
size_t _filelength(int filedes)
{
    off_t pos = lseek(filedes, 0, SEEK_CUR);
    if (pos != (off_t)-1)
    {
        off_t size = lseek(filedes, 0, SEEK_END);
        lseek(filedes, pos, SEEK_SET);
        return (size_t)size;
    }
    return (off_t)-1;
}
#endif

// 青ジャックiPad液晶など、Aspectモードを持たない液晶にてアスペクト比4:3選択時、
// Acer液晶のAspect改造と同じ機能を追加する(実験中)
// 将来的にはCocopar/252B9も対応予定
bool AddAspectMode(enMode mode, enModel model)
{
	int nOffset = 0x4BA00;
	int nJMP4x3Pos, nJMP5x4Pos, nJMPRetPos, nSJMPISTPTRPos1, nSJMPISTPTRPos2;

	// アスペクト比取得関数呼び出し先変更
	if (model == LHRD56_IPAD97) {
		buf[0x009E6] = 0xBA;
		buf[0x009E7] = 0x00;
	}
	else {
		printf("code not implemented\n");
		return false;
	}

	// 引数1のｱﾄﾞﾚｽ？を退避 R3の値0x01	
	buf[nOffset++] = 0xE9;														// MOV A,R1
	buf[nOffset++] = 0xFC;														// MOV R4,A
	buf[nOffset++] = 0xEA;														// MOV A,R2
	buf[nOffset++] = 0xFD;														// MOV R5,A

	// R3,R2,R1を0xD716に退避
	if (model == LHRD56_IPAD97) {
		buf[nOffset++]=0x90;	buf[nOffset++]=0xd7;	buf[nOffset++]=0x16;		// MOV	DPTR,#0xd716	
	}
	else {
		printf("code not implemented\n");
		return false;
	}
	buf[nOffset++]=0xeb;														// MOV	A,R3
	buf[nOffset++]=0xf0;														// MOVX	@DPTR=>DAT_EXTMEM_d716,A
	buf[nOffset++]=0xa3;														// INC	DPTR
	buf[nOffset++]=0xea;														// MOV	A,R2
	buf[nOffset++]=0xf0;														// MOVX	@DPTR=>DAT_EXTMEM_d717,A
	buf[nOffset++]=0xa3;														// INC	DPTR
	buf[nOffset++]=0xe9;														// MOV	A,R1
	buf[nOffset++]=0xf0;														// MOVX	@DPTR=>DAT_EXTMEM_d718,A

	if (model == LHRD56_IPAD97) {
		buf[nOffset++] = 0x90;	buf[nOffset++] = 0xD8;	buf[nOffset++] = 0x48;		// MOV DPTR,#0xD848 1=4:3 2=16:9 3=5:4
	}
	else {
		printf("code not implemented\n");
		return false;
	}
	buf[nOffset++] = 0xE0;														// MOV A,DPTR
	buf[nOffset++] = 0x54;	buf[nOffset++] = 0x07;								// ANL A,#0x7
	buf[nOffset++] = 0x14;														// DEC A

	buf[nOffset++]=0x60;	buf[nOffset++]=0x00;								// JZ	LAB_4x3
	nJMP4x3Pos = nOffset-1;
	buf[nOffset++]=0x24;	buf[nOffset++]=0xfe;								// ADD	A,#0xfe	
	buf[nOffset++]=0x60;	buf[nOffset++]=0x00;								// JZ	LAB_5x4	
	nJMP5x4Pos = nOffset-1;
	buf[nOffset++]=0x04;														// INC	A	
	buf[nOffset++]=0x70;	buf[nOffset++]=0x00;								// JNZ	LAB_RET	
	nJMPRetPos = nOffset-1;
	// Set 16:9
	buf[nOffset++]=0x75;	buf[nOffset++]=0xf0;	buf[nOffset++]=0x10;		// MOV	B,#0x10	
	if (model == LHRD56_IPAD97) {
		buf[nOffset++]=0x12;	buf[nOffset++]=0x73;	buf[nOffset++]=0xfb;		// LCALL	FUN_Bank4__73fb	
	}
	else {
		printf("code not implemented\n");
		return false;
	}
	buf[nOffset++]=0x75;	buf[nOffset++]=0xf0;	buf[nOffset++]=0x09;		// MOV	B,#0x9	
	buf[nOffset++]=0x80;	buf[nOffset++]=0x00;								// SJMP	LAB_ISTPTR	
	nSJMPISTPTRPos1 = nOffset-1;

	// LAB_4x3 Set 4:3
	buf[nJMP4x3Pos] = nOffset-nJMP4x3Pos-1;
#if 0	// Original Code
	buf[nOffset++]=0x75;	buf[nOffset++]=0xf0;	buf[nOffset++]=0x04;		// MOV	B,#0x4	
	buf[nOffset++]=0x12;	buf[nOffset++]=0x73;	buf[nOffset++]=0xfb;		// LCALL	FUN_Bank4__73fb	
	buf[nOffset++]=0x75;	buf[nOffset++]=0xf0;	buf[nOffset++]=0x03;		// MOV	B,#0x3	
	buf[nOffset++]=0x80;	buf[nOffset++]=0x0f;								// SJMP	LAB_ISTPTR	
	nSJMPISTPTRPos2 = nOffset-1;
#else
	if (!SetAcerWideModeFunction(mode, model, nOffset, 0, false)) {					
		return false;
	}
	// set V
	if (model == LHRD56_IPAD97) {
		buf[nOffset++]=0x90;	buf[nOffset++]=0xD7;	buf[nOffset++]=0x19;		// MOV	 DPTR,#0xD719
	}
	else {
		printf("code not implemented\n");
		return false;
	}
	buf[nOffset++]=0xe0;														// MOVX	A,@DPTR
	buf[nOffset++]=0xfb;														// MOV	 R3,A
	buf[nOffset++]=0xa3;														// INC	 DPTR
	buf[nOffset++]=0xe0;														// MOVX	A,@DPTR
	buf[nOffset++]=0xfa;														// MOV	 R2,A
	buf[nOffset++]=0xa3;														// INC	 DPTR
	buf[nOffset++]=0xe0;														// MOVX	A,@DPTR
	buf[nOffset++]=0xc9;														// XCH	 A,R1
	buf[nOffset++]=0x8f;	buf[nOffset++]=0xf0;								// MOV	 B,R7
	buf[nOffset++]=0x80;	buf[nOffset++]=0x00;								// SJMP	LAB_ISTPTR	
	nSJMPISTPTRPos2 = nOffset-1;
#endif

	// LAB_5x4 Set 5:4		
	buf[nJMP5x4Pos] = nOffset-nJMP5x4Pos-1;
	if (model == LHRD56_IPAD97) {
		buf[nOffset++]=0x90;	buf[nOffset++]=0xd7;	buf[nOffset++]=0x16;		// MOV	DPTR,#0xd716	
		buf[nOffset++]=0x12;	buf[nOffset++]=0x74;	buf[nOffset++]=0x0d;		// LCALL	FUN_Bank4__740d	
	}
	else {
		printf("code not implemented\n");
		return false;
	}
	buf[nOffset++]=0x75;	buf[nOffset++]=0xf0;	buf[nOffset++]=0x05;		// MOV	B,#0x5	
	if (model == LHRD56_IPAD97) {
		buf[nOffset++]=0x12;	buf[nOffset++]=0x74;	buf[nOffset++]=0x07;		// LCALL	FUN_Bank4__7407	
	}
	else {
		printf("code not implemented\n");
		return false;
	}
	buf[nOffset++]=0x75;	buf[nOffset++]=0xf0;	buf[nOffset++]=0x04;		// MOV	B,#0x4	

	// LAB_ISTPTR
	buf[nSJMPISTPTRPos1] = nOffset-nSJMPISTPTRPos1-1;
	buf[nSJMPISTPTRPos2] = nOffset-nSJMPISTPTRPos2-1;
	OutputISTPTR(model, nOffset);												// LCALL ISTPTR
	
	// LAB_RET
	buf[nJMPRetPos] = nOffset-nJMPRetPos-1;
	buf[nOffset++]=0x22;														// RET		

	printf("Add %s aspect ratio mode\n", mode == ModeModify4x3 ? "force 4:3" : "original" );

	return true;
}

// 空き領域にアスペクト比取得関数を追加し、そちらを呼び出すようにする(実験中)
// CB272Ebmiprxは元の領域サイズが小さく書き換えできないのでこちらを使用
// X68000の1024x424は4:3表示することで対応
bool AddAspectModeForAcer(enMode mode, enModel model)
{
	int nOffset = 0x7F300, nCallOffset;
	BYTE	bAddr[2];

	// アスペクト比取得関数呼び出し先変更
	if (model == CB272Ebmiprx) {
		nOffset = 0x7F300;
		nCallOffset = 0x30AA0;
		// Bank7
		buf[nCallOffset+3] = 0x02;	buf[nCallOffset+4] = 0x9B;
	}
	else if (model == EK221QE3bi) {
		nOffset = 0x6FC00;
		nCallOffset = 0x309C0;
		// Bank6
		buf[nCallOffset+3] = 0x04;	buf[nCallOffset+4] = 0x55;
	}
	else if (model == EK241YEbmix) {
		nOffset = 0x6FC00;
		nCallOffset = 0x30A6E;
		// Bank6
		buf[nCallOffset+3] = 0x04;	buf[nCallOffset+4] = 0x55;
	}
	else if (model == EK271Ebmix) {
		nOffset = 0x6FC00;
		nCallOffset = 0x30A4A;
		// Bank6
		buf[nCallOffset+3] = 0x04;	buf[nCallOffset+4] = 0x55;
	}
	else if (model == KA222Q) {
		nOffset = 0x5FC00;
		nCallOffset = 0x306D5;
		// Bank5
		buf[nCallOffset+3] = 0x02;	buf[nCallOffset+4] = 0x00;
	}
	else if (model == QG221QHbmiix) {
		nOffset = 0x6FC00;
		nCallOffset = 0x30AFB;
		// Bank6
		buf[nCallOffset+3] = 0x04;	buf[nCallOffset+4] = 0xA0;
	}
	else if (model == C24M2020DJP) {
		nOffset = 0x6FC00;
		nCallOffset = 0x20A0F;
		// Bank6
		buf[nCallOffset+3] = 0x02;	buf[nCallOffset+4] = 0x94;
	}
	else if (model == C27M2020DJP) {
		nOffset = 0x6FC00;
		nCallOffset = 0x20A09;
		// Bank6
		buf[nCallOffset+3] = 0x02;	buf[nCallOffset+4] = 0x94;
	}
	else {
		printf("code not implemented\n");
		return false;
	}

	memcpy(bAddr, &nOffset, sizeof(bAddr));
	buf[nCallOffset  ] = bAddr[1];
	buf[nCallOffset+1] = bAddr[0];

	if (!SetAcerWideModeFunction(mode, model, nOffset, 0, true)) {					
		return false;
	}
	// 退避していた第2引数用情報をR2/R3/R7に復元？
	if (model == CB272Ebmiprx) {
		buf[nOffset++]=0x90;	buf[nOffset++]=0xE1;	buf[nOffset++]=0xB5;	// MOV	 DPTR,#0xE1B5
	}
	else if (model == EK221QE3bi || model == EK241YEbmix || model == EK271Ebmix) {
		buf[nOffset++]=0x90;	buf[nOffset++]=0xE0;	buf[nOffset++]=0x88;	// MOV	 DPTR,#0xE088
	}
	else if (model == KA222Q) {
		buf[nOffset++]=0x90;	buf[nOffset++]=0xE2;	buf[nOffset++]=0x87;	// MOV	 DPTR,#0xE287
	}
	else if (model == QG221QHbmiix) {
		buf[nOffset++]=0x90;	buf[nOffset++]=0xE3;	buf[nOffset++]=0x08;	// MOV	 DPTR,#0xE308
	}
	else if (model == C24M2020DJP || model == C27M2020DJP) {
		buf[nOffset++]=0x90;	buf[nOffset++]=0xE3;	buf[nOffset++]=0x35;	// MOV	 DPTR,#0xE335
	}
	else {
		printf("code not implemented\n");
		return false;
	}
	buf[nOffset++]=0xe0;														// MOVX	A,@DPTR
	buf[nOffset++]=0xfb;														// MOV	 R3,A
	buf[nOffset++]=0xa3;														// INC	 DPTR
	buf[nOffset++]=0xe0;														// MOVX	A,@DPTR
	buf[nOffset++]=0xfa;														// MOV	 R2,A
	buf[nOffset++]=0xa3;														// INC	 DPTR
	buf[nOffset++]=0xe0;														// MOVX	A,@DPTR
	buf[nOffset++]=0xc9;														// XCH	 A,R1
	buf[nOffset++]=0x8f;	buf[nOffset++]=0xf0;								// MOV	 B,R7
	OutputISTPTR(model, nOffset);												// LCALL ISTPTR
	
	// LAB_RET
	buf[nOffset++]=0x22;														// RET		

	printf("Add %s aspect ratio func\n", mode == ModeModify4x3 ? "force 4:3" : "original" );

	return true;
}

// Dell P2214H/P2314Hのアスペクト比5:4選択時、Acer液晶のAspect改造と同様解像度でアスペクト比表示する(実験中)
// おそらく使っていないコード領域とおそらく未使用のメモリを使用して実装しているが動作が不安定になる可能性あり
bool AddAspectModeForDell(enMode mode, enModel model)
{
	char	szBinName[_MAX_PATH];
	int		nFuncOffset, nCallOffset;
	BYTE	bAddr[2];
	if (model == P2214H) {
		nFuncOffset = 0x1FA00;
		nCallOffset = 0x1F9B1;
		strcpy(szBinName, "p2214aspect.bin");
	}
	else if (model == P2314H) {
		nFuncOffset = 0x1FA00;
		nCallOffset = 0x1CFFC;
		strcpy(szBinName, "p2314aspect.bin");
	}
	else {
		printf("Invlid model\n");
		return false;
	}

	FILE *fp = fopen(szBinName, "rb");
	if (!fp) {
		printf("can't open %s\n", szBinName);
		return false;
	}
	int size = _filelength(fileno(fp));
	fread(&buf[nFuncOffset], size, 1, fp);
	fclose(fp);

	memcpy(bAddr, &nFuncOffset, sizeof(bAddr));
	buf[nCallOffset  ] = bAddr[1];
	buf[nCallOffset+1] = bAddr[0];

	printf("Add Aspect Mode For Dell Done\n");
	return true;
}
