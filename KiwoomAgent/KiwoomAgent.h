/*
   Copyright 2016 Hosang Yoon

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

// KiwoomAgent.h : PROJECT_NAME 응용 프로그램에 대한 주 헤더 파일입니다.
//

#pragma once

#ifndef __AFXWIN_H__
    #error "PCH에 대해 이 파일을 포함하기 전에 'stdafx.h'를 포함합니다."
#endif

#include "resource.h"        // 주 기호입니다.

#include <string>
#include "sibyl/util/OstreamRedirector.h"

// CKiwoomAgentApp:
// 이 클래스의 구현에 대해서는 KiwoomAgent.cpp을 참조하십시오.
//

class CKiwoomAgentApp : public CWinApp
{
public:
    CKiwoomAgentApp();
    std::string name;
    std::string path;

// 재정의입니다.
public:
    virtual BOOL InitInstance();

// 구현입니다.
private:
    sibyl::OstreamRedirector rdCout, rdCerr;

public:
    DECLARE_MESSAGE_MAP()
};

extern CKiwoomAgentApp theApp;
