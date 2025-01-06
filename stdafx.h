#pragma once

#ifdef WIN32
#include "targetver.h"

#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>

#ifdef WIN32
#include <WinSock.h>
#include <io.h>
#else
#include <arpa/inet.h>
#include <libgen.h>
#include <fcntl.h>
#include <unistd.h>
#endif


enum enModel
{
	UNKNOWN = 0,
	RTD2668,			// RTD2668使用基板(aitendo?)
	PCB800099,			// RTD2662使用基板
	E1715S,				// DELL E1715S tsutsui氏情報提供ありがとうございます
	P2214H_P72WF,		// DELL P2214H P72WF
	P2314H_48H1R,		// DELL P2314H 48H1R
	P2314H_48H1R_A01,	// DELL P2314H 48H1R A01 もより氏情報提供ありがとうございます
	P2314H_79H3D,		// DELL P2314H 79H3D
	P2314H_79H3D_B01,	// DELL P2314H 79H3D B01 もより氏情報提供ありがとうございます
	P2314H_9R54N,		// DELL P2314H 9R54N Thanks Carlos Bragatto
	PHI_252B9,			// PHILIPS 252B9/11
	LHRD56_IPAD97,		// 青ジャック基板にiPad 9.7型液晶を使用した15KHzモニタ用→なんか良い愛称ないんでしょうか
	LHRD56_IPAD97_POO,	// 青ジャック基板にiPad 9.7型液晶を使用した15KHzモニタ用(別版) プー氏情報提供ありがとうございます
	LHRD56_1366x768,	// 同 1366x768液晶用ファーム
	M_RT2556_FHD,		// 黒ジャック基板にgithubで見つけたファームウェア適用
	V_M56VDA_IPAD97,	// V.M56VDA iPad基板 okaz氏情報提供ありがとうございます
	V_M56VDA_IPAD97_2,	// V.M56VDA iPad基板 taobao
	JG2555TC_IPAD97,	// JG2555TC iPad基板 Thanks Newman -> KVC 9.7 iPad液晶(KCL-97DHS9) MJ氏情報提供ありがとうございます 
	Cocoper133,			// Cocoper 13.3 イカ先生氏情報提供ありがとうございます
	CZ617Ph,			// Cocoper CZ-617Ph ふゆき氏情報提供ありがとうございます
	CB242YEbmiprx,		// Acer CB242YEbmiprx
	CB272Ebmiprx,		// Acer CB272Ebmiprx AKT氏情報提供ありがとうございます
	EK221QE3bi,			// Acer EK221QE3bi tomo_retro氏情報提供ありがとうございます
	EK241YEbmix,		// Acer EK241YEbmix KAPPY.氏情報提供ありがとうございます
	EK241YEbmix_2,		// Acer EK241YEbmix_2 しげ氏情報提供ありがとうございます
	EK271Ebmix,			// Acer EK271Ebmix KAPPY.氏情報提供ありがとうございます
	EK271Ebmix_2,		// Acer EK271Ebmix_2 しげ氏情報提供ありがとうございます
	EK271Ebmix_3,		// Acer EK271Ebmix_3 Shuripon氏情報提供ありがとうございます
	KA222Q,				// Acer KA222Q CAT-2氏情報提供ありがとうございます
	KA222Q_2,			// Acer KA222Q_2 SATOSHI氏情報提供ありがとうございます
	QG221QHbmiix,		// Acer QG221QHbmiix KAPPY.氏情報提供ありがとうございます
	QG271Ebmiix,		// Acer QG271Ebmiix てまりあ氏情報提供ありがとうございます
	C24M2020DJP,		// Amazon C24M2020DJP えくしび氏情報提供ありがとうございます
	C27M2020DJP,		// Amazon C27M2020DJP みゆ氏情報提供ありがとうございます
	X2377HS,			// iiyama X2377HS KAPPY.氏情報提供ありがとうございます
	WIMAXIT,			// WIMAXIT FW:V001 楊手令氏情報提供ありがとうございます
	RTD2513A,			// RTD2513A使用基板 ponzu氏情報提供ありがとうございます
	BL912				// BenQ BL912 Thanks Higgy69
};

enum enMode
{
	ModeCheck = -1,
	ModeDump, 
	ModeModify,
	ModeModify4x3,
	ModeModifyExp
};
