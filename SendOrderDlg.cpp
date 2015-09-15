
// SendOrderDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SendOrder.h"
#include "SendOrderDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSendOrderDlg dialog

// Static variables
CSendOrderDlg *CSendOrderDlg::pThis;


CSendOrderDlg::CSendOrderDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSendOrderDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSendOrderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_KHOPENAPICTRL1, theApp.m_cKHOpenAPI);
	DDX_Control(pDX, IDC_BUTTON_LOGIN, m_btLogin);
	DDX_Control(pDX, IDC_BUTTON_START, m_btStart);
	DDX_Control(pDX, IDC_BUTTON_SAVELOG, m_btSaveLog);
	DDX_Control(pDX, IDCANCEL, m_btCancel);
	DDX_Control(pDX, IDC_LIST_MAIN, m_listMain);
	DDX_Control(pDX, IDC_LIST_MSG, m_listMsg);
	DDX_Control(pDX, IDC_LIST_BAL, m_listBal);
	DDX_Control(pDX, IDC_CHECK_ENABLE_TR, m_ckEnableTR);
}

BEGIN_MESSAGE_MAP(CSendOrderDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_LOGIN, &CSendOrderDlg::OnBnClickedButtonLogin)
	ON_BN_CLICKED(IDC_BUTTON_START, &CSendOrderDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_SAVELOG, &CSendOrderDlg::OnBnClickedButtonSaveLog)
	ON_BN_CLICKED(IDCANCEL, &CSendOrderDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CSendOrderDlg message handlers

BOOL CSendOrderDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	// For access from static member functions
	pThis = this;

	m_btStart.EnableWindow(FALSE);
	m_btSaveLog.EnableWindow(FALSE);

	DisplayUpdatedTime();

	m_lContTr = 0;
	m_bStarted = false;
	m_bInitializing = false;
	m_bLoggedIn = false;
	m_bConnected = false;
	m_bSORunning = false;

	m_nTrCntThisTick = 0;
	((CWnd*)GetDlgItem(IDC_STATIC_DISP_TRCNT))->SetWindowText(_T(" 0"));

	m_font.CreatePointFont(80, _T("Courier New"));
	m_listMain.SetFont(&m_font);
	m_listMsg.SetFont(&m_font);
	m_listBal.SetFont(&m_font);

	// Used for API communication
	m_sScrNo = _T("8888");

	// Will be obtained thru API upon login
	m_sAccNo = _T("8069605211");

	InitFIDName();

	// Read config.ini
	const unsigned int nBufSize=64;
	TCHAR buf[nBufSize];
	::GetPrivateProfileString(_T("SENDORDER"), _T("TCP_PORT"), _T("50505"), buf, nBufSize, theApp.m_sAppPath + _T("\\SendOrder\\config.ini"));
	sscanf_s(buf, _T("%d"), &m_iTCPPort);

	::GetPrivateProfileString(_T("SENDORDER"), _T("KRX_CODE"), _T("000660"), m_sKRXCodes, m_ncKRXCodesBufSize, theApp.m_sAppPath + _T("\\SendOrder\\config.ini"));
	CString sItem;
	int i = 0;
	while(AfxExtractSubString(sItem, m_sKRXCodes, i++, _T(';')))
	{
		m_nKRXCodes = i;
		m_piKRXCodes[i - 1] = _ttoi(sItem);
	}
	sItem.Format(_T("Found %d codes in SendOrder\\config.ini"), m_nKRXCodes);
	DisplayMsg(sItem);

	::GetPrivateProfileString(_T("SENDORDER"), _T("POS"), _T("100 100"), buf, nBufSize, theApp.m_sAppPath + _T("\\SendOrder\\config.ini"));
	int x, y;
	if (2 == sscanf_s(buf, _T("%d %d"), &x, &y))
		SetWindowPos(NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	// Function in/out log for debugging
	if (FCN_LOG) fopen_s(&m_pfFcnLog, theApp.m_sAppPath + _T("\\SendOrder\\SendOrder_fcnlog.txt"), _T("w"));

	// TCP server thread
	AfxBeginThread(CSendOrderDlg::TCPServerThread, NULL);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSendOrderDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSendOrderDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CSendOrderDlg::OnBnClickedButtonLogin()
{
	// TODO: Add your control notification handler code here

	if (theApp.m_cKHOpenAPI.CommConnect() < 0)
	{
		DisplayMsg(_T("KHOpenAPI.CommConnect: Login failed"));
	}

}

BEGIN_EVENTSINK_MAP(CSendOrderDlg, CDialogEx)
	ON_EVENT(CSendOrderDlg, IDC_KHOPENAPICTRL1, 5, CSendOrderDlg::OnEventConnect, VTS_I4)
	ON_EVENT(CSendOrderDlg, IDC_KHOPENAPICTRL1, 2, CSendOrderDlg::OnReceiveRealData, VTS_BSTR VTS_BSTR VTS_BSTR)
	ON_EVENT(CSendOrderDlg, IDC_KHOPENAPICTRL1, 1, CSendOrderDlg::OnReceiveTrData, VTS_BSTR VTS_BSTR VTS_BSTR VTS_BSTR VTS_BSTR VTS_I4 VTS_BSTR VTS_BSTR VTS_BSTR)
	ON_EVENT(CSendOrderDlg, IDC_KHOPENAPICTRL1, 3, CSendOrderDlg::OnReceiveMsg, VTS_BSTR VTS_BSTR VTS_BSTR VTS_BSTR)
	ON_EVENT(CSendOrderDlg, IDC_KHOPENAPICTRL1, 4, CSendOrderDlg::OnReceiveChejanData, VTS_BSTR VTS_I4 VTS_BSTR)
END_EVENTSINK_MAP()


void CSendOrderDlg::OnEventConnect(long nErrCode)
{
	// TODO: Add your message handler code here
	
	CString sTemp;

	if (nErrCode < 0)
	{
		sTemp.Format(_T("KHOpenAPI.OnEventConnect: Error %d"), nErrCode);
		DisplayMsg(sTemp);
	}
	else
	{
		m_bLoggedIn = true;
		DisplayMsg(_T("Login successful"));

		sTemp = theApp.m_cKHOpenAPI.GetLoginInfo(_T("ACCNO"));
		AfxExtractSubString(m_sAccNo, sTemp, 0, _T(';'));
		sTemp.Format(_T("Account number %s"), m_sAccNo);
		DisplayMsg(sTemp);

		DisplayMsg(_T("IMPORTANT: Enter account password before proceeding"));
			
		m_btLogin.EnableWindow(FALSE);
		if (m_bConnected)
			m_btStart.EnableWindow(TRUE);
	}

}


void CSendOrderDlg::OnBnClickedButtonStart()
{
	// TODO: Add your control notification handler code here

	long lRet;

	KillTimer(m_ncFlushTimerID);

	CSingleLock lockRealData(&m_mutexRealData);
	if (lockRealData.Lock())
	{
		// Initialize RealData
		memset(m_pPr   , 0, sizeof(float)  * MAX_CODE_N);
		memset(m_pSumPQ, 0, sizeof(_int64) * MAX_CODE_N);
		memset(m_pSumQ , 0, sizeof(_int64) * MAX_CODE_N);
		memset(m_pTbP  , 0, sizeof(int)    * MAX_CODE_N * 20);
		memset(m_pTbQ  , 0, sizeof(int)    * MAX_CODE_N * 20);
		lockRealData.Unlock();
	}

	CSingleLock lockBalance(&m_mutexBalance);
	if (lockBalance.Lock())
	{
		// Initialize Balance
		memset(m_pStkCnt, 0, sizeof(int)   * MAX_CODE_N);
		memset(m_pOrdCnt, 0, sizeof(int)   * MAX_CODE_N);
		lockBalance.Unlock();
	}

	m_btStart.EnableWindow(false);

	DisplayMsg(_T("[OPT10001] Retrieving daily reference price..."));

	CString sText, sCode;
	CString sRQName = _T("reference price");
	CString sTrCode = _T("OPT10001");

	for (unsigned int i = 0; i < m_nKRXCodes; i++)
	{
		sCode.Format(_T("%06d"), m_piKRXCodes[i]);

		lRet = 0;
		do {
			if (lRet == OP_ERR_SISE_OVERFLOW)
			{
				sText.Format(_T("RefPrice: Overflow %s"), sCode);
				DisplayMsg(sText);
				Sleep(OVERFLOW_WAIT_MS);
			}
			InitTr();
			theApp.m_cKHOpenAPI.SetInputValue(_T("종목코드"), sCode);
			lRet = theApp.m_cKHOpenAPI.CommRqData(sRQName, sTrCode, 0, m_sScrNo);
		} while (lRet == OP_ERR_SISE_OVERFLOW);

		WaitTr();
	}

	DisplayMsg(_T("[OPT10001] Done"));
	m_btSaveLog.EnableWindow(TRUE);

	m_bInitializing = true;
	if(DisplayBalance() == -1) // This is also for initializing RealData & Balance
	{
		DisplayMsg(_T("Initialization: Error (account password not entered?)"));
		m_btStart.EnableWindow(true);
		return;
	}
	m_bInitializing = false;

	// Set up real time monitor API (KRX codes & FIDs)
	if ((lRet = theApp.m_cKHOpenAPI.SetRealReg(m_sScrNo, m_sKRXCodes, _T("10;15;20;21;27;28;41;42;43;44;45;46;47;48;49;50;51;52;53;54;55;56;57;58;59;60;61;62;63;64;65;66;67;68;69;70;71;72;73;74;75;76;77;78;79;80"), _T("0"))) < 0)
	{
		CString sDisp;
		sDisp.Format(_T("SetRealReg: Error #%d"), lRet);
		DisplayMsg(sDisp);
	}
	else
	{
		DisplayMsg(_T("SetRealReg: Starting realtime monitor"));
	}
	
	DisplayMsg(_T("RefreshTimer: Starting soon..."));

	// Wait until next integer multiple of REFRESH_INTERVAL_SEC since 09:00:00
	CTime tObj = CTime::GetCurrentTime();
	int t = (tObj.GetHour() - 9) * 3600 + tObj.GetMinute() * 60 + tObj.GetSecond();
	int tStart = (t / REFRESH_INTERVAL_SEC) * REFRESH_INTERVAL_SEC + REFRESH_INTERVAL_SEC;

	int iCount = 0;
	BOOL bDoingBackgroundProcessing = TRUE;
	while (bDoingBackgroundProcessing) 
	{ 
		MSG msg;
		while (::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) 
		{ 
			if (!AfxGetApp()->PumpMessage()) 
			{ 
				bDoingBackgroundProcessing = FALSE; 
				::PostQuitMessage(0); 
				break; 
			} 
		} 
		// let MFC do its idle processing
		LONG lIdle = 0;
		while (AfxGetApp()->OnIdle(lIdle++ ))
			;  
		// Perform some background processing here  
		// using another call to OnIdle
		
		Sleep(BLOCK_WAIT_MS);
		
		CTime tObj = CTime::GetCurrentTime();
		if ((tStart <= (tObj.GetHour() - 9) * 3600 + tObj.GetMinute() * 60 + tObj.GetSecond()) && (iCount++ == 1))
		{
			// Guarantee at least BLOCK_WAIT_MS separation to avoid timer drift problems
			SetTimer(m_ncFlushTimerID, REFRESH_INTERVAL_SEC * 1000, TimerCallback);
			break;
		}
	}
}

void CSendOrderDlg::OnBnClickedButtonSaveLog()
{
	// TODO: Add your control notification handler code here

	CString sItem;
	int n = m_listMain.GetCount();
	std::ofstream f;
	sItem = theApp.m_sAppPath;
	sItem.Append(_T("\\SendOrder\\SendOrder_log.txt"));
	f.open(sItem, std::ofstream::out);

	for (int i = 0; i < n; i++)
	{
		m_listMain.GetText(i, sItem);
		sItem.AppendChar(_T('\n'));
		f.write(sItem, sItem.GetLength());
	}

	f.close();

	DisplayMsg(_T("Log saved"));
}

void CSendOrderDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here

	CDialogEx::OnCancel();

	const unsigned int nBufSize=64;
	TCHAR buf[nBufSize];
	RECT rect;
	GetWindowRect(&rect);
	sprintf_s(buf, _T("%d %d"), rect.left, rect.top);
	::WritePrivateProfileString(_T("SENDORDER"), _T("POS"), buf, theApp.m_sAppPath + _T("\\SendOrder\\config.ini"));

	if (FCN_LOG) fclose(m_pfFcnLog);

	if (m_sockServ != -1) closesocket(m_sockServ);
	if (m_sockConn != -1) closesocket(m_sockConn);
    WSACleanup();

	// This very often causes infinite loop in the background after app closes
	// theApp.m_cKHOpenAPI.CommTerminate();
	// PostQuitMessage(0);

	// Force exit without invoking theApp.m_cKHOpenAPI.CommTerminate() in destructors
	SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
	abort();
}

void CSendOrderDlg::DisplayUpdatedTime()
{
	// Display time of last event
	m_sLastTime = CTime::GetCurrentTime().Format(_T("%Y.%m.%d %H:%M:%S"));
	((CWnd*)GetDlgItem(IDC_STATIC_DISP_TIME))->SetWindowText(m_sLastTime);
}

void CSendOrderDlg::DisplayMain(LPCTSTR sDisp, bool bRefresh)
{
	CString str = CTime::GetCurrentTime().Format(_T("%H:%M:%S "));
	str.Append(sDisp);
	m_listMain.AddString(str);

	if (bRefresh)
	{
		m_listMain.SetCurSel(m_listMain.GetCount() - 1);
		m_listMain.SetCurSel(-1);

		DisplayUpdatedTime();
	}
}

void CSendOrderDlg::DisplayMsg(LPCTSTR sDisp, bool bRefresh)
{
	CString str = CTime::GetCurrentTime().Format(_T("%H:%M:%S "));
	str.Append(sDisp);
	m_listMsg.AddString(str);

	if (bRefresh)
	{
		m_listMsg.SetCurSel(m_listMsg.GetCount() - 1);
		m_listMsg.SetCurSel(-1);

		DisplayUpdatedTime();
	}
}

void CSendOrderDlg::OnReceiveRealData(LPCTSTR sRealKey, LPCTSTR _sRealType, LPCTSTR sRealData)
{
	// TODO: Add your message handler code here
	if (FCN_LOG) WriteFcnLog(_T(">> OnReceiveRealData"));

	CSingleLock lockRealData(&m_mutexRealData);
	CString sRealType(_sRealType);

	unsigned int iCode = _ttoi(sRealKey);
	unsigned int iCodeInd;
	for (iCodeInd = 0; iCodeInd < m_nKRXCodes; iCodeInd++)
		if (iCode == m_piKRXCodes[iCodeInd])
			break;

	if ((iCodeInd != m_nKRXCodes) && (_T("주식체결") == sRealType))
	{
		if (lockRealData.Lock())
		{
			int p = abs(_ttoi(theApp.m_cKHOpenAPI.GetCommRealData(sRealKey, 10)));
			int q = abs(_ttoi(theApp.m_cKHOpenAPI.GetCommRealData(sRealKey, 15)));

			m_pSumPQ[iCodeInd] += (_int64)p * q;
			m_pSumQ[iCodeInd] += q;

			lockRealData.Unlock();
		}
	}

	if ((iCodeInd != m_nKRXCodes) && (_T("주식호가잔량") == sRealType))
	{
		if (lockRealData.Lock())
		{
			for (int i = 0; i < 10; i++)
			{
				// Sell price 10 ~ 1 FID(50->41, 70->61)
				m_pTbP[iCodeInd][i] = abs(_ttoi(theApp.m_cKHOpenAPI.GetCommRealData(sRealKey, 50 - i)));
				m_pTbQ[iCodeInd][i] = abs(_ttoi(theApp.m_cKHOpenAPI.GetCommRealData(sRealKey, 70 - i)));
				
				// Buy price 1 ~ 10 FID(51->60, 71->80)
				m_pTbP[iCodeInd][i + 10] = abs(_ttoi(theApp.m_cKHOpenAPI.GetCommRealData(sRealKey, 51 + i)));
				m_pTbQ[iCodeInd][i + 10] = abs(_ttoi(theApp.m_cKHOpenAPI.GetCommRealData(sRealKey, 71 + i)));
			}

			lockRealData.Unlock();
		}
	}

	if (FCN_LOG) WriteFcnLog(_T("<< OnReceiveRealData"));
}

void CSendOrderDlg::OnReceiveTrData(LPCTSTR sScrNo, LPCTSTR sRQName, LPCTSTR sTrCode, LPCTSTR sRecordName, LPCTSTR sPrevNext, long nDataLength, LPCTSTR sErrorCode, LPCTSTR sMessage, LPCTSTR sSplmMsg)
{
	// TODO: Add your message handler code here
	if (FCN_LOG) WriteFcnLog(_T(">> OnReceiveTrData"));

	CSingleLock lockRealData(&m_mutexRealData);
	CSingleLock lockBalance(&m_mutexBalance);
	long nCnt = theApp.m_cKHOpenAPI.GetRepeatCnt(sTrCode, sRQName);
	CString sData;
	CString sTemp = sRQName;

	if (sTemp == _T("reference price"))
	{
		unsigned int iCode = _ttoi(theApp.m_cKHOpenAPI.GetCommData(sTrCode, sRQName, 0, _T("종목코드")).Trim());
		unsigned int iCodeInd;
		for (iCodeInd = 0; iCodeInd < m_nKRXCodes; iCodeInd++)
			if (iCode == m_piKRXCodes[iCodeInd])
				break;
		int iRefPrice = _ttoi(theApp.m_cKHOpenAPI.GetCommData(sTrCode, sRQName, 0, _T("기준가")).Trim());
		
		if (iCodeInd == m_nKRXCodes)
			DisplayMsg(_T("RefPrice: Unable to find code")); // this happens if code is nonexistent
		else if (lockRealData.Lock())
		{
			m_pPr[iCodeInd] = (float)iRefPrice;
			m_pTbP[iCodeInd][9] = iRefPrice;
			FixTbQuantization(&m_pTbP[iCodeInd][0], &m_pTbQ[iCodeInd][0]);

			lockRealData.Unlock();
		}
		
		sTemp.Format(_T("{%06d} %8d"), iCode, iRefPrice);
		DisplayMsg(sTemp);

		EndTr(0);
	}
	if (sTemp == _T("account balance"))
	{
		_int64 iBal = _ttoi64(theApp.m_cKHOpenAPI.GetCommData(sTrCode, sRQName, 0, _T("d+2출금가능금액")).Trim());

		if (m_bInitializing && lockBalance.Lock())
		{
			m_iBal = iBal;

			lockBalance.Unlock();
		}

		sTemp.Format(_T("Balance %12I64d"), iBal);
		m_listBal.AddString(sTemp);

		EndTr(0);
	}
	if (sTemp == _T("stock list"))
	{
		if (nCnt > 0)
		{
			if (m_bInitializing)
				lockBalance.Lock();

			for (int i = 0; i < nCnt; i++)
			{
				unsigned int iCode = _ttoi(theApp.m_cKHOpenAPI.GetCommData(sTrCode, sRQName, i, _T("종목번호")).Trim().Mid(1));
				unsigned int iCodeInd;
				for (iCodeInd = 0; iCodeInd < m_nKRXCodes; iCodeInd++)
					if (iCode == m_piKRXCodes[iCodeInd])
						break;
				int iStkCnt = _ttoi(theApp.m_cKHOpenAPI.GetCommData(sTrCode, sRQName, i, _T("보유수량")).Trim());

				if (iCodeInd == m_nKRXCodes)
					DisplayMsg(_T("StockList: Unable to find code")); // this can happen if I own stock purchased thru other channels
				else if (m_bInitializing)
					m_pStkCnt[iCodeInd] = iStkCnt;  // need to subtract amount in sale order later!

				sTemp.Format(_T("{%06d} (%2d) (inc. ord.)"), iCode, iStkCnt);
				m_listBal.AddString(sTemp);
			}

			if (m_bInitializing)
				lockBalance.Unlock();
		}

		EndTr(_ttoi(sPrevNext));
	}
	if (sTemp == _T("order list"))
	{
		if (nCnt > 0)
		{
			if (m_bInitializing)
				lockBalance.Lock();

			for (int i = 0; i < nCnt; i++) // newest -> oldest (need to be reversed after all 'continued' TRs finish)
			{
				int iOrdQ = _ttoi(theApp.m_cKHOpenAPI.GetCommData(sTrCode, sRQName, i, _T("미체결수량")).Trim());
				if (iOrdQ != 0)
				{
					int iOrdNo = _ttoi(theApp.m_cKHOpenAPI.GetCommData(sTrCode, sRQName, i, _T("주문번호")).Trim());
					sData = theApp.m_cKHOpenAPI.GetCommData(sTrCode, sRQName, i, _T("주문구분")).Trim();
					int iOrdType = ((sData == _T("+매수")) || (sData == _T("+매수정정"))) - ((sData == _T("-매도")) || (sData == _T("-매도정정"))); // +1: buy, -1: sell, 0 should not happen
					if (iOrdType == 0)
						DisplayMsg(_T("OrderList: Zero order type found"));
					unsigned int iCode = _ttoi(theApp.m_cKHOpenAPI.GetCommData(sTrCode, sRQName, i, _T("종목코드")).Trim());
					unsigned int iCodeInd;
					for (iCodeInd = 0; iCodeInd < m_nKRXCodes; iCodeInd++)
						if (iCode == m_piKRXCodes[iCodeInd])
							break;
					int iOrdP = _ttoi(theApp.m_cKHOpenAPI.GetCommData(sTrCode, sRQName, i, _T("주문가격")).Trim());

					if (iCodeInd == m_nKRXCodes)
						DisplayMsg(_T("OrderList: Unable to find code")); // this can happen if I own orders placed thru other channels
					else if (m_bInitializing && (iOrdType != 0))
					{
						int iQInd = m_pOrdCnt[iCodeInd]++;
						if (iQInd == ORD_Q_SIZE)
							DisplayMsg(_T("OrderList: Order queue overflow"));
						m_pOrdNo[iCodeInd][iQInd] = iOrdNo * iOrdType;
						m_pOrdP [iCodeInd][iQInd] = iOrdP;
						m_pOrdQ [iCodeInd][iQInd] = iOrdQ;
						if (iOrdType == -1)
						{
							m_pStkCnt[iCodeInd] -= iOrdQ;
							if (m_pStkCnt[iCodeInd] < 0)
								DisplayMsg(_T("OrderList: Negative order found")); // this should never happen, but left for debugging
						}
					}

					sTemp.Format(_T("[%07d] %2s {%06d} %8d (%2d)"), iOrdNo, (iOrdType > 0 ? _T("b") : _T("s")), iCode, iOrdP, iOrdQ);
					m_listBal.AddString(sTemp);
				}
			}

			if (m_bInitializing)
				lockBalance.Unlock();
		}

		EndTr(_ttoi(sPrevNext));
	}
	if (sTemp == _T("order stock"))
	{
		EndTr(0);
	}

	if (FCN_LOG) WriteFcnLog(_T("<< OnReceiveTrData"));
}


void CSendOrderDlg::OnReceiveMsg(LPCTSTR sScrNo, LPCTSTR sRQName, LPCTSTR sTrCode, LPCTSTR sMsg)
{
	// TODO: Add your message handler code here
	if (FCN_LOG) WriteFcnLog(_T(">> OnReceiveMsg"));

	CString sText(sMsg), sDisp(sMsg);
	bool bDisp = true;

	// Ignore list
	if (sText.Mid(1, 6) == _T("00Z310"))		bDisp = false; // 모의서버 조회 완료
	if (sText == _T(" 조회가 완료되었습니다."))	bDisp = false;

	// Modify known msg's
	if (DISP_VERBOSE)
	{
		if (sText.Mid(1, 6) == _T("00Z112")) sDisp.Format(_T("  b   sent"));
		if (sText.Mid(1, 6) == _T("00Z113")) sDisp.Format(_T("  s   sent"));
		if (sText.Mid(1, 6) == _T("00Z114")) sDisp.Format(_T("mb/ms sent"));
		if (sText.Mid(1, 6) == _T("00Z115")) sDisp.Format(_T("cb/cs sent"));
	}
	else
	{
		if (sText.Mid(1, 6) == _T("00Z112")) bDisp = false;
		if (sText.Mid(1, 6) == _T("00Z113")) bDisp = false;
		if (sText.Mid(1, 6) == _T("00Z114")) bDisp = false;
		if (sText.Mid(1, 6) == _T("00Z115")) bDisp = false;
	}

	//// Unsuccessful request
	//if (sDisp.Mid(0, 1) == _T("["))
	//	EndTr();

	if (bDisp)
	{
		sText.Format(_T("Msg: %s"), sDisp);
		DisplayMsg(sText);
	}

	if (FCN_LOG) WriteFcnLog(_T("<< OnReceiveMsg"));
}


void CSendOrderDlg::OnReceiveChejanData(LPCTSTR _sGubun, long nItemCnt, LPCTSTR sFIdList)
{
	// TODO: Add your message handler code here
	if (FCN_LOG) WriteFcnLog(_T(">> OnReceiveChejanData"));

	/*
* Order queue logic
	if (sGubun == 0)
		if (주문상태 == 접수)
			if (주문구분 != 취소)
				if (!exists 주문번호) && (미체결수량 != 0) (prevent re-creating removed order)
					add order
					if (주문구분 == +매수)
						sub balance
					if (주문구분 == +매수정정)
						mod balance
			else
				if (주문구분 == 매수취소)
					add balance (look up original's price)
			if (주문구분 == 취소 | 정정)
				update 원주문 미체결수량 -= 미체결수량
				if (원주문 미체결수량 == 0)
					remove 원주문
		if (주문상태 == 체결)
			update 미체결수량
			if (주문구분 == -매도 | -매도정정)
				add balance
		if (미체결수량 == 0) && (주문상태 != 확인)
			remove order		
	if (sGubun == 1)
		update 주문가능수량
	*/

	CString sGubun(_sGubun), sDisp;
	CSingleLock lockBalance(&m_mutexBalance);

	if ((sGubun == _T("0")) && lockBalance.Lock())
	{
		int     iCode    = _ttoi(theApp.m_cKHOpenAPI.GetChejanData(OP_FID_종목코드    ).Trim().Mid(1));
		CString sOrdStat =       theApp.m_cKHOpenAPI.GetChejanData(OP_FID_주문상태    ).Trim();
		CString sOrdCat  =       theApp.m_cKHOpenAPI.GetChejanData(OP_FID_주문구분    ).Trim();
		int     iOrdNo   = _ttoi(theApp.m_cKHOpenAPI.GetChejanData(OP_FID_주문번호    ).Trim());
		int     iOrdP    = _ttoi(theApp.m_cKHOpenAPI.GetChejanData(OP_FID_주문가격    ).Trim());
		int     iOrdQ    = _ttoi(theApp.m_cKHOpenAPI.GetChejanData(OP_FID_미체결수량  ).Trim());
		int     iOgOrdNo = _ttoi(theApp.m_cKHOpenAPI.GetChejanData(OP_FID_원주문번호  ).Trim());
		int		iDeltaQ  = _ttoi(theApp.m_cKHOpenAPI.GetChejanData(OP_FID_단위체결량  ).Trim());
		int     iOrdType = ((sOrdCat == _T("+매수")) || (sOrdCat == _T("매수취소")) || (sOrdCat == _T("+매수정정"))) -
						   ((sOrdCat == _T("-매도")) || (sOrdCat == _T("매도취소")) || (sOrdCat == _T("-매도정정")));
		if (iOrdType == 0)
		{
			DisplayMsg(_T("ChejanData: Zero order type found"));
			if (FCN_LOG) WriteFcnLog(_T("<< OnReceiveChejanData"));
			return;
		}
		else
		{
			iOrdNo   *= iOrdType; // + for buy, - for sell
			iOgOrdNo *= iOrdType;
		}

		for (unsigned int iCodeInd = 0; iCodeInd < m_nKRXCodes; iCodeInd++)
			if (iCode == m_piKRXCodes[iCodeInd])
			{
				if (sOrdStat == _T("접수"))
				{
					if ((sOrdCat != _T("매수취소")) && (sOrdCat != _T("매도취소")))
					{
						if (iOrdQ != 0)
						{
							int iQInd;
							for (iQInd = 0; iQInd < m_pOrdCnt[iCodeInd]; iQInd++)
								if (iOrdNo == m_pOrdNo[iCodeInd][iQInd])
									break;

							if (iQInd == m_pOrdCnt[iCodeInd])
							{
								//// Acknowledge receipt of new b/s/mb/ms
								//EndTr();

								if (iQInd == ORD_Q_SIZE)
								{
									DisplayMsg(_T("ChejanData: Order queue overflow"));
									if (FCN_LOG) WriteFcnLog(_T("<< OnReceiveChejanData"));
									return;
								}
								m_pOrdNo[iCodeInd][iQInd] = iOrdNo;
								m_pOrdP [iCodeInd][iQInd] = iOrdP;
								m_pOrdQ [iCodeInd][iQInd] = iOrdQ;
								m_pOrdCnt[iCodeInd]++;

								if (DISP_VERBOSE)
								{
									sDisp.Format(_T("Add order [%07d]"), abs(iOrdNo));
									DisplayMain(sDisp, false);
								}

								if (sOrdCat == _T("+매수"))
								{
									m_iBal -= (_int64)iOrdP * iOrdQ + (_int64)((_int64)iOrdP * iOrdQ * BUY_FEE_KIWOOM);

									if (DISP_VERBOSE)
									{
										sDisp.Format(_T("Subtract balance %12I64d"), (_int64)iOrdP * iOrdQ + (_int64)((_int64)iOrdP * iOrdQ * BUY_FEE_KIWOOM));
										DisplayMain(sDisp, false);
									}
								}
								if (sOrdCat == _T("+매수정정"))
								{
									for (int iQInd = 0; iQInd < m_pOrdCnt[iCodeInd]; iQInd++)
									{
										if (iOgOrdNo == m_pOrdNo[iCodeInd][iQInd])
										{
											int iOgOrdP = m_pOrdP[iCodeInd][iQInd];
											m_iBal -= (_int64)(iOrdP - iOgOrdP) * iOrdQ + (_int64)((_int64)(iOrdP - iOgOrdP) * iOrdQ * BUY_FEE_KIWOOM);

											if (DISP_VERBOSE)
											{
												if (iOrdP > iOgOrdP)
													sDisp.Format(_T("Subtract balance %12I64d"), (_int64)(iOrdP - iOgOrdP) * iOrdQ + (_int64)((_int64)(iOrdP - iOgOrdP) * iOrdQ * BUY_FEE_KIWOOM));
												else
													sDisp.Format(_T("Add balance %12I64d"), (_int64)(iOgOrdP - iOrdP) * iOrdQ + (_int64)((_int64)(iOgOrdP - iOrdP) * iOrdQ * BUY_FEE_KIWOOM));
												DisplayMain(sDisp, false);
											}

											break;
										}
									}
								}
							}
						}
					}
					else
					{
						//// Acknowledge receipt of new cb/cs
						//EndTr();

						// Look up original order's price as this event's iOrdP == 0
						for (int iQInd = 0; iQInd < m_pOrdCnt[iCodeInd]; iQInd++)
						{
							if (iOgOrdNo == m_pOrdNo[iCodeInd][iQInd])
							{
								iOrdP = m_pOrdP[iCodeInd][iQInd];
								if (sOrdCat == _T("매수취소"))
								{
									m_iBal += (_int64)iOrdP * iOrdQ + (_int64)((_int64)iOrdP * iOrdQ * BUY_FEE_KIWOOM);

									if (DISP_VERBOSE)
									{
										sDisp.Format(_T("Add balance %12I64d"), (_int64)iOrdP * iOrdQ + (_int64)((_int64)iOrdP * iOrdQ * BUY_FEE_KIWOOM));
										DisplayMain(sDisp, false);
									}
								}
								break;
							}
						}
					}

					if ((sOrdCat == _T("매수취소")) || (sOrdCat == _T("매도취소")) || (sOrdCat == _T("+매수정정")) || (sOrdCat == _T("-매도정정")))
					{
						for (int iQInd = 0; iQInd < m_pOrdCnt[iCodeInd]; iQInd++)
						{
							if (iOgOrdNo == m_pOrdNo[iCodeInd][iQInd])
							{
								m_pOrdQ[iCodeInd][iQInd] -= iOrdQ;
								if (m_pOrdQ[iCodeInd][iQInd] <= 0)
								{
									if (m_pOrdQ[iCodeInd][iQInd] < 0)
										DisplayMsg(_T("ChejanData: Negative original iOrdQ found"));

									// Remove original order if emptied
									for (int iQ = iQInd + 1; iQ < m_pOrdCnt[iCodeInd]; iQ++)
									{
										m_pOrdNo[iCodeInd][iQ - 1] = m_pOrdNo[iCodeInd][iQ];
										m_pOrdP [iCodeInd][iQ - 1] = m_pOrdP [iCodeInd][iQ];
										m_pOrdQ [iCodeInd][iQ - 1] = m_pOrdQ [iCodeInd][iQ];
									}
									m_pOrdCnt[iCodeInd]--;

									if (DISP_VERBOSE)
									{
										sDisp.Format(_T("Remove order [%07d]"), abs(iOgOrdNo));
										DisplayMain(sDisp, false);
									}

									break;
								}
								break;
							}
						}
					}

					if (DISP_VERBOSE)
					{
						sDisp.Format(_T("[%07d] {%06d} %8d (%2d) %s %s"), abs(iOrdNo), iCode, iOrdP, iOrdQ, sOrdCat, sOrdStat);
						DisplayMain(sDisp, false);
					}
				}
				
				if (sOrdStat == _T("체결"))
				{
					for (int iQInd = 0; iQInd < m_pOrdCnt[iCodeInd]; iQInd++)
					{
						if (iOrdNo == m_pOrdNo[iCodeInd][iQInd])
						{
							m_pOrdQ[iCodeInd][iQInd] = iOrdQ;
							if ((sOrdCat == _T("-매도")) || (sOrdCat == _T("-매도정정")))
							{
								m_iBal += (_int64)iOrdP * iDeltaQ - (_int64)((_int64)iOrdP * iDeltaQ * (SELL_FEE_KIWOOM + SELL_TAX_COMBINED));

								if (DISP_VERBOSE)
								{
									sDisp.Format(_T("Add balance %12I64d"), (_int64)iOrdP * iDeltaQ - (_int64)((_int64)iOrdP * iDeltaQ * (SELL_FEE_KIWOOM + SELL_TAX_COMBINED)));
									DisplayMain(sDisp, false);
								}
							}
							break;
						}
					}

					if (DISP_VERBOSE)
					{
						sDisp.Format(_T("[%07d] {%06d} %8d (%2d) %s %s"), abs(iOrdNo), iCode, iOrdP, iOrdQ, sOrdCat, sOrdStat);
						DisplayMain(sDisp, false);
					}
				}

				if ((iOrdQ <= 0) && (sOrdStat != _T("확인")))
				{
					if (iOrdQ < 0)
						DisplayMsg(_T("ChejanData: Negative iOrdQ found"));

					for (int iQInd = 0; iQInd < m_pOrdCnt[iCodeInd]; iQInd++)
					{
						if (iOrdNo == m_pOrdNo[iCodeInd][iQInd])
						{
							for (int iQ = iQInd + 1; iQ < m_pOrdCnt[iCodeInd]; iQ++)
							{
								m_pOrdNo[iCodeInd][iQ - 1] = m_pOrdNo[iCodeInd][iQ];
								m_pOrdP [iCodeInd][iQ - 1] = m_pOrdP [iCodeInd][iQ];
								m_pOrdQ [iCodeInd][iQ - 1] = m_pOrdQ [iCodeInd][iQ];
							}
							m_pOrdCnt[iCodeInd]--;

							if (DISP_VERBOSE)
							{
								sDisp.Format(_T("Remove order [%07d]"), abs(iOrdNo));
								DisplayMain(sDisp, false);
							}

							break;
						}
					}
				}

				break;
			}

		lockBalance.Unlock();
	}
	
	if ((sGubun == _T("1")) && lockBalance.Lock())
	{
		int     iCode    = _ttoi(theApp.m_cKHOpenAPI.GetChejanData(OP_FID_종목코드    ).Trim().Mid(1));
		int     iStkCnt  = _ttoi(theApp.m_cKHOpenAPI.GetChejanData(OP_FID_주문가능수량).Trim());

		for (unsigned int iCodeInd = 0; iCodeInd < m_nKRXCodes; iCodeInd++)
			if (iCode == m_piKRXCodes[iCodeInd])
			{
				m_pStkCnt[iCodeInd] = iStkCnt;
				break;
			}

		lockBalance.Unlock();
	}

	if (FCN_LOG) WriteFcnLog(_T("<< OnReceiveChejanData"));
}

void CSendOrderDlg::InitFIDName() // TODO: filter out unneeded FIDs
{
	m_mapFIDName.SetAt(_T("9201"), _T("계좌번호"));
	m_mapFIDName.SetAt(_T("9203"), _T("주문번호"));
	m_mapFIDName.SetAt(_T("9205"), _T("관리자사")); // 관리자사번
	m_mapFIDName.SetAt(_T("9001"), _T("종목코드"));
	m_mapFIDName.SetAt(_T("912"), _T("주문업무")); //주문업무분류
	m_mapFIDName.SetAt(_T("913"), _T("주문상태"));
	m_mapFIDName.SetAt(_T("302"), _T("  종목명"));
	m_mapFIDName.SetAt(_T("900"), _T("주문수량"));
	m_mapFIDName.SetAt(_T("901"), _T("주문가격"));
	m_mapFIDName.SetAt(_T("902"), _T("미체결수")); // 미체결수량
	m_mapFIDName.SetAt(_T("903"), _T("체결누계")); // 체결누계금액
	m_mapFIDName.SetAt(_T("904"), _T("원주문번")); // 원주문번호
	m_mapFIDName.SetAt(_T("905"), _T("주문구분"));
	m_mapFIDName.SetAt(_T("906"), _T("매매구분"));
	m_mapFIDName.SetAt(_T("907"), _T("매도수구")); // 매도수구분
	m_mapFIDName.SetAt(_T("908"), _T("주체시간")); // 주문/체결시간
	m_mapFIDName.SetAt(_T("909"), _T("체결번호"));
	m_mapFIDName.SetAt(_T("910"), _T("  체결가"));
	m_mapFIDName.SetAt(_T("911"), _T("  체결량"));
	m_mapFIDName.SetAt(_T("10"), _T("  현재가"));
	m_mapFIDName.SetAt(_T("27"), _T("매도호가")); // (최우선)매도호가
	m_mapFIDName.SetAt(_T("28"), _T("매수호가")); // (최우선)매수호가
	m_mapFIDName.SetAt(_T("914"), _T("단위결가")); // 단위체결가
	m_mapFIDName.SetAt(_T("915"), _T("단위결량")); // 단위체결량
	m_mapFIDName.SetAt(_T("938"), _T("매매수수")); // 당일매매수수료
	m_mapFIDName.SetAt(_T("939"), _T("매매세금")); // 당일매매세금
	m_mapFIDName.SetAt(_T("919"), _T("거부사유"));
	m_mapFIDName.SetAt(_T("920"), _T("화면번호"));
	m_mapFIDName.SetAt(_T("921"), _T("터미널번")); // 터미널번호
	m_mapFIDName.SetAt(_T("922"), _T("신용구분")); // 신용구분(실시간 체결용)
	m_mapFIDName.SetAt(_T("923"), _T("  대출일")); // 대출일(실시간 체결용)

	m_mapFIDName.SetAt(_T("917"), _T("신용구분"));
	m_mapFIDName.SetAt(_T("916"), _T("  대출일"));
	m_mapFIDName.SetAt(_T("930"), _T("보유수량"));
	m_mapFIDName.SetAt(_T("931"), _T("매입단가"));
	m_mapFIDName.SetAt(_T("932"), _T("총매입가"));
	m_mapFIDName.SetAt(_T("933"), _T("주문가수")); // 주문가능수량
	m_mapFIDName.SetAt(_T("945"), _T("당매수량")); // 당일순매수수량
	m_mapFIDName.SetAt(_T("946"), _T("도수구분")); // 매도/매수구분
	m_mapFIDName.SetAt(_T("950"), _T("당매도손")); // 당일총매도손일
	m_mapFIDName.SetAt(_T("951"), _T("예수금"));
	m_mapFIDName.SetAt(_T("307"), _T("기준가"));
	m_mapFIDName.SetAt(_T("8019"), _T("  손익율"));
	m_mapFIDName.SetAt(_T("957"), _T("신용금액"));
	m_mapFIDName.SetAt(_T("958"), _T("신용이자"));
	m_mapFIDName.SetAt(_T("918"), _T("만기일"));
	m_mapFIDName.SetAt(_T("990"), _T("손익유가")); // 당일실현손익(유가
	m_mapFIDName.SetAt(_T("991"), _T("익률유가")); // 당일실현손익률(유가
	m_mapFIDName.SetAt(_T("992"), _T("손익신용")); // 당일실현손익(신용
	m_mapFIDName.SetAt(_T("993"), _T("익률신용")); // 당일실현손익률(신용
	m_mapFIDName.SetAt(_T("959"), _T("담대출수")); // 담보대출수량

	m_mapFIDName.SetAt(_T("397"), _T("파생단위")); // 파생상품거래단위
	m_mapFIDName.SetAt(_T("305"), _T("  상한가"));
	m_mapFIDName.SetAt(_T("306"), _T("  하한가"));
}

UINT CSendOrderDlg::TCPServerThread(LPVOID pParam)
{
	char buf[TCP_BUF_SIZE], msg[TCP_BUF_SIZE];
	struct sockaddr_in serv_addr, client_addr;
	socklen_t addr_size = sizeof(client_addr);
	CString sText;

    struct WSAData* wd = (struct WSAData*)malloc(sizeof(struct WSAData));
    WSAStartup(MAKEWORD(2, 0), wd);
    free(wd);

	pThis->m_sockServ = pThis->m_sockConn = -1;

	if ((pThis->m_sockServ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
	{
		pThis->DisplayMsg(_T("TCPServer: Socket creation error"));
		return 0;
	}

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(pThis->m_iTCPPort);

    if (bind(pThis->m_sockServ, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
    	pThis->DisplayMsg(_T("TCPServer: Socket bind error"));
    	return 0;
    }

	listen(pThis->m_sockServ, TCP_BACKLOG_SIZE);

	bool bWaitConn = true;
	while (1)
	{
		while (bWaitConn)
		{
			sText.Format(_T("TCPServer: Listening on port %d"), pThis->m_iTCPPort);
			pThis->DisplayMsg(sText);

			if(-1 == (pThis->m_sockConn = accept(pThis->m_sockServ, (struct sockaddr *)&client_addr, &addr_size))) // blocking
			{
				pThis->DisplayMsg(_T("TCPServer: Failed accepting client"));
			}
			else
			{
				pThis->DisplayMsg(_T("TCPServer: Querying password from client"));

				memset(buf, 0, TCP_BUF_SIZE);
				if (recv(pThis->m_sockConn, buf, TCP_BUF_SIZE, 0) > 0) // blocking
				{
					CString sPassword(buf);
					sPassword = sPassword.SpanExcluding(_T("\r\n"));

					if (sPassword == _T(TCP_PASSWORD))
					{
						pThis->DisplayMsg(_T("TCPServer: Client connected"));
						pThis->m_bConnected = true;
						if (pThis->m_bLoggedIn && !pThis->m_bStarted)
							pThis->m_btStart.EnableWindow(TRUE);
						bWaitConn = false;
					}
					else
					{
						pThis->DisplayMsg(_T("TCPServer: Incorrect password"));
						closesocket(pThis->m_sockConn);
						pThis->m_sockConn = -1;
					}
				}
				else
				{
					pThis->DisplayMsg(_T("TCPServer: Client disconnected"));
					closesocket(pThis->m_sockConn);
					pThis->m_sockConn = -1;
				}
			}
		}

		msg[0] = '\0';
		while (1)
		{
			memset(buf, 0, TCP_BUF_SIZE);
			if (recv(pThis->m_sockConn, buf, TCP_BUF_SIZE, 0) > 0) // blocking
			{
				strcat_s (msg, buf);
				size_t szMsg = strlen (msg);
				if (msg[szMsg - 1] == '\n')
				{
					pThis->SendOrder(msg);
					break;
				}
			}
			else
			{
				pThis->DisplayMsg(_T("TCPServer: Client disconnected"));
				pThis->m_bConnected = false;
			
				closesocket(pThis->m_sockConn);
				pThis->m_sockConn = -1;
				bWaitConn = true;

				break;
			}
		}
    }

	return 0;
}

void CSendOrderDlg::SendOrder(LPCTSTR _sCommand)
{
	if (FCN_LOG) WriteFcnLog(_T(">> SendOrder"));

	if (!m_bStarted && !m_bInitializing)
	{
		DisplayMsg(_T("SendOrder: Cannot send order before full initialization"));
		if (FCN_LOG) WriteFcnLog(_T("<< SendOrder"));
		return;
	}

	CString sCommand(_sCommand);
	CString sLine, sWord, sOrdType, sCode, sOrdNo;
	long lOrderType, lPrice, lQuant, lModPrice;
	CString sRQName = _T("order stock");
	CSingleLock lockBalance(&m_mutexBalance);
	CSingleLock lockRealData(&m_mutexRealData);
	
	m_bSORunning = true;

	for (int iLine = 0; AfxExtractSubString(sLine, sCommand, iLine, _T('\n')); iLine++)
	{
		if (!m_bSORunning)
			break;

		sLine = sLine.SpanExcluding(_T("\r"));
		if (sLine.IsEmpty())
			continue;

		lModPrice = 0;

		bool bFail = true;
		for (int iWord = 0; AfxExtractSubString(sWord, sLine, iWord, _T(' ')); iWord++)
		{
			if (iWord == 0)
			{
				sOrdType = sWord;
				lOrderType = 0;
				if (sWord == _T("b") ) lOrderType = 1;
				if (sWord == _T("s") ) lOrderType = 2;
				if (sWord == _T("cb")) lOrderType = 3;
				if (sWord == _T("cs")) lOrderType = 4;
				if (sWord == _T("mb")) lOrderType = 5;
				if (sWord == _T("ms")) lOrderType = 6;
				if ((sWord == _T("ca")) || (sWord == _T("sa")))
				{
					bFail = false;
					break;
				}
				if (lOrderType == 0)
					break;
			}
			if (iWord == 1)
			{
				if ((sWord.SpanIncluding(_T("0123456789")) != sWord) || (sWord.GetLength() != 6))
					break;
				sCode = sWord;
			}
			if (iWord == 2)
			{
				if (sWord.SpanIncluding(_T("0123456789")) != sWord)
					break;
				lPrice = _ttoi(sWord);
				if (lPrice <= 0)
					break;
			}
			if (iWord == 3)
			{
				if (sWord.SpanIncluding(_T("0123456789")) != sWord)
					break;
				lQuant = _ttoi(sWord); // lQuant == 0 means all available quantity at the given price
				if ((lOrderType != 5) && (lOrderType != 6))
					bFail = false;
			}
			if (iWord == 4)
			{
				if ((lOrderType != 5) && (lOrderType != 6))
				{
					bFail = true;
					break;
				}
				if (sWord.SpanIncluding(_T("0123456789")) != sWord)
					break;
				lModPrice = _ttoi(sWord);
				if (lModPrice <= 0)
					break;
				bFail = false;
			}
			if (iWord >= 5)
			{
				bFail = true;
				break;
			}
		}

		if (bFail)
		{
			sWord.Format(_T("Invalid order: %s"), sLine);
			DisplayMsg(sWord);
			continue;
		}

		if (sWord == _T("ca")) // Cancel all orders
		{
			int iOrdP, iOrdQ;
			for (unsigned int i = 0; i < m_nKRXCodes; i++)
			{
				sCode.Format(_T("%06d"), m_piKRXCodes[i]);
				for (int j = m_pOrdCnt[i] - 1; j >= 0; j--)
				{
					if (lockBalance.Lock())
					{
						sOrdNo.Format(_T("%07d"), abs(m_pOrdNo[i][j]));
						sOrdType.Format(_T("%2s"), (m_pOrdNo[i][j] > 0 ? _T("cb") : _T("cs")));
						lOrderType = (m_pOrdNo[i][j] > 0 ? 3 : 4);
						iOrdP = m_pOrdP[i][j];
						iOrdQ = m_pOrdQ[i][j];
						lockBalance.Unlock();
					}

					if (DISP_VERBOSE)
					{
						sWord.Format(_T("Suborder[%2d][%2d]: [%s] %2s {%s} %8d (%2d)"), i, j, sOrdNo, sOrdType, sCode, iOrdP, iOrdQ);
						DisplayMain(sWord);
					}

					long lRet = 0;
					do {
						if (lRet == OP_ERR_ORD_OVERFLOW)
						{
							DisplayMsg(_T("SendOrder: Overflow"));
							Sleep(OVERFLOW_WAIT_MS);
						}
						InitTr();
						lRet = theApp.m_cKHOpenAPI.SendOrder(sRQName, m_sScrNo, m_sAccNo, lOrderType, sCode, iOrdQ, iOrdP, _T("00"), sOrdNo);
					} while (lRet == OP_ERR_ORD_OVERFLOW);

					WaitTr();
				}
			}
			continue;
		}

		if (sWord == _T("sa")) // Sell all stocks
		{
			int iCnt = 0;
			int iOrdP, iOrdQ;
			sOrdNo.Format(_T(""));
			lOrderType = 2;
			for (unsigned int iC = 0; iC < m_nKRXCodes; iC++)
			{
				sCode.Format(_T("%06d"), m_piKRXCodes[iC]);
				if (m_pStkCnt[iC] > 0)
				{
					if (lockBalance.Lock() && lockRealData.Lock())
					{
						iOrdP = m_pTbP[iC][10];
						iOrdQ = m_pStkCnt[iC];
						lockRealData.Unlock();
						lockBalance.Unlock();
					}

					if (DISP_VERBOSE)
					{
						sWord.Format(_T("Suborder[%2d]: %2s {%s} %8d (%2d)"), iCnt++, _T("s"), sCode, iOrdP, iOrdQ);
						DisplayMain(sWord);
					}

					long lRet = 0;
					do {
						if (lRet == OP_ERR_ORD_OVERFLOW)
						{
							DisplayMsg(_T("SendOrder: Overflow"));
							Sleep(OVERFLOW_WAIT_MS);
						}
						InitTr();
						lRet = theApp.m_cKHOpenAPI.SendOrder(sRQName, m_sScrNo, m_sAccNo, lOrderType, sCode, iOrdQ, iOrdP, _T("00"), sOrdNo);
					} while (lRet == OP_ERR_ORD_OVERFLOW);

					WaitTr();
				}
			}
			continue;
		}

		// Filter out incorrect code & price
		int iCode = _ttoi(sCode);
		unsigned int iCodeInd;
		for (iCodeInd = 0; iCodeInd < m_nKRXCodes; iCodeInd++)
			if (iCode == m_piKRXCodes[iCodeInd])
				break;
		if (iCodeInd == m_nKRXCodes)
		{
			sWord.Format(_T("SendOrder: Code {%s} not found"), sCode);
			DisplayMsg(sWord);
			continue;
		}
		if ((!IsPriceValid(lPrice)) || ((lModPrice > 0) && (!IsPriceValid(lModPrice))) || (lPrice == lModPrice))
		{
			sWord.Format(_T("SendOrder: Price %8d is invalid"), (IsPriceValid(lPrice) ? lModPrice : lPrice));
			DisplayMsg(sWord);
			continue;
		}

		if (DISP_VERBOSE)
		{
			DisplayMain(_T("-----------------------------------"));

			if (lModPrice == 0)
				sWord.Format(_T("Order: %2s {%s} %8d (%2d)"), sOrdType, sCode, lPrice, lQuant);
			else
				sWord.Format(_T("Order: %2s {%s} %8d (%2d) %8d"), sOrdType, sCode, lPrice, lQuant, lModPrice);
			DisplayMain(sWord);
		}

		// Build order sequence
		if ((lOrderType == 1) || (lOrderType == 2))
		{
			if (lockBalance.Lock())
			{
				// Correct lQuant for insufficient fund (sOrdType == b)
				if (lOrderType == 1)
				{
					_int64 iMaxQ64 = (_int64)floor(m_iBal / (lPrice * (1 + BUY_FEE_KIWOOM)));
					if (iMaxQ64 > 0x7FFFFFFF) iMaxQ64 = 0x7FFFFFFF;
					int iMaxQ = (int)iMaxQ64;

					if (lQuant == 0)
						lQuant = iMaxQ;
					else
						lQuant = (lQuant > iMaxQ ? iMaxQ : lQuant);
				}

				// Correct lQuant for insufficient stock (sOrdType == s)
				if (lOrderType == 2)
				{
					if (lQuant == 0)
						lQuant = m_pStkCnt[iCodeInd];
					else
						lQuant = (lQuant > m_pStkCnt[iCodeInd] ? m_pStkCnt[iCodeInd] : lQuant);
				}				

				lockBalance.Unlock();
			}

			if (lQuant > 0)
			{
				sOrdNo.Format(_T(""));
				long lRet = 0;
				do {
					if (lRet == OP_ERR_ORD_OVERFLOW)
					{
						DisplayMsg(_T("SendOrder: Overflow"));
						Sleep(OVERFLOW_WAIT_MS);
					}
					InitTr();
					lRet = theApp.m_cKHOpenAPI.SendOrder(sRQName, m_sScrNo, m_sAccNo, lOrderType, sCode, lQuant, lPrice, _T("00"), sOrdNo);
				} while (lRet == OP_ERR_ORD_OVERFLOW);

				WaitTr();
			}
		}
		else
		{
			int nQuantLeft = lQuant;
			int nQOrd = 0;
			int iQOrdNo[ORD_Q_SIZE];
			int iQOrdQ [ORD_Q_SIZE];
			bool isOrdBuy = ((lOrderType == 3) || (lOrderType == 5));
			bool isLimited = true;

			if (lockBalance.Lock())
			{
				// Correct nQuantLeft for insufficient fund (sOrdType == mb)
				if ((lOrderType == 5) && (lModPrice > lPrice))
				{
					_int64 iMaxQ64 = (_int64)floor(m_iBal / ((lModPrice - lPrice) * (1 + BUY_FEE_KIWOOM)));
					if (iMaxQ64 > 0x7FFFFFFF) iMaxQ64 = 0x7FFFFFFF;
					int iMaxQ = (int)iMaxQ64;

					if (lQuant == 0)
						nQuantLeft = iMaxQ;
					else
						nQuantLeft = (nQuantLeft > iMaxQ ? iMaxQ : nQuantLeft);
				}
				else
					if (lQuant == 0)
						isLimited = false;

				for (int i = m_pOrdCnt[iCodeInd] - 1; i >= 0; i--)
				{
					if ((lPrice == m_pOrdP[iCodeInd][i]) && ((isOrdBuy && (m_pOrdNo[iCodeInd][i] > 0)) || (!isOrdBuy && (m_pOrdNo[iCodeInd][i] < 0))))
					{
						iQOrdNo[nQOrd] = m_pOrdNo[iCodeInd][i];
						iQOrdQ [nQOrd] = (nQuantLeft > m_pOrdQ[iCodeInd][i] ? m_pOrdQ[iCodeInd][i] : nQuantLeft);
						nQuantLeft -= iQOrdQ[nQOrd];
						nQOrd++;
						if (isLimited && (nQuantLeft <= 0))
							break;
					}
				}

				lockBalance.Unlock();
			}

			if ((DISP_VERBOSE) && (nQOrd == 0))
			{
				sWord.Format(_T("SendOrder: No order found at price %d"), lPrice);
				DisplayMsg(sWord);
			}

			for (int i = 0; i < nQOrd; i++)
			{
				if (DISP_VERBOSE)
				{
					sWord.Format(_T("Suborder[%2d]: [%07d] %2s {%s} %8d (%2d)"), i, abs(iQOrdNo[i]), sOrdType, sCode, (lModPrice > 0 ? lModPrice : lPrice), iQOrdQ[i]);
					DisplayMain(sWord);
				}

				sOrdNo.Format(_T("%07d"), abs(iQOrdNo[i]));
				long lRet = 0;
				do {
					if (lRet == OP_ERR_ORD_OVERFLOW)
					{
						DisplayMsg(_T("SendOrder: Overflow"));
						Sleep(OVERFLOW_WAIT_MS);
					}
					InitTr();
					lRet = theApp.m_cKHOpenAPI.SendOrder(sRQName, m_sScrNo, m_sAccNo, lOrderType, sCode, iQOrdQ[i], (lModPrice > 0 ? lModPrice : lPrice), _T("00"), sOrdNo);
				} while (lRet == OP_ERR_ORD_OVERFLOW);

				WaitTr();
			}
		}
	}

	m_bSORunning = false;
	if (FCN_LOG) WriteFcnLog(_T("<< SendOrder"));
}

int CSendOrderDlg::DisplayBalance()
{
	if (FCN_LOG) WriteFcnLog(_T(">> DisplayBalance"));

	CString sRQName, sTRCode;
	long lRet, lContTr;
	CSingleLock lockBalance(&m_mutexBalance);
	CSingleLock lockRealData(&m_mutexRealData);

	if (m_listBal.GetCount())
		while (m_listBal.DeleteString(0)); // empty

	if (m_bInitializing || (m_ckEnableTR.GetCheck() == BST_CHECKED))
	{
		m_listBal.AddString(_T("Tr results"));
		m_listBal.AddString(_T("-----------------------------------"));

		sRQName = _T("account balance");
		sTRCode = _T("OPW00001");
		lRet = 0;
		do {
			if (lRet == OP_ERR_SISE_OVERFLOW)
			{
				DisplayMsg(_T("DisplayBalance: Overflow"));
				Sleep(OVERFLOW_WAIT_MS);
			}
			InitTr();
			theApp.m_cKHOpenAPI.SetInputValue(_T("계좌번호"), m_sAccNo);
			theApp.m_cKHOpenAPI.SetInputValue(_T("비밀번호"), _T(""));
			theApp.m_cKHOpenAPI.SetInputValue(_T("비밀번호입력매체구분"), _T("00"));
			theApp.m_cKHOpenAPI.SetInputValue(_T("조회구분"), _T("1"));
			lRet = theApp.m_cKHOpenAPI.CommRqData(sRQName, sTRCode, 0, m_sScrNo);
		} while (lRet == OP_ERR_SISE_OVERFLOW);

		if (m_bInitializing && (lRet != OP_ERR_NONE)) // password not entered
		{
			if (FCN_LOG) WriteFcnLog(_T("<< DisplayBalance"));
			return -1;
		}
	
		WaitTr();


		sRQName = _T("stock list");
		sTRCode = _T("OPW00018");
		lContTr = 0;
		do {
			lRet = 0;
			do {
				if (lRet == OP_ERR_SISE_OVERFLOW)
				{
					DisplayMsg(_T("DisplayStockList: Overflow"));
					Sleep(OVERFLOW_WAIT_MS);
				}
				InitTr();
				theApp.m_cKHOpenAPI.SetInputValue(_T("계좌번호"), m_sAccNo);
				theApp.m_cKHOpenAPI.SetInputValue(_T("비밀번호"), _T(""));
				theApp.m_cKHOpenAPI.SetInputValue(_T("비밀번호입력매체구분"), _T("00"));
				theApp.m_cKHOpenAPI.SetInputValue(_T("조회구분"), _T("2"));
				lRet = theApp.m_cKHOpenAPI.CommRqData(sRQName, sTRCode, lContTr, m_sScrNo);
			} while (lRet == OP_ERR_SISE_OVERFLOW);
			
			lContTr = WaitTr();
		} while (lContTr == 2);


		sRQName = _T("order list");
		sTRCode = _T("OPT10075");
		lContTr = 0;
		do {
			lRet = 0;
			do {
				if (lRet == OP_ERR_SISE_OVERFLOW)
				{
					DisplayMsg(_T("DisplayOrderList: Overflow"));
					Sleep(OVERFLOW_WAIT_MS);
				}
				InitTr();
				theApp.m_cKHOpenAPI.SetInputValue(_T("계좌번호"), m_sAccNo);
				theApp.m_cKHOpenAPI.SetInputValue(_T("체결구분"), _T("1")); // 1 for 미체결 only
				theApp.m_cKHOpenAPI.SetInputValue(_T("매매구분"), _T("0"));
				lRet = theApp.m_cKHOpenAPI.CommRqData(sRQName, sTRCode, lContTr, m_sScrNo);
			} while (lRet == OP_ERR_SISE_OVERFLOW);

			lContTr = WaitTr();
		} while (lContTr == 2);

		// reverse order ordering
		if (m_bInitializing && lockBalance.Lock())
		{
			for (unsigned int iC = 0; iC < m_nKRXCodes; iC++)
			{
				for (int iQ = 0; iQ < (m_pOrdCnt[iC] / 2); iQ++)
				{
					int tempNo = m_pOrdNo[iC][iQ];
					int tempP  = m_pOrdP [iC][iQ];
					int tempQ  = m_pOrdQ [iC][iQ];
					m_pOrdNo[iC][iQ] = m_pOrdNo[iC][m_pOrdCnt[iC] - 1 - iQ];
					m_pOrdP [iC][iQ] = m_pOrdP [iC][m_pOrdCnt[iC] - 1 - iQ];
					m_pOrdQ [iC][iQ] = m_pOrdQ [iC][m_pOrdCnt[iC] - 1 - iQ];
					m_pOrdNo[iC][m_pOrdCnt[iC] - 1 - iQ] = tempNo;
					m_pOrdP [iC][m_pOrdCnt[iC] - 1 - iQ] = tempP ;
					m_pOrdQ [iC][m_pOrdCnt[iC] - 1 - iQ] = tempQ ;
				}
			}
			lockBalance.Unlock();
		}

		m_listBal.AddString(_T("-----------------------------------"));
		m_listBal.AddString(_T(""));
	}

	m_listBal.AddString(_T("Internal state"));
	m_listBal.AddString(_T("-----------------------------------"));

	CString sDisp;
	if (lockBalance.Lock() && lockRealData.Lock())
	{
		sDisp.Format(_T("Balance %12I64d (pure)"), m_iBal);
		m_listBal.AddString(sDisp);

		_int64 iBalTot = m_iBal;
		for (unsigned int i = 0; i < m_nKRXCodes; i++)
		{
			iBalTot += (_int64)m_pTbP[i][10] * m_pStkCnt[i] - (_int64)((_int64)m_pTbP[i][10] * m_pStkCnt[i] * (SELL_FEE_KIWOOM + SELL_TAX_COMBINED));
			for (int j = 0; j < m_pOrdCnt[i]; j++)
			{
				if (m_pOrdNo[i][j] > 0)
					iBalTot += (_int64)m_pOrdP[i][j] * m_pOrdQ[i][j] + (_int64)((_int64)m_pOrdP[i][j] * m_pOrdQ[i][j] * BUY_FEE_KIWOOM);
				else
					iBalTot += (_int64)m_pTbP[i][10] * m_pOrdQ[i][j] - (_int64)((_int64)m_pTbP[i][10] * m_pOrdQ[i][j] * (SELL_FEE_KIWOOM + SELL_TAX_COMBINED));;
			}
		}
		sDisp.Format(_T("        %12I64d (est. total)"), iBalTot);
		m_listBal.AddString(sDisp);

		m_listBal.AddString(_T(""));
		m_listBal.AddString(_T("Stock list (exc. ord.)"));
		for (unsigned int i = 0; i < m_nKRXCodes; i++)
		{
			sDisp.Format(_T("             {%06d} %8.0f (%2d)"), m_piKRXCodes[i], m_pPr[i], m_pStkCnt[i]);
			m_listBal.AddString(sDisp);
		}
		m_listBal.AddString(_T(""));

		int nOrder = 0;
		for (unsigned int i = 0; i < m_nKRXCodes; i++)
			nOrder += m_pOrdCnt[i];

		sDisp.Format(_T("Order list (%2d)"), nOrder);
		m_listBal.AddString(sDisp);
		for (unsigned int i = 0; i < m_nKRXCodes; i++)
			if (m_pOrdCnt[i] > 0)
				for (int j = 0; j < m_pOrdCnt[i]; j++)
				{
					sDisp.Format(_T("[%07d] %2s {%06d} %8d (%2d)"), abs(m_pOrdNo[i][j]), (m_pOrdNo[i][j] > 0 ? _T("b") : _T("s")), m_piKRXCodes[i], m_pOrdP[i][j], m_pOrdQ[i][j]);
					m_listBal.AddString(sDisp);
				}

		lockRealData.Unlock();
		lockBalance.Unlock();
	}

	m_listBal.AddString(_T("-----------------------------------"));


	m_listMain.SetCurSel(m_listMain.GetCount() - 1);
	m_listMain.SetCurSel(-1);
	m_listMsg.SetCurSel(m_listMsg.GetCount() - 1);
	m_listMsg.SetCurSel(-1);

	DisplayUpdatedTime();

	if (FCN_LOG) WriteFcnLog(_T("<< DisplayBalance"));
	return 0;
}

void CSendOrderDlg::InitTr()
{
	if (FCN_LOG) WriteFcnLog(_T(">> InitTr"));

	static ULONGLONG tQ[TR_RATE_CAP] = {0};
	ULONGLONG tCur = GetTickCount64(); // time from system start in milliseconds

	m_bLockTr = true;

	if (tCur - tQ[0] <= 1000)
	{
		do {
			Sleep(BLOCK_WAIT_MS);
			tCur = GetTickCount64();
		} while (tCur - tQ[0] <= 1000);
	}

	for (int i = 0; i < TR_RATE_CAP - 1; i++)
		tQ[i] = tQ[i + 1];

	tQ[TR_RATE_CAP - 1] = tCur;

	if (FCN_LOG) WriteFcnLog(_T("<< InitTr"));
}

long CSendOrderDlg::WaitTr()
{
	if (FCN_LOG) WriteFcnLog(_T(">> WaitTr"));

	int iRet = 0;
	CString sDisp;
	sDisp.Format(_T("%2d"), ++m_nTrCntThisTick);
	((CWnd*)GetDlgItem(IDC_STATIC_DISP_TRCNT))->SetWindowText(sDisp);

	ULONGLONG tStart = GetTickCount64();
	BOOL bDoingBackgroundProcessing = TRUE;
	while (bDoingBackgroundProcessing) 
	{ 
		MSG msg;
		//while (::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) 
		//{ 
		//	if (!AfxGetApp()->PumpMessage()) 
		//	{ 
		//		bDoingBackgroundProcessing = FALSE; 
		//		::PostQuitMessage(0); 
		//		break; 
		//	} 
		//} 
		//// let MFC do its idle processing
		//LONG lIdle = 0;
		//while (AfxGetApp()->OnIdle(lIdle++ ))
		//	;  
		// Perform some background processing here  
		// using another call to OnIdle

		while (::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
			AfxGetThread()->PumpMessage();

		if (!m_bLockTr)
			break;
		Sleep(BLOCK_WAIT_MS);
	}

	if (FCN_LOG) WriteFcnLog(_T("<< WaitTr"));
	return m_lContTr;
}

void CSendOrderDlg::EndTr(long lPrevNext)
{
	if (FCN_LOG) WriteFcnLog(_T(">> EndTr"));

	m_lContTr = lPrevNext;
	m_bLockTr = false;

	if (FCN_LOG) WriteFcnLog(_T("<< EndTr"));
}

void CSendOrderDlg::FixTbQuantization(int *inP, int *inQ)
{
	int i, j, q;
	int outP[20], outQ[20];

	outP[9] = inP[9];
	outQ[9] = inQ[9];
	for (i = 9; i >= 1; i--)
	{
		q =                       (outP[i] <   1000) *    1 +
            (outP[i] >=   1000) * (outP[i] <   5000) *    5 +
            (outP[i] >=   5000) * (outP[i] <  10000) *   10 +
            (outP[i] >=  10000) * (outP[i] <  50000) *   50 +
            (outP[i] >=  50000) * (outP[i] < 100000) *  100 +
            (outP[i] >= 100000) * (outP[i] < 500000) *  500 +
            (outP[i] >= 500000)                      * 1000;
		for (j = 0; j < 9; j++)
			if (inP[j] == outP[i] + q)
				break;
		if (j == 9)
		{
			outP[i - 1] = outP[i] + q;
			outQ[i - 1] = 0;
		}
		else
		{
			outP[i - 1] = inP[j];
			outQ[i - 1] = inQ[j];
		}
	}
	for (i = 9; i <= 18; i++)
	{
		q =                      (outP[i] <=   1000) *    1 +
            (outP[i] >   1000) * (outP[i] <=   5000) *    5 +
            (outP[i] >   5000) * (outP[i] <=  10000) *   10 +
            (outP[i] >  10000) * (outP[i] <=  50000) *   50 +
            (outP[i] >  50000) * (outP[i] <= 100000) *  100 +
            (outP[i] > 100000) * (outP[i] <= 500000) *  500 +
            (outP[i] > 500000)                       * 1000;
		for (j = 10; j < 20; j++)
			if (inP[j] == outP[i] - q)
				break;
		if (j == 20)
		{
			outP[i + 1] = outP[i] - q;
			outQ[i + 1] = 0;
		}
		else
		{
			outP[i + 1] = inP[j];
			outQ[i + 1] = inQ[j];
		}
	}
	for (i = 0; i < 20; i++)
	{
		inP[i] = outP[i];
		inQ[i] = outQ[i];
	}
}

VOID CALLBACK CSendOrderDlg::TimerCallback(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	if (FCN_LOG) pThis->WriteFcnLog(_T(">> TimerCallback"));

	static int tFloorLast = -9 * 3600;
	int tFloor;
	CSingleLock lockRealData(&pThis->m_mutexRealData);
	CSingleLock lockBalance(&pThis->m_mutexBalance);
	CString sOut, sLine, sOrder;

	pThis->m_bSORunning = false;

	if (pThis->m_bConnected && !pThis->m_bStarted)
	{
		pThis->m_bStarted = true;
		pThis->DisplayMsg(_T("RefreshTimer: Started"));
	}

	if (pThis->m_bStarted && lockRealData.Lock() && lockBalance.Lock())
	{
		if (FCN_LOG) pThis->WriteFcnLog(_T(">> TimerCallback lock"));

		do {
			CTime tObj = CTime::GetCurrentTime();
			int t = (tObj.GetHour() - 9) * 3600 + tObj.GetMinute() * 60 + tObj.GetSecond();
			tFloor = (t / REFRESH_INTERVAL_SEC) * REFRESH_INTERVAL_SEC;
			if (tFloor == tFloorLast)
				Sleep(BLOCK_WAIT_MS);
		} while (tFloor == tFloorLast);
		tFloorLast = tFloor;

		sOut.Format(_T("/*\nb %d %I64d\n"), tFloor, pThis->m_iBal);

		for (unsigned int i = 0; i < pThis->m_nKRXCodes; i++)
		{
			// RealData
			if (pThis->m_pSumQ[i] != 0)
				pThis->m_pPr[i] = (float)((double)pThis->m_pSumPQ[i] / (double)pThis->m_pSumQ[i]);
			pThis->FixTbQuantization(&pThis->m_pTbP[i][0], &pThis->m_pTbQ[i][0]);
			
			sLine.Format(_T("d %06d %.5e %I64d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n"),
							pThis->m_piKRXCodes[i], pThis->m_pPr[i], pThis->m_pSumQ[i],
							pThis->m_pTbP[i][ 0], pThis->m_pTbP[i][ 1], pThis->m_pTbP[i][ 2], pThis->m_pTbP[i][ 3], pThis->m_pTbP[i][ 4],
							pThis->m_pTbP[i][ 5], pThis->m_pTbP[i][ 6], pThis->m_pTbP[i][ 7], pThis->m_pTbP[i][ 8], pThis->m_pTbP[i][ 9],
							pThis->m_pTbP[i][10], pThis->m_pTbP[i][11], pThis->m_pTbP[i][12], pThis->m_pTbP[i][13], pThis->m_pTbP[i][14],
							pThis->m_pTbP[i][15], pThis->m_pTbP[i][16], pThis->m_pTbP[i][17], pThis->m_pTbP[i][18], pThis->m_pTbP[i][19],
							pThis->m_pTbQ[i][ 0], pThis->m_pTbQ[i][ 1], pThis->m_pTbQ[i][ 2], pThis->m_pTbQ[i][ 3], pThis->m_pTbQ[i][ 4],
							pThis->m_pTbQ[i][ 5], pThis->m_pTbQ[i][ 6], pThis->m_pTbQ[i][ 7], pThis->m_pTbQ[i][ 8], pThis->m_pTbQ[i][ 9],
							pThis->m_pTbQ[i][10], pThis->m_pTbQ[i][11], pThis->m_pTbQ[i][12], pThis->m_pTbQ[i][13], pThis->m_pTbQ[i][14],
							pThis->m_pTbQ[i][15], pThis->m_pTbQ[i][16], pThis->m_pTbQ[i][17], pThis->m_pTbQ[i][18], pThis->m_pTbQ[i][19]);
			sOut.Append(sLine);

			pThis->m_pSumPQ[i] = pThis->m_pSumQ[i] = 0;	


			// StkCnt and order queue
			sLine.Format(_T("o %06d %d"), pThis->m_piKRXCodes[i], pThis->m_pStkCnt[i]);

			// merge orders of the same price
			int nOrd = pThis->m_pOrdCnt[i];
			int pP[ORD_Q_SIZE], pQ[ORD_Q_SIZE];
			for (int iQ = 0; iQ < nOrd; iQ++)
			{
				pP[iQ] = pThis->m_pOrdP[i][iQ];
				pQ[iQ] = pThis->m_pOrdQ[i][iQ] * (pThis->m_pOrdNo[i][iQ] > 0 ? 1 : -1); // + for buy, - for sell
			}
			for (int iQ = 0; iQ < nOrd; iQ++) // nOrd is varying inside loop
			{
				for (int jQ = iQ + 1; jQ < nOrd; jQ++)
					if ((pP[iQ] == pP[jQ]) && (((pQ[iQ] > 0) && (pQ[jQ] > 0)) || ((pQ[iQ] < 0) && (pQ[jQ] < 0))))
					{
						pQ[iQ] += pQ[jQ];
						for (int kQ = jQ + 1; kQ < nOrd; kQ++)
						{
							pP[kQ - 1] = pP[kQ];
							pQ[kQ - 1] = pQ[kQ];
						}
						nOrd--;
					}
			}
			// nOrd now represents total number of different prices in queue
			for (int iQ = 0; iQ < nOrd; iQ++)
			{
				sOrder.Format(_T(" %d %+d"), pP[iQ], pQ[iQ]);
				sLine.Append(sOrder);
			}
			sLine.Append(_T("\n"));
			sOut.Append(sLine);
		}
		sOut.Append(_T("*/\n"));

		lockBalance.Unlock();
		lockRealData.Unlock();
		if (FCN_LOG) pThis->WriteFcnLog(_T("<< TimerCallback lock"));

		pThis->m_nTrCntThisTick = 0;
		((CWnd*)pThis->GetDlgItem(IDC_STATIC_DISP_TRCNT))->SetWindowText(_T(" 0"));

		if (pThis->m_sockConn != -1)
			send(pThis->m_sockConn, sOut, sOut.GetLength(), 0);

		pThis->DisplayBalance();
	}

	if (FCN_LOG) pThis->WriteFcnLog(_T("<< TimerCallback"));
}

bool CSendOrderDlg::IsPriceValid(int p)
{
	int	q =                 (p <   1000) *      1 +
            (p >=   1000) * (p <   5000) *      5 +
            (p >=   5000) * (p <  10000) *     10 +
            (p >=  10000) * (p <  50000) *     50 +
            (p >=  50000) * (p < 100000) *    100 +
            (p >= 100000) * (p < 500000) *    500 +
            (p >= 500000)                *   1000;
	int b =                 (p <   1000) *      0 +
            (p >=   1000) * (p <   5000) *   1000 +
            (p >=   5000) * (p <  10000) *   5000 +
            (p >=  10000) * (p <  50000) *  10000 +
            (p >=  50000) * (p < 100000) *  50000 +
            (p >= 100000) * (p < 500000) * 100000 +
            (p >= 500000)                * 500000;
	return (p > 0) && (((p - b) % q) == 0);
}

void CSendOrderDlg::WriteFcnLog(LPCTSTR sText)
{
	static int iDepth = 0;
	CSingleLock lockFcnLog(&m_mutexFcnLog);

	if (sText[0] == '<')
		iDepth--;
	if (lockFcnLog.Lock())
	{
		for (int i = 0; i < iDepth; i++)
			fprintf(m_pfFcnLog, "\t");
		fprintf(m_pfFcnLog, _T("%s\n"), sText);
		fflush(m_pfFcnLog);
		lockFcnLog.Unlock();
	}
	if (sText[0] == '>')
		iDepth++;
}