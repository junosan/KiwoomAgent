
// SendOrderDlg.h : header file
//

#pragma once
#include "afxwin.h"


// CSendOrderDlg dialog
class CSendOrderDlg : public CDialogEx
{
// Construction
public:
	CSendOrderDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_SENDORDER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// My data

	CFont m_font;
	CString m_sScrNo, m_sAccNo;
	CString m_sLastTime;
	CMapStringToString m_mapFIDName;
	static CSendOrderDlg *pThis;

	static const unsigned int m_ncKRXCodesBufSize = 1024;
	TCHAR m_sKRXCodes[m_ncKRXCodesBufSize];		// String of ;-separated KRX codes from config.ini
	unsigned int m_nKRXCodes;					// Number of codes in "SendOrder\config.ini"
	unsigned int m_piKRXCodes[MAX_CODE_N];		// Array of codes, parsed

	double m_dBuyFee0;
	double m_dSelFee0;
	double m_dSelTax0;
	double m_dSelTax1;

	CMutex m_mutexRealData;
	float  m_pPr   [MAX_CODE_N];
	_int64 m_pSumPQ[MAX_CODE_N];
	_int64 m_pSumQ [MAX_CODE_N];
	int    m_pTbP  [MAX_CODE_N][20];
	int    m_pTbQ  [MAX_CODE_N][20];

	CMutex m_mutexBalance;
	_int64 m_iBal;							// corresponds to d+2출금가능금액
	int m_pStkCnt[MAX_CODE_N];				// number of stocks in possession and not in queue for sale (= 주문가능수량)
	int m_pOrdCnt[MAX_CODE_N];				// number of orders (buy + sell) in queue 
	int m_pOrdNo[MAX_CODE_N][ORD_Q_SIZE];	// packed from left, leftmost first, + means buy, - means sell
	int m_pOrdP [MAX_CODE_N][ORD_Q_SIZE];
	int m_pOrdQ [MAX_CODE_N][ORD_Q_SIZE];	// (= 미체결수량)

	static const UINT_PTR m_ncFlushTimerID = 1;
	static VOID CALLBACK TimerCallback(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

	void InitFIDName();
	void SendOrder(LPCTSTR sCommand);
	void DisplayUpdatedTime();
	void DisplayMain(LPCTSTR sDisp, bool bRefresh = true);
	void DisplayMsg(LPCTSTR sDisp, bool bRefresh = true);
	int DisplayBalance();
	void FixTbQuantization(int *inP, int *inQ);
	bool IsPriceValid(int p);

	// For synchronization and rate limiting of Tr & Order requests
	void InitTr();
	long WaitTr(bool bTimeout = false);
	void EndTr(long lPrevNext);

	// TCP server
	unsigned short m_iTCPPort;
	SOCKET m_sockServ, m_sockConn;
	static UINT __cdecl TCPServerThread(LPVOID pParam);

private:
	long m_lContTr;		// 0 for ordinary Tr, 2 for continued Tr (in case of > 100 items)

	bool m_bStarted;
	bool m_bLoggedIn;
	bool m_bConnected;
	bool m_bLockTr; // not to be modified directly
	bool m_bInitializing;
	bool m_bSORunning;

	int m_iOrdTrEventCnt;

	int m_nTrCntThisTick;

	CMutex m_mutexFcnLog;
	FILE *m_pfFcnLog;
	void WriteFcnLog(LPCTSTR sText);

	static inline double round (double val) {return floor (val + 0.5);}

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonLogin();
	DECLARE_EVENTSINK_MAP()
	void OnEventConnect(long nErrCode);
	afx_msg void OnBnClickedButtonStart();
	void OnReceiveRealData(LPCTSTR sRealKey, LPCTSTR sRealType, LPCTSTR sRealData);
	CButton m_btLogin;
	CButton m_btStart;
	CButton m_btCancel;
	afx_msg void OnBnClickedButtonSaveLog();
	CButton m_btSaveLog;
	afx_msg void OnBnClickedCancel();
	CListBox m_listMain;
	CListBox m_listMsg;
	void OnReceiveTrData(LPCTSTR sScrNo, LPCTSTR sRQName, LPCTSTR sTrCode, LPCTSTR sRecordName, LPCTSTR sPrevNext, long nDataLength, LPCTSTR sErrorCode, LPCTSTR sMessage, LPCTSTR sSplmMsg);
	void OnReceiveMsg(LPCTSTR sScrNo, LPCTSTR sRQName, LPCTSTR sTrCode, LPCTSTR sMsg);
	void OnReceiveChejanData(LPCTSTR sGubun, long nItemCnt, LPCTSTR sFIdList);
	CListBox m_listBal;
	CButton m_ckEnableTR;
};
