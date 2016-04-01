
// SendOrder.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


#include <fstream>
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <math.h>

#define DISP_VERBOSE			1
#define FCN_LOG					0

#define MAX_CODE_N				200
#define ORD_Q_SIZE				(1 << 12)

#define REFRESH_INTERVAL_SEC	10
#define TR_RATE_CAP				5
#define BLOCK_WAIT_MS			30
#define OVERFLOW_WAIT_MS		200
#define ORD_TIMEOUT_MS			5000

#define TCP_PASSWORD			"sendorder"
#define TCP_BACKLOG_SIZE		8
#define TCP_BUF_SIZE			(1 << 16)


#include "KHOpenAPICtrl.h"

#define OP_ERR_NONE				0			//"정상처리"
#define OP_ERR_LOGIN			-100			//"사용자정보교환에 실패하였습니다. 잠시후 다시 시작하여 주십시오."
#define OP_ERR_CONNECT			-101			//"서버 접속 실패"
#define OP_ERR_VERSION			-102			//"버전처리가 실패하였습니다."

#define OP_ERR_SISE_OVERFLOW	-200			//"시세조회 과부하"
#define OP_ERR_RQ_STRUCT_FAIL	-201		//"REQUEST_INPUT_st Failed"
#define OP_ERR_RQ_STRING_FAIL	-202			//"요청 전문 작성 실패"
#define OP_ERR_NO_DATA			-203			//"데이터가 존재하지 않습니다."
#define OP_ERR_OVER_MAX_DATA	-204			//"한번에 조회 가능한 종목개수는 최대 100종목 입니다."

#define OP_ERR_ORD_WRONG_INPUT	-300			//"입력값 오류"
#define OP_ERR_ORD_WRONG_ACCNO	-301			//"계좌비밀번호를 입력하십시오."
#define OP_ERR_OTHER_ACC_USE	-302			//"타인계좌는 사용할 수 없습니다."
#define OP_ERR_MIS_2BILL_EXC	-303			//"주문가격이 20억원을 초과합니다."
#define OP_ERR_MIS_5BILL_EXC	-304			//"주문가격이 50억원을 초과합니다."
#define OP_ERR_MIS_1PER_EXC		-305			//"주문수량이 총발행주수의 1%를 초과합니다."
#define OP_ERR_MIS_3PER_EXC		-306			//"주문수량은 총발행주수의 3%를 초과할 수 없습니다."
#define OP_ERR_SEND_FAIL		-307			//"주문전송실패"
#define OP_ERR_ORD_OVERFLOW		-308			//"주문전송 과부하"

#define OP_FID_종목코드			9001		// A###### (A 제거 후 사용할것)
#define OP_FID_주문가능수량		933			// 보유수량 - 판매주문중수량
#define OP_FID_주문상태			913			// 접수, 체결, 확인
#define OP_FID_주문구분			905			// +매수 매수취소 +매수정정 -매도 매도취소 -매도정정
#define OP_FID_주문번호			9203		// %07d
#define OP_FID_주문가격			901
#define OP_FID_미체결수량		902			
#define OP_FID_원주문번호		904			// %07d
#define OP_FID_단위체결가		914			// 반드시 주문가격과 같지는 않음에 유의
#define OP_FID_단위체결량		915			// 주문의 누적 체결량이 아닌 이벤트 자체의 체결량


// CSendOrderApp:
// See SendOrder.cpp for the implementation of this class
//

class CSendOrderApp : public CWinApp
{
public:
	CSendOrderApp();
	CKHOpenAPI m_cKHOpenAPI;
	CString m_sAppPath;

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CSendOrderApp theApp;