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
	RTD2668,			// RTD2668�g�p���(aitendo?)
	PCB800099,			// RTD2662�g�p���
	E1715S,				// DELL E1715S tsutsui�����񋟂��肪�Ƃ��������܂�
	P2214H_P72WF,		// DELL P2214H P72WF
	P2314H_48H1R,		// DELL P2314H 48H1R
	P2314H_48H1R_A01,	// DELL P2314H 48H1R A01 ����莁���񋟂��肪�Ƃ��������܂�
	P2314H_79H3D,		// DELL P2314H 79H3D
	P2314H_79H3D_B01,	// DELL P2314H 79H3D B01 ����莁���񋟂��肪�Ƃ��������܂�
	P2314H_9R54N,		// DELL P2314H 9R54N Thanks Carlos Bragatto
	PHI_252B9,			// PHILIPS 252B9/11
	LHRD56_IPAD97,		// �W���b�N���iPad 9.7�^�t�����g�p����15KHz���j�^�p���Ȃ񂩗ǂ����̂Ȃ���ł��傤��
	LHRD56_IPAD97_POO,	// �W���b�N���iPad 9.7�^�t�����g�p����15KHz���j�^�p(�ʔ�) �v�[�����񋟂��肪�Ƃ��������܂�
	LHRD56_1366x768,	// �� 1366x768�t���p�t�@�[��
	M_RT2556_FHD,		// ���W���b�N���github�Ō������t�@�[���E�F�A�K�p
	V_M56VDA_IPAD97,	// V.M56VDA iPad��� okaz�����񋟂��肪�Ƃ��������܂�
	V_M56VDA_IPAD97_2,	// V.M56VDA iPad��� taobao
	JG2555TC_IPAD97,	// JG2555TC iPad��� Thanks Newman -> KVC 9.7 iPad�t��(KCL-97DHS9) MJ�����񋟂��肪�Ƃ��������܂� 
	Cocoper133,			// Cocoper 13.3 �C�J�搶�����񋟂��肪�Ƃ��������܂�
	CZ617Ph,			// Cocoper CZ-617Ph �ӂ䂫�����񋟂��肪�Ƃ��������܂�
	CB242YEbmiprx,		// Acer CB242YEbmiprx
	CB272Ebmiprx,		// Acer CB272Ebmiprx AKT�����񋟂��肪�Ƃ��������܂�
	EK221QE3bi,			// Acer EK221QE3bi tomo_retro�����񋟂��肪�Ƃ��������܂�
	EK241YEbmix,		// Acer EK241YEbmix KAPPY.�����񋟂��肪�Ƃ��������܂�
	EK241YEbmix_2,		// Acer EK241YEbmix_2 ���������񋟂��肪�Ƃ��������܂�
	EK271Ebmix,			// Acer EK271Ebmix KAPPY.�����񋟂��肪�Ƃ��������܂�
	EK271Ebmix_2,		// Acer EK271Ebmix_2 ���������񋟂��肪�Ƃ��������܂�
	EK271Ebmix_3,		// Acer EK271Ebmix_3 Shuripon�����񋟂��肪�Ƃ��������܂�
	KA222Q,				// Acer KA222Q CAT-2�����񋟂��肪�Ƃ��������܂�
	KA222Q_2,			// Acer KA222Q_2 SATOSHI�����񋟂��肪�Ƃ��������܂�
	QG221QHbmiix,		// Acer QG221QHbmiix KAPPY.�����񋟂��肪�Ƃ��������܂�
	QG271Ebmiix,		// Acer QG271Ebmiix �Ă܂肠�����񋟂��肪�Ƃ��������܂�
	C24M2020DJP,		// Amazon C24M2020DJP �������ю����񋟂��肪�Ƃ��������܂�
	C27M2020DJP,		// Amazon C27M2020DJP �݂䎁���񋟂��肪�Ƃ��������܂�
	X2377HS,			// iiyama X2377HS KAPPY.�����񋟂��肪�Ƃ��������܂�
	WIMAXIT,			// WIMAXIT FW:V001 �k��ߎ����񋟂��肪�Ƃ��������܂�
	RTD2513A,			// RTD2513A�g�p��� ponzu�����񋟂��肪�Ƃ��������܂�
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
