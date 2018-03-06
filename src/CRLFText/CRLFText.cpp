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
	sTxt = Replace(sTxt, L"\r\n", L"\n");
	sTxt = Replace(sTxt, L'\r', L'\n');
	sTxt = Replace(sTxt, L'\n', L"\r\n");
	fp = UFopen(argv[1], USTR("wb"));
	if (fp == nullptr)
	{
		return 1;
	}
	fwrite("\xFF\xFE", 2, 1, fp);
	fwrite(sTxt.c_str(), 2, sTxt.size(), fp);
	fclose(fp);
	return 0;
}
