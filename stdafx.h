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
#else
#include <arpa/inet.h>
#include <libgen.h>
#include <fcntl.h>
#include <unistd.h>
#endif


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
	V_M56VDA_IPAD97_2,	// V.M56VDA iPad基板 taobao
	JG2555TC_IPAD97,	// JG2555TC iPad基板 Thanks Newman -> KVC 9.7 iPad液晶(KCL-97DHS9) MJ氏情報提供ありがとうございます 
	LHRD56_IPAD97_POO,	// 青ジャック基板にiPad 9.7型液晶を使用した15KHzモニタ用(別版) プー氏情報提供ありがとうございます
	EK271Ebmix,			// Acer EK271Ebmix KAPPY.氏情報提供ありがとうございます
	EK241YEbmix,		// Acer EK241YEbmix KAPPY.氏情報提供ありがとうございます
	QG221QHbmiix,		// Acer QG221QHbmiix KAPPY.氏情報提供ありがとうございます
	C24M2020DJP,		// Amazon C24M2020DJP えくしび氏情報提供ありがとうございます
	C27M2020DJP,		// Amazon C27M2020DJP みゆ氏情報提供ありがとうございます
	CZ617Ph,			// Cocoper CZ-617Ph ふゆき氏情報提供ありがとうございます
	KA222Q,				// Acer KA222Q CAT-2氏情報提供ありがとうございます
	EK221QE3bi,			// Acer EK221QE3bi tomo_retro氏情報提供ありがとうございます
	Cocoper133,			// Cocoper 13.3 イカ先生氏情報提供ありがとうございます
	X2377HS,			// iiyama X2377HS KAPPY.氏情報提供ありがとうございます
	CB272Ebmiprx,		// Acer CB272Ebmiprx AKT氏情報提供ありがとうございます
	E1715S,				// DELL E1715S tsutsui氏情報提供ありがとうございます
};

enum enMode
{
	ModeCheck = -1,
	ModeDump, 
	ModeModify,
	ModeModify4x3,
	ModeModifyExp
};
