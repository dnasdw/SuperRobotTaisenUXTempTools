#include <sdw.h>

int UMain(int argc, UChar* argv[])
{
	if (argc != 3)
	{
		UPrintf(USTR("ERROR: argument count error\n\n"));
		return 1;
	}
	FILE* fp = UFopen(argv[1], USTR("rb"));
	if (fp == nullptr)
	{
		return 1;
	}
	fseek(fp, 0, SEEK_END);
	u32 uTxtSize = ftell(fp);
	if (uTxtSize % 2 != 0)
	{
		UPrintf(USTR("%") PRIUS USTR("\n"), argv[1]);
		UPrintf(USTR("ERROR: not Unicode text\n\n"));
		fclose(fp);
		return 1;
	}
	uTxtSize /= 2;
	fseek(fp, 0, SEEK_SET);
	Char16_t* pTemp = new Char16_t[uTxtSize + 1];
	fread(pTemp, 2, uTxtSize, fp);
	fclose(fp);
	if (pTemp[0] != 0xFEFF)
	{
		UPrintf(USTR("%") PRIUS USTR("\n"), argv[1]);
		UPrintf(USTR("ERROR: no Unicode BOM\n\n"));
		delete[] pTemp;
		return 1;
	}
	pTemp[uTxtSize] = 0;
	wstring sTxtOrig = U16ToW(pTemp + 1);
	delete[] pTemp;
	fp = UFopen(argv[2], USTR("rb"));
	if (fp == nullptr)
	{
		return 1;
	}
	fseek(fp, 0, SEEK_END);
	uTxtSize = ftell(fp);
	if (uTxtSize % 2 != 0)
	{
		UPrintf(USTR("%") PRIUS USTR("\n"), argv[2]);
		UPrintf(USTR("ERROR: not Unicode text\n\n"));
		fclose(fp);
		return 1;
	}
	uTxtSize /= 2;
	fseek(fp, 0, SEEK_SET);
	pTemp = new Char16_t[uTxtSize + 1];
	fread(pTemp, 2, uTxtSize, fp);
	fclose(fp);
	if (pTemp[0] != 0xFEFF)
	{
		UPrintf(USTR("%") PRIUS USTR("\n"), argv[2]);
		UPrintf(USTR("ERROR: no Unicode BOM\n\n"));
		delete[] pTemp;
		return 1;
	}
	pTemp[uTxtSize] = 0;
	wstring sTxt = U16ToW(pTemp + 1);
	delete[] pTemp;
	wstring sTxtNew;
	wstring::size_type uPosOrig0 = 0;
	wstring::size_type uPos0 = 0;
	while ((uPosOrig0 = sTxtOrig.find(L"No.", uPosOrig0)) != wstring::npos)
	{
		wstring::size_type uPosOrig1 = sTxtOrig.find(L"\r\n--------------------------------------\r\n", uPosOrig0);
		if (uPosOrig1 == wstring::npos)
		{
			wstring sNumOrig;
			uPosOrig1 = sTxtOrig.find(L"\r\n", uPosOrig0);
			if (uPosOrig1 != wstring::npos)
			{
				sNumOrig = sTxtOrig.substr(uPosOrig0, uPosOrig1 - uPosOrig0);
			}
			else
			{
				sNumOrig = sTxtOrig.substr(uPosOrig0);
			}
			UPrintf(USTR("ERROR: %") PRIUS USTR(" can not find first \\r\\n--------------------------------------\\r\\n\n\n"), WToU(sNumOrig).c_str());
			return 1;
		}
		wstring sNumOrig = sTxtOrig.substr(uPosOrig0, uPosOrig1 - uPosOrig0);
		uPosOrig0 = uPosOrig1 + wcslen(L"\r\n--------------------------------------\r\n");
		uPosOrig1 = sTxtOrig.find(L"\r\n======================================\r\n", uPosOrig0);
		if (uPosOrig1 == wstring::npos)
		{
			UPrintf(USTR("ERROR: %") PRIUS USTR(" can not find \\r\\n======================================\\r\\n\n\n"), WToU(sNumOrig).c_str());
			return 1;
		}
		wstring sStmtOrigOld = sTxtOrig.substr(uPosOrig0, uPosOrig1 - uPosOrig0);
		uPosOrig0 = uPosOrig1 + wcslen(L"\r\n======================================\r\n");
		uPosOrig1 = sTxtOrig.find(L"\r\n--------------------------------------", uPosOrig0);
		if (uPosOrig1 == wstring::npos)
		{
			UPrintf(USTR("ERROR: %") PRIUS USTR(" can not find last \\r\\n--------------------------------------\n\n"), WToU(sNumOrig).c_str());
			return 1;
		}
		uPosOrig0 = uPosOrig1 + wcslen(L"\r\n--------------------------------------");
		wstring sTempTxt = sStmtOrigOld;
		sTempTxt = Replace(sTempTxt, L"[--------------------------------------]", L"");
		sTempTxt = Replace(sTempTxt, L"[======================================]", L"");
		if (sTempTxt.find(L"--------------------------------------") != wstring::npos)
		{
			UPrintf(USTR("ERROR: %") PRIUS USTR(" find --------------------------------------\n\n"), WToU(sNumOrig).c_str());
			return 1;
		}
		if (sTempTxt.find(L"======================================") != wstring::npos)
		{
			UPrintf(USTR("ERROR: %") PRIUS USTR(" find ======================================\n\n"), WToU(sNumOrig).c_str());
			return 1;
		}
		if (sTempTxt.find(L"No.") != wstring::npos)
		{
			UPrintf(USTR("ERROR: %") PRIUS USTR(" find No.\n\n"), WToU(sNumOrig).c_str());
			return 1;
		}
		uPos0 = sTxt.find(L"No.", uPos0);
		if (uPos0 == wstring::npos)
		{
			UPrintf(USTR("ERROR: %") PRIUS USTR(" can not find No.\n\n"), WToU(sNumOrig).c_str());
			return 1;
		}
		wstring::size_type uPos1 = sTxt.find(L"\r\n--------------------------------------\r\n", uPos0);
		if (uPos1 == wstring::npos)
		{
			wstring sNum;
			uPos1 = sTxt.find(L"\r\n", uPos0);
			if (uPos1 != wstring::npos)
			{
				sNum = sTxt.substr(uPos0, uPos1 - uPos0);
			}
			else
			{
				sNum = sTxt.substr(uPos0);
			}
			UPrintf(USTR("%") PRIUS USTR("\n"), WToU(sNumOrig).c_str());
			UPrintf(USTR("ERROR: %") PRIUS USTR(" can not find first \\r\\n--------------------------------------\\r\\n\n\n"), WToU(sNum).c_str());
			return 1;
		}
		wstring sNum = sTxt.substr(uPos0, uPos1 - uPos0);
		uPos0 = uPos1 + wcslen(L"\r\n--------------------------------------\r\n");
		uPos1 = sTxt.find(L"\r\n======================================\r\n", uPos0);
		if (uPos1 == wstring::npos)
		{
			UPrintf(USTR("%") PRIUS USTR("\n"), WToU(sNumOrig).c_str());
			UPrintf(USTR("ERROR: %") PRIUS USTR(" can not find \\r\\n======================================\\r\\n\n\n"), WToU(sNum).c_str());
			return 1;
		}
		uPos0 = uPos1 + wcslen(L"\r\n======================================\r\n");
		uPos1 = sTxt.find(L"\r\n--------------------------------------", uPos0);
		if (uPos1 == wstring::npos)
		{
			UPrintf(USTR("%") PRIUS USTR("\n"), WToU(sNumOrig).c_str());
			UPrintf(USTR("ERROR: %") PRIUS USTR(" can not find last \\r\\n--------------------------------------\\r\\n\n\n"), WToU(sNum).c_str());
			return 1;
		}
		wstring sStmtNew = sTxt.substr(uPos0, uPos1 - uPos0);
		uPos0 = uPos1 + wcslen(L"\r\n--------------------------------------");
		sTempTxt = sStmtNew;
		sTempTxt = Replace(sTempTxt, L"[--------------------------------------]", L"");
		sTempTxt = Replace(sTempTxt, L"[======================================]", L"");
		if (sTempTxt.find(L"--------------------------------------") != wstring::npos)
		{
			UPrintf(USTR("%") PRIUS USTR("\n"), WToU(sNumOrig).c_str());
			UPrintf(USTR("ERROR: %") PRIUS USTR(" find --------------------------------------\n\n"), WToU(sNum).c_str());
			return 1;
		}
		if (sTempTxt.find(L"======================================") != wstring::npos)
		{
			UPrintf(USTR("%") PRIUS USTR("\n"), WToU(sNumOrig).c_str());
			UPrintf(USTR("ERROR: %") PRIUS USTR(" find ======================================\n\n"), WToU(sNum).c_str());
			return 1;
		}
		if (sTempTxt.find(L"No.") != wstring::npos)
		{
			UPrintf(USTR("%") PRIUS USTR("\n"), WToU(sNumOrig).c_str());
			UPrintf(USTR("ERROR: %") PRIUS USTR(" find No.\n\n"), WToU(sNum).c_str());
			return 1;
		}
		if (!EndWith(sNumOrig, L"\u3002") && EndWith(sNum, L"\u3002"))
		{
			sNum = sNumOrig + L"\u3002";
		}
		if (!sTxtNew.empty())
		{
			sTxtNew += L"\r\n\r\n";
		}
		sTxtNew += sNum;
		sTxtNew += L"\r\n--------------------------------------\r\n";
		sTxtNew += sStmtOrigOld;
		sTxtNew += L"\r\n======================================\r\n";
		sTxtNew += sStmtNew;
		sTxtNew += L"\r\n--------------------------------------\r\n";
	}
	fp = UFopen(argv[2], USTR("wb"));
	if (fp == nullptr)
	{
		return 1;
	}
	fwrite("\xFF\xFE", 2, 1, fp);
	fwrite(sTxtNew.c_str(), 2, sTxtNew.size(), fp);
	fclose(fp);
	return 0;
}
