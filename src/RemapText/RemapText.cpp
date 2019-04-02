#include <sdw.h>

int UMain(int argc, UChar* argv[])
{
	if (argc < 4)
	{
		UPrintf(USTR("ERROR: argument count error\n\n"));
		return 1;
	}
	n32 nRemapType = SToN32(argv[1]);
	UString sInDirName;
	UString sOutDirName;
	if (nRemapType == 0)
	{
		sInDirName = argv[3];
		sOutDirName = argv[2];
	}
	else if (nRemapType == 1)
	{
		sInDirName = argv[2];
		sOutDirName = argv[3];
	}
	else
	{
		UPrintf(USTR("ERROR: remap type error\n\n"));
		return 1;
	}
	vector<URegex> vCopy;
	vector<URegex> vRemap;
	UString sRegexPath;
	if (argc > 4)
	{
		if (argc != 5)
		{
			UPrintf(USTR("ERROR: argument count error\n\n"));
			return 1;
		}
		sRegexPath = argv[4];
		FILE* fp = UFopen(sRegexPath.c_str(), USTR("rb"));
		if (fp == nullptr)
		{
			return 1;
		}
		else
		{
			fclose(fp);
		}
	}
	else
	{
		sRegexPath = UGetModuleDirName() + USTR("/regex_remaptext.txt");
		FILE* fp = UFopen(sRegexPath.c_str(), USTR("rb"));
		if (fp == nullptr)
		{
			try
			{
				URegex rAll(USTR(".*\\.txt"), regex_constants::ECMAScript | regex_constants::icase);
				vRemap.push_back(rAll);
			}
			catch (regex_error& e)
			{
				UPrintf(USTR("ERROR: %") PRIUS USTR("\n\n"), AToU(e.what()).c_str());
			}
			sRegexPath.clear();
		}
		else
		{
			fclose(fp);
		}
	}
	if (!sRegexPath.empty())
	{
		FILE* fp = UFopen(sRegexPath.c_str(), USTR("rb"));
		if (fp == nullptr)
		{
			return 1;
		}
		fseek(fp, 0, SEEK_END);
		u32 uTxtSize = ftell(fp);
		if (uTxtSize % 2 != 0)
		{
			UPrintf(USTR("%") PRIUS USTR("\n"), sRegexPath.c_str());
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
			UPrintf(USTR("%") PRIUS USTR("\n"), sRegexPath.c_str());
			UPrintf(USTR("ERROR: no Unicode BOM\n\n"));
			delete[] pTemp;
			return 1;
		}
		pTemp[uTxtSize] = 0;
		wstring sTxt = U16ToW(pTemp + 1);
		delete[] pTemp;
		vector<wstring> vTxt = SplitOf(sTxt, L"\r\n");
		vector<URegex>* pList = &vRemap;
		for (vector<wstring>::const_iterator it = vTxt.begin(); it != vTxt.end(); ++it)
		{
			sTxt = Trim(*it);
			if (!sTxt.empty())
			{
				if (StartWith(sTxt, L"//"))
				{
					vector<wstring> vTag = Split<wstring>(sTxt.c_str() + wcslen(L"//"), L":");
					if (vTag.size() == 1 && EndWith(sTxt, L":"))
					{
						vTag[0] = Trim(vTag[0]);
						if (vTag[0] == L"copy")
						{
							pList = &vCopy;
						}
						else if (vTag[0] == L"remap")
						{
							pList = &vRemap;
						}
					}
				}
				else
				{
					try
					{
						URegex rFile(WToU(sTxt), regex_constants::ECMAScript | regex_constants::icase);
						pList->push_back(rFile);
					}
					catch (regex_error& e)
					{
						UPrintf(USTR("ERROR: %") PRIUS USTR("\n\n"), AToU(e.what()).c_str());
					}
				}
			}
		}
	}
	if (vCopy.empty() && vRemap.empty())
	{
		UPrintf(USTR("ERROR: no rule\n\n"));
		return 1;
	}
	map<UString, UString> mFileMapping;
	vector<UString> vDir;
	queue<vector<UString>> qPath;
	qPath.push(vDir);
	queue<UString> qDir;
	qDir.push(sInDirName);
	while (!qDir.empty())
	{
		UString& sParent = qDir.front();
		vDir = qPath.front();
#if SDW_PLATFORM == SDW_PLATFORM_WINDOWS
		WIN32_FIND_DATAW ffd;
		HANDLE hFind = INVALID_HANDLE_VALUE;
		wstring sPattern = sParent + L"/*";
		hFind = FindFirstFileW(sPattern.c_str(), &ffd);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			do
			{
				if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
				{
					wstring sFileName = sParent + L"/" + ffd.cFileName;
					UString sOutPath = sOutDirName;
					for (vector<UString>::iterator it = vDir.begin(); it != vDir.end(); ++it)
					{
						sOutPath += L"/" + *it;
					}
					sOutPath += L"/";
					sOutPath += ffd.cFileName;
					mFileMapping.insert(make_pair(sFileName, sOutPath));
				}
				else if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 && wcscmp(ffd.cFileName, L".") != 0 && wcscmp(ffd.cFileName, L"..") != 0)
				{
					vDir.push_back(ffd.cFileName);
					qPath.push(vDir);
					vDir.pop_back();
					wstring sDir = sParent + L"/" + ffd.cFileName;
					qDir.push(sDir);
				}
			} while (FindNextFileW(hFind, &ffd) != 0);
		}
#else
		DIR* pDir = opendir(sParent.c_str());
		if (pDir != nullptr)
		{
			dirent* pDirent = nullptr;
			while ((pDirent = readdir(pDir)) != nullptr)
			{
				if (pDirent->d_type == DT_REG)
				{
					string sFileName = sParent + "/" + pDirent->d_name;
					UString sOutPath = sOutDirName;
					for (vector<UString>::iterator it = vDir.begin(); it != vDir.end(); ++it)
					{
						sOutPath += "/" + *it;
					}
					sOutPath += "/";
					sOutPath += pDirent->d_name;
					mFileMapping.insert(make_pair(sFileName, sOutPath));
				}
				else if (pDirent->d_type == DT_DIR && strcmp(pDirent->d_name, ".") != 0 && strcmp(pDirent->d_name, "..") != 0)
				{
					vDir.push_back(pDirent->d_name);
					qPath.push(vDir);
					vDir.pop_back();
					string sDir = sParent + "/" + pDirent->d_name;
					qDir.push(sDir);
				}
			}
			closedir(pDir);
		}
#endif
		qPath.pop();
		qDir.pop();
	}
	set<UString> sFileSet;
	map<n32, set<UString>> mFileMap;
	for (map<UString, UString>::iterator it = mFileMapping.begin(); it != mFileMapping.end(); ++it)
	{
		UString sFileName = it->first;
		for (n32 i = 0; i < static_cast<n32>(vCopy.size()); i++)
		{
			if (regex_search(sFileName, vCopy[i]))
			{
				sFileSet.insert(sFileName);
				mFileMap[-1].insert(sFileName);
				break;
			}
		}
		for (n32 i = 0; i < static_cast<n32>(vRemap.size()); i++)
		{
			if (regex_search(sFileName, vRemap[i]))
			{
				if (sFileSet.insert(sFileName).second)
				{
					mFileMap[i].insert(sFileName);
				}
				break;
			}
		}
	}
	// £¤£¤£¤£¤£¤£¤£¤£¤ÖØ¸´ÎÄ±¾£¤£¤£¤£¤£¤£¤£¤
	wstring sReplacement = L"\uFFE5\uFFE5\uFFE5\uFFE5\uFFE5\uFFE5\uFFE5\uFFE5\u91CD\u590D\u6587\u672C\uFFE5\uFFE5\uFFE5\uFFE5\uFFE5\uFFE5\uFFE5";
	unordered_map<wstring, wstring> mText;
	for (map<n32, set<UString>>::iterator itMap = mFileMap.begin(); itMap != mFileMap.end(); ++itMap)
	{
		bool bCopy = itMap->first == -1;
		if (bCopy)
		{
			continue;
		}
		set<UString>& sFile = itMap->second;
		for (set<UString>::iterator it = sFile.begin(); it != sFile.end(); ++it)
		{
			const UString& sInFileName = *it;
			FILE* fp = UFopen(sInFileName.c_str(), USTR("rb"));
			if (fp == nullptr)
			{
				return 1;
			}
			fseek(fp, 0, SEEK_END);
			u32 uTxtSize = ftell(fp);
			if (uTxtSize % 2 != 0)
			{
				UPrintf(USTR("%") PRIUS USTR("\n"), sInFileName.c_str());
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
				UPrintf(USTR("%") PRIUS USTR("\n"), sInFileName.c_str());
				UPrintf(USTR("ERROR: no Unicode BOM\n\n"));
				delete[] pTemp;
				return 1;
			}
			pTemp[uTxtSize] = 0;
			wstring sTxt = U16ToW(pTemp + 1);
			delete[] pTemp;
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
					UPrintf(USTR("%") PRIUS USTR("\n"), sInFileName.c_str());
					UPrintf(USTR("ERROR: %") PRIUS USTR(" can not find first \\r\\n--------------------------------------\\r\\n\n\n"), WToU(sNum).c_str());
					return 1;
				}
				wstring sNum = sTxt.substr(uPos0, uPos1 - uPos0);
				uPos0 = uPos1 + wcslen(L"\r\n--------------------------------------\r\n");
				uPos1 = sTxt.find(L"\r\n======================================\r\n", uPos0);
				if (uPos1 == wstring::npos)
				{
					UPrintf(USTR("%") PRIUS USTR("\n"), sInFileName.c_str());
					UPrintf(USTR("ERROR: %") PRIUS USTR(" can not find \\r\\n======================================\\r\\n\n\n"), WToU(sNum).c_str());
					return 1;
				}
				wstring sStmtOld = sTxt.substr(uPos0, uPos1 - uPos0);
				uPos0 = uPos1 + wcslen(L"\r\n======================================\r\n");
				uPos1 = sTxt.find(L"\r\n--------------------------------------", uPos0);
				if (uPos1 == wstring::npos)
				{
					UPrintf(USTR("%") PRIUS USTR("\n"), sInFileName.c_str());
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
					UPrintf(USTR("%") PRIUS USTR("\n"), sInFileName.c_str());
					UPrintf(USTR("ERROR: %") PRIUS USTR(" find --------------------------------------\n\n"), WToU(sNum).c_str());
					return 1;
				}
				if (sTempTxt.find(L"======================================") != wstring::npos)
				{
					UPrintf(USTR("%") PRIUS USTR("\n"), sInFileName.c_str());
					UPrintf(USTR("ERROR: %") PRIUS USTR(" find ======================================\n\n"), WToU(sNum).c_str());
					return 1;
				}
				if (sTempTxt.find(L"No.") != wstring::npos)
				{
					UPrintf(USTR("%") PRIUS USTR("\n"), sInFileName.c_str());
					UPrintf(USTR("ERROR: %") PRIUS USTR(" find No.\n\n"), WToU(sNum).c_str());
					return 1;
				}
				sTempTxt = sStmtNew;
				sTempTxt = Replace(sTempTxt, L"[--------------------------------------]", L"");
				sTempTxt = Replace(sTempTxt, L"[======================================]", L"");
				if (sTempTxt.find(L"--------------------------------------") != wstring::npos)
				{
					UPrintf(USTR("%") PRIUS USTR("\n"), sInFileName.c_str());
					UPrintf(USTR("ERROR: %") PRIUS USTR(" find --------------------------------------\n\n"), WToU(sNum).c_str());
					return 1;
				}
				if (sTempTxt.find(L"======================================") != wstring::npos)
				{
					UPrintf(USTR("%") PRIUS USTR("\n"), sInFileName.c_str());
					UPrintf(USTR("ERROR: %") PRIUS USTR(" find ======================================\n\n"), WToU(sNum).c_str());
					return 1;
				}
				if (sTempTxt.find(L"No.") != wstring::npos)
				{
					UPrintf(USTR("%") PRIUS USTR("\n"), sInFileName.c_str());
					UPrintf(USTR("ERROR: %") PRIUS USTR(" find No.\n\n"), WToU(sNum).c_str());
					return 1;
				}
				if (sStmtNew != sReplacement && !EndWith(sNum, L"\u3002"))
				{
					mText.insert(make_pair(sStmtOld, sStmtNew));
				}
			}
		}
	}
	for (map<n32, set<UString>>::iterator itMap = mFileMap.begin(); itMap != mFileMap.end(); ++itMap)
	{
		bool bCopy = itMap->first == -1;
		set<UString>& sFile = itMap->second;
		for (set<UString>::iterator it = sFile.begin(); it != sFile.end(); ++it)
		{
			const UString& sInFileName = *it;
			FILE* fp = UFopen(sInFileName.c_str(), USTR("rb"));
			if (fp == nullptr)
			{
				return 1;
			}
			fseek(fp, 0, SEEK_END);
			u32 uTxtSize = ftell(fp);
			if (uTxtSize % 2 != 0)
			{
				UPrintf(USTR("%") PRIUS USTR("\n"), sInFileName.c_str());
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
				UPrintf(USTR("%") PRIUS USTR("\n"), sInFileName.c_str());
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
					UPrintf(USTR("%") PRIUS USTR("\n"), sInFileName.c_str());
					UPrintf(USTR("ERROR: %") PRIUS USTR(" can not find first \\r\\n--------------------------------------\\r\\n\n\n"), WToU(sNum).c_str());
					return 1;
				}
				wstring sNum = sTxt.substr(uPos0, uPos1 - uPos0);
				uPos0 = uPos1 + wcslen(L"\r\n--------------------------------------\r\n");
				uPos1 = sTxt.find(L"\r\n======================================\r\n", uPos0);
				if (uPos1 == wstring::npos)
				{
					UPrintf(USTR("%") PRIUS USTR("\n"), sInFileName.c_str());
					UPrintf(USTR("ERROR: %") PRIUS USTR(" can not find \\r\\n======================================\\r\\n\n\n"), WToU(sNum).c_str());
					return 1;
				}
				wstring sStmtOld = sTxt.substr(uPos0, uPos1 - uPos0);
				uPos0 = uPos1 + wcslen(L"\r\n======================================\r\n");
				uPos1 = sTxt.find(L"\r\n--------------------------------------", uPos0);
				if (uPos1 == wstring::npos)
				{
					UPrintf(USTR("%") PRIUS USTR("\n"), sInFileName.c_str());
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
					UPrintf(USTR("%") PRIUS USTR("\n"), sInFileName.c_str());
					UPrintf(USTR("ERROR: %") PRIUS USTR(" find --------------------------------------\n\n"), WToU(sNum).c_str());
					return 1;
				}
				if (sTempTxt.find(L"======================================") != wstring::npos)
				{
					UPrintf(USTR("%") PRIUS USTR("\n"), sInFileName.c_str());
					UPrintf(USTR("ERROR: %") PRIUS USTR(" find ======================================\n\n"), WToU(sNum).c_str());
					return 1;
				}
				if (sTempTxt.find(L"No.") != wstring::npos)
				{
					UPrintf(USTR("%") PRIUS USTR("\n"), sInFileName.c_str());
					UPrintf(USTR("ERROR: %") PRIUS USTR(" find No.\n\n"), WToU(sNum).c_str());
					return 1;
				}
				sTempTxt = sStmtNew;
				sTempTxt = Replace(sTempTxt, L"[--------------------------------------]", L"");
				sTempTxt = Replace(sTempTxt, L"[======================================]", L"");
				if (sTempTxt.find(L"--------------------------------------") != wstring::npos)
				{
					UPrintf(USTR("%") PRIUS USTR("\n"), sInFileName.c_str());
					UPrintf(USTR("ERROR: %") PRIUS USTR(" find --------------------------------------\n\n"), WToU(sNum).c_str());
					return 1;
				}
				if (sTempTxt.find(L"======================================") != wstring::npos)
				{
					UPrintf(USTR("%") PRIUS USTR("\n"), sInFileName.c_str());
					UPrintf(USTR("ERROR: %") PRIUS USTR(" find ======================================\n\n"), WToU(sNum).c_str());
					return 1;
				}
				if (sTempTxt.find(L"No.") != wstring::npos)
				{
					UPrintf(USTR("%") PRIUS USTR("\n"), sInFileName.c_str());
					UPrintf(USTR("ERROR: %") PRIUS USTR(" find No.\n\n"), WToU(sNum).c_str());
					return 1;
				}
				bool bMark = EndWith(sNum, L"\u3002");
				if (bMark && nRemapType == 0 && sStmtNew == sReplacement)
				{
					unordered_map<wstring, wstring>::iterator itText = mText.find(sStmtOld);
					if (itText != mText.end())
					{
						sStmtNew = itText->second;
						sNum.erase(sNum.size() - 1);
					}
				}
				if (!bCopy && !bMark)
				{
					unordered_map<wstring, wstring>::iterator itText = mText.find(sStmtOld);
					if (nRemapType == 0)
					{
						if (itText == mText.end())
						{
							UPrintf(USTR("ERROR: %") PRIUS USTR(" %") PRIUS USTR("\n\n"), sInFileName.c_str(), WToU(sNum).c_str());
							return 1;
						}
						else
						{
							sStmtNew = itText->second;
						}
					}
					else if (nRemapType == 1)
					{
						if (itText != mText.end())
						{
							sStmtNew = itText->second;
							mText.erase(itText);
						}
						else
						{
							sStmtNew = sReplacement;
						}
					}
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
			UString sOutFileName = mFileMapping[sInFileName];
			vector<UString> vPath = SplitOf(sOutFileName, USTR("/\\"));
			if (vPath.size() > 1)
			{
				UString sOutDirPath = vPath[0];
				if (!UMakeDir(sOutDirPath.c_str()))
				{
					UPrintf(USTR("ERROR: create dir %") PRIUS USTR(" error\n\n"), sOutDirPath.c_str());
					return 1;
				}
				for (n32 i = 1; i < static_cast<n32>(vPath.size()) - 1; i++)
				{
					sOutDirPath += USTR("/") + vPath[i];
					if (!UMakeDir(sOutDirPath.c_str()))
					{
						UPrintf(USTR("ERROR: create dir %") PRIUS USTR(" error\n\n"), sOutDirPath.c_str());
						return 1;
					}
				}
			}
			fp = UFopen(sOutFileName.c_str(), USTR("wb"));
			if (fp == nullptr)
			{
				return 1;
			}
			fwrite("\xFF\xFE", 2, 1, fp);
			fwrite(sTxtNew.c_str(), 2, sTxtNew.size(), fp);
			fclose(fp);
		}
	}
	return 0;
}
