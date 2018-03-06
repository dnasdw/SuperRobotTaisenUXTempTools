#include <sdw.h>

int UMain(int argc, UChar* argv[])
{
	if (argc != 2)
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
		UPrintf(USTR("ERROR: no Unicode BOM\n\n"));
		delete[] pTemp;
		return 1;
	}
	pTemp[uTxtSize] = 0;
	wstring sTxt = U16ToW(pTemp + 1);
	delete[] pTemp;
	wstring sTxtNew;
	wstring::size_type uPos0 = 0;
	while ((uPos0 = sTxt.find(L"No.", uPos0)) != wstring::npos)
	{
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
			UPrintf(USTR("ERROR: %") PRIUS USTR(" can not find first \\r\\n--------------------------------------\\r\\n\n\n"), WToU(sNum).c_str());
			return 1;
		}
		wstring sNum = sTxt.substr(uPos0, uPos1 - uPos0);
		uPos0 = uPos1 + wcslen(L"\r\n--------------------------------------\r\n");
		uPos1 = sTxt.find(L"\r\n======================================\r\n", uPos0);
		if (uPos1 == wstring::npos)
		{
			UPrintf(USTR("ERROR: %") PRIUS USTR(" can not find \\r\\n======================================\\r\\n\n\n"), WToU(sNum).c_str());
			return 1;
		}
		wstring sStmtOld = sTxt.substr(uPos0, uPos1 - uPos0);
		uPos0 = uPos1 + wcslen(L"\r\n======================================\r\n");
		uPos1 = sTxt.find(L"\r\n--------------------------------------", uPos0);
		if (uPos1 == wstring::npos)
		{
			UPrintf(USTR("ERROR: %") PRIUS USTR(" can not find last \\r\\n--------------------------------------\n\n"), WToU(sNum).c_str());
			return 1;
		}
		wstring sStmtNew = sTxt.substr(uPos0, uPos1 - uPos0);
		uPos0 = uPos1 + wcslen(L"\r\n--------------------------------------");
		wstring sTempTxt = sStmtOld;
		sTempTxt = Replace(sTempTxt, L"[--------------------------------------]", L"");
		sTempTxt = Replace(sTempTxt, L"[======================================]", L"");
		if (sTempTxt.find(L"--------------------------------------") != wstring::npos)
		{
			UPrintf(USTR("ERROR: %") PRIUS USTR(" find --------------------------------------\n\n"), WToU(sNum).c_str());
			return 1;
		}
		if (sTempTxt.find(L"======================================") != wstring::npos)
		{
			UPrintf(USTR("ERROR: %") PRIUS USTR(" find ======================================\n\n"), WToU(sNum).c_str());
			return 1;
		}
		if (sTempTxt.find(L"No.") != wstring::npos)
		{
			UPrintf(USTR("ERROR: %") PRIUS USTR(" find No.\n\n"), WToU(sNum).c_str());
			return 1;
		}
		sTempTxt = sStmtNew;
		sTempTxt = Replace(sTempTxt, L"[--------------------------------------]", L"");
		sTempTxt = Replace(sTempTxt, L"[======================================]", L"");
		if (sTempTxt.find(L"--------------------------------------") != wstring::npos)
		{
			UPrintf(USTR("ERROR: %") PRIUS USTR(" find --------------------------------------\n\n"), WToU(sNum).c_str());
			return 1;
		}
		if (sTempTxt.find(L"======================================") != wstring::npos)
		{
			UPrintf(USTR("ERROR: %") PRIUS USTR(" find ======================================\n\n"), WToU(sNum).c_str());
			return 1;
		}
		if (sTempTxt.find(L"No.") != wstring::npos)
		{
			UPrintf(USTR("ERROR: %") PRIUS USTR(" find No.\n\n"), WToU(sNum).c_str());
			return 1;
		}
		if (!sTxtNew.empty())
		{
			sTxtNew += L"\r\n\r\n";
		}
		sTxtNew += sNum;
		sTxtNew += L"\r\n--------------------------------------\r\n";
		sTxtNew += sStmtOld;
		sTxtNew += L"\r\n======================================\r\n";
		sTxtNew += sStmtNew;
		sTxtNew += L"\r\n--------------------------------------\r\n";
	}
	fp = UFopen(argv[1], USTR("wb"));
	if (fp == nullptr)
	{
		return 1;
	}
	fwrite("\xFF\xFE", 2, 1, fp);
	fwrite(sTxtNew.c_str(), 2, sTxtNew.size(), fp);
	fclose(fp);
	return 0;
}
