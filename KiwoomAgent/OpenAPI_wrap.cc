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

#include "OpenAPI_wrap.h"

#include <map>
#include <iostream>
#include "sibyl/util/DispPrefix.h"
#include "sibyl/ostream_format.h"

// The one and only instance
CKHOpenAPI openAPI;

namespace sibyl
{

std::map<STR, MarketEventType> map_sRealType;
std::map<STR, STR> map_msgCode;
std::map<InputKey   , LPCTSTR> map_InputKey;
std::map<CommDataKey, LPCTSTR> map_CommDataKey;
std::map<STR, STR> map_OrdType;
std::map<STR, STR> map_ReqStat;
std::map<STR, STR> map_ReqType;
std::map<long, STR> map_FID; // for debugging ChejanData

void OpenAPI_wrap_initialize_maps()
{
    map_sRealType.clear();
    map_sRealType["주식체결"    ] = MarketEventType::trade;
    map_sRealType["주식호가잔량"] = MarketEventType::table;
    map_sRealType["ETF NAV"    ] = MarketEventType::NAV;
    map_sRealType["업종지수"    ] = MarketEventType::index;

    map_msgCode.clear();
    // Real server
    map_msgCode["100000"] = ""; // 조회가 완료되었습니다.
    map_msgCode["107066"] = ""; // 매수주문이 완료되었습니다.
    map_msgCode["107048"] = ""; // 매도주문이 완료되었습니다.
    map_msgCode["107071"] = ""; // 매수취소 주문입력이 완료되었습니다
    map_msgCode["107055"] = ""; // 매도취소 주문입력이 완료되었습니다.
    map_msgCode["107062"] = ""; // 매수정정 주문입력이 완료되었습니다
    map_msgCode["107046"] = ""; // 매도정정 주문입력이 완료되었습니다.
    
    // Test server
    map_msgCode["00Z310"] = ""; // 모의서버 조회 완료
    map_msgCode["00Z112"] = ""; // "[ACK]   b   received";
    map_msgCode["00Z113"] = ""; // "[ACK]   s   received";
    map_msgCode["00Z114"] = ""; // "[ACK] mb/ms received";
    map_msgCode["00Z115"] = ""; // "[ACK] cb/cs received";
    map_msgCode["00Z231"] = "[ERR] 0 quant left for mod / cancel";
    map_msgCode["00Z353"] = "[ERR] Order quant incorrect?";
    map_msgCode["00Z924"] = "[ERR] Excessive quant for cancel";

    map_InputKey.clear();
    map_InputKey[InputKey::code       ] = "종목코드";
    map_InputKey[InputKey::accno      ] = "계좌번호";
    map_InputKey[InputKey::pin        ] = "비밀번호";
    map_InputKey[InputKey::pin_input_s] = "비밀번호입력매체구분";
    map_InputKey[InputKey::query_s    ] = "조회구분";
    map_InputKey[InputKey::reqstat_s  ] = "체결구분";
    map_InputKey[InputKey::ordtype_s  ] = "매매구분";
    map_InputKey[InputKey::market_s   ] = "시장구분";
    map_InputKey[InputKey::sector_code] = "업종코드";

    map_CommDataKey.clear();
    map_CommDataKey[CommDataKey::code       ] = "종목코드";
    map_CommDataKey[CommDataKey::code_g     ] = "종목번호";
    map_CommDataKey[CommDataKey::refprice   ] = "기준가";
    map_CommDataKey[CommDataKey::delayedbal ] = "d+2출금가능금액";
    map_CommDataKey[CommDataKey::cnt_plus_so] = "보유수량";
    map_CommDataKey[CommDataKey::ordtype    ] = "주문구분";
    map_CommDataKey[CommDataKey::ordno      ] = "주문번호";
    map_CommDataKey[CommDataKey::ordp       ] = "주문가격";
    map_CommDataKey[CommDataKey::ordq       ] = "미체결수량";

    map_OrdType.clear();
    map_OrdType["+매수"    ] = std::to_string(static_cast<int>(OrdType::buy));
    map_OrdType["+매수정정"] = std::to_string(static_cast<int>(OrdType::buy));
    map_OrdType["-매도"    ] = std::to_string(static_cast<int>(OrdType::sell));
    map_OrdType["-매도정정"] = std::to_string(static_cast<int>(OrdType::sell));

    map_ReqStat.clear();
    map_ReqStat["접수"] = std::to_string(static_cast<int>(ReqStat::received));
    map_ReqStat["확인"] = std::to_string(static_cast<int>(ReqStat::confirmed));
    map_ReqStat["체결"] = std::to_string(static_cast<int>(ReqStat::traded));

    map_ReqType.clear();
    map_ReqType["+매수"    ] = std::to_string(static_cast<long>(ReqType::b));
    map_ReqType["-매도"    ] = std::to_string(static_cast<long>(ReqType::s));
    map_ReqType["매수취소" ] = std::to_string(static_cast<long>(ReqType::cb));
    map_ReqType["매도취소" ] = std::to_string(static_cast<long>(ReqType::cs));
    map_ReqType["+매수정정"] = std::to_string(static_cast<long>(ReqType::mb));
    map_ReqType["-매도정정"] = std::to_string(static_cast<long>(ReqType::ms));

    map_FID.clear();
    map_FID[kFID::code]    = "code";
    map_FID[kFID::cnt]     = "cnt";
    map_FID[kFID::reqstat] = "stat";
    map_FID[kFID::reqtype] = "type";
    map_FID[kFID::ordno]   = "no";
    map_FID[kFID::ordp]    = "ordp";
    map_FID[kFID::ordq]    = "ordq";
    map_FID[kFID::ordno_o] = "no_o";
    map_FID[kFID::delta_p] = "delp";
    map_FID[kFID::delta_q] = "delq";
}

MarketEventType Map_sRealType(LPCTSTR sRealType)
{
    auto it = map_sRealType.find(sRealType);
    if (it != std::end(map_sRealType))
        return it->second;
    else
        return MarketEventType::null;
}

CSTR& Map_sMsg(LPCTSTR sMsg)
{
    static STR str;
    auto it = map_msgCode.find(STR(sMsg).substr(1, 6));
    if (it != std::end(map_msgCode))
        return it->second;
    else
    {
        if (sMsg == STR(" 조회가 완료되었습니다."))
        {
            str.clear();
            return str;
        }
        else
            return str = STR("[MSG] ") + sMsg;
    }
}

void SetInputValue(InputKey key, CSTR &val)
{
    auto it = map_InputKey.find(key);
    if (it != std::end(map_InputKey))
        openAPI.SetInputValue(it->second, val.c_str());
    else
        std::cerr << dispPrefix << "SetInputValue: Unknown InputKey " << static_cast<int>(key) << std::endl;
}
long GetRepeatCnt(CSTR &TR_name, CSTR &TR_code)
{
    long cnt = openAPI.GetRepeatCnt(TR_code.c_str(), TR_name.c_str());
    // debug_msg("[ TRRepeatCnt] " << cnt);
    return cnt;
}

long CommRqData(CSTR &TR_name, CSTR &TR_code, bool carry, CSTR &scrno)
{
    return openAPI.CommRqData(TR_name.c_str(), TR_code.c_str(), (carry == true ? 2 : 0), scrno.c_str());
}

CSTR& GetCommData(CSTR &TR_name, CSTR &TR_code, long idx, CommDataKey key)
{
    // CommDataKey::code_g  : strip first character
    // CommDataKey::ordtype : convert EUC-KR string to "1"/"-1" (use enum OrdType)

    static STR str;
    auto it = map_CommDataKey.find(key);
    if (it != std::end(map_CommDataKey))
    {
        str = openAPI.GetCommData(TR_code.c_str(), TR_name.c_str(), idx, it->second).Trim();
        if      (key == CommDataKey::code_g ) str = str.substr(1);
        else if (key == CommDataKey::ordtype)
        {
            auto it = map_OrdType.find(str);
            if (it != std::end(map_OrdType))
                str = it->second;
            else
            {
                std::cerr << dispPrefix << "GetCommData: Unknown OrdType string \"" << str << "\"" << std::endl;
                str.clear();
            }
        }
    }
    else
    {
        std::cerr << dispPrefix << "GetCommData: Unknown CommDataKey " << static_cast<int>(key) << std::endl;
        str.clear();
    }

    // debug_msg("[" << std::setw(12) << TR_name << "] <" << std::setw(2) << static_cast<int>(key) << "> " << str);

    return str;
}

long SendOrder(CSTR &TR_name, CSTR &scrno, CSTR &accno, ReqType type, CSTR &code, PQ pq, CSTR &ordno_o)
{
    return openAPI.SendOrder(TR_name.c_str(), scrno.c_str(), accno.c_str(), static_cast<long>(type), code.c_str(), pq.q, pq.p, "00", ordno_o.c_str());
}

long SetRealReg(CSTR &scrno, CSTR &codes, CSTR &FIDs)
{
    return openAPI.SetRealReg(scrno.c_str(), codes.c_str(), FIDs.c_str(), "0");
}

CSTR& GetCommRealData(CSTR &code, long FID)
{
    static STR str;
    str = openAPI.GetCommRealData(code.c_str(), FID);
    return str;
}

CSTR& GetChejanData(long FID)
{
    // kFID::code           : strip first character
    // kFID::reqstat        : convert EUC-KR string to "0"/"1"/"2" (use enum ReqStat)
    // kFID::reqtype        : convert EUC-KR string to "1"/"2"/"3"/"4"/"5"/"6" (use enum ReqType)

    static STR str;
    str = openAPI.GetChejanData(FID).Trim();
    
    // debug_msg("      raw " << std::setw(4) << FID << " " << str);

    if      (FID == kFID::code   ) str = str.substr(1);
    else if (FID == kFID::reqstat)
    {
        auto it = map_ReqStat.find(str);
        if (it != std::end(map_ReqStat))
            str = it->second;
        else
        {
            std::cerr << dispPrefix << "GetChejanData: Unknown ReqStat string \"" << str << "\"" << std::endl;
            str.clear();
        }
    }
    else if (FID == kFID::reqtype)
    {
        auto it = map_ReqType.find(str);
        if (it != std::end(map_ReqType))
            str = it->second;
        else
        {
            std::cerr << dispPrefix << "GetChejanData: Unknown ReqType string \"" << str << "\"" << std::endl;
            str.clear();
        }
    }

    // debug_msg("[ChejanData " << std::setw(4) << map_FID[FID] << "] " << str);

    return str;
}

}
