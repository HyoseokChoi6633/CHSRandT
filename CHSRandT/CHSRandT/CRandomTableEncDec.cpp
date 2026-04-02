#include "CRandomTableEncDec.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "resource.h"

// 객체 직렬화를 위한 boost 라이브러리
#include <boost/serialization/serialization.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <fstream>
using namespace boost;


namespace CHSRandT_Library
{
	CRandomTableEncDec::CRandomTableEncDec() :
		m_nNumFormat(16),
		m_nTransNum(0),
		m_enEncMode(NormalEnc),
		m_iEncRepeatCnt(1),
		m_hBitMSG(nullptr),
		m_hBitMSG_Enc(nullptr),
		m_enDataEnc(None),
		m_lBitWidth(0),
		m_lBitHeight(0),
		m_wBitCnt(0),
		m_bRT_Cfg_Enable(false)
	{
		srand(GetTickCount64());

		SetTransNum();

		SetEncMode();
	}

	CRandomTableEncDec::~CRandomTableEncDec()
	{
		ReleaseImgHandle();
	}

	void CRandomTableEncDec::SetEncMode(RandTableEncMode enEncMode)
	{
		GenPassTable();
		SetTransNum();

		m_enEncMode = enEncMode;
	}

	CRandomTableEncDec::RandTableEncMode CRandomTableEncDec::GetEncMode()
	{
		return m_enEncMode;
	}

	/*
	16진 만으로도 쓸만하고 난수표도 데이터라... 그다지 큰것이 필요 없다고 봐서 주석처리 했습니다.
	void CRandomTableEncDec::SetNumFormat(int nNumFormat)
	{
		switch (nNumFormat) {
		case 8:
		case 16:
		case 32:
			m_nNumFormat = nNumFormat;
			break;
		default:
			m_nNumFormat = 16;
			break;
		}
	}
	*/

	void CRandomTableEncDec::SetTransNum(int nTransNum)
	{
		if (m_bRT_Cfg_Enable) {
			return;
		}

		if (nTransNum > 0 && nTransNum < m_nNumFormat - 1) {
			m_nTransNum = nTransNum;
		}
		else {
			m_nTransNum = rand() % (m_nNumFormat - 2) + 1;
		}
	}

	int CRandomTableEncDec::GetNumFormat()
	{
		return m_nNumFormat;
	}

	int CRandomTableEncDec::GetTransNum()
	{
		return m_nTransNum;
	}

	void CRandomTableEncDec::GenPassTable(int iPassTarget)
	{
		int i, j;
		list<int>::iterator t;

		// 암호화 테이블이 변하면 다음 값은 쓰레기 값이 되어 초기화 시킨다.
		if (iPassTarget == 1) {
			m_iListTransEnc.clear();
		}
		m_iListTransEnc2.clear();
		m_iListRestoreDec.clear();

		if (m_bRT_Cfg_Enable) {
			return;
		}

		list<int> iListTmp;
		list<int> iListShuffled;

		for (i = 0; i < m_nNumFormat; i++) {
			iListTmp.push_back(i);
		}

		while (iListTmp.size() > 0) {
			t = iListTmp.begin();
			j = rand() % iListTmp.size();
			for (i = 0; i < j; i++) {
				t++;
			}

			iListShuffled.push_back(*t);
			iListTmp.erase(t);
		}

		switch (iPassTarget) {
		case 1:
			m_iListPassTable.clear();
			m_iListPassTable = iListShuffled;
			m_iListPassTable2.clear();
			m_iListPassTable3.clear();
			break;
		case 2:
			m_iListPassTable2.clear();
			m_iListPassTable2 = iListShuffled;
			break;
		case 3:
			m_iListPassTable3.clear();
			m_iListPassTable3 = iListShuffled;
			break;
		default:
			break;
		}
	}

	const list<int> CRandomTableEncDec::GetPassTable(int iPassTarget)
	{
		switch (iPassTarget) {
		default:
		case 1:
			return m_iListPassTable;
		case 2:
			return m_iListPassTable2;
		case 3:
			return m_iListPassTable3;
		}
	}

	// 메시지 암호화 변환
	// 1차 : 기본, 2차 : 홀짝, 3차 : 반반, 4차 : 홀앞짝뒤, 5차 : 짝앞홀뒤
	void CRandomTableEncDec::TransWideMsgEnc(wstring strMsg)
	{
		bool bImgExist = false;
		int i, j;
		BYTE b;
		list<int>::iterator t;

		list<int> iListTransEnc;

		if (strMsg.length() == 0) {
			return;
		}

		if (m_hBitMSG) {
			DeleteObject(m_hBitMSG);
			m_hBitMSG = NULL;

			bImgExist = true;
		}

		if (m_hBitMSG_Enc) {
			DeleteObject(m_hBitMSG_Enc);
			m_hBitMSG_Enc = NULL;

			bImgExist = true;
		}

		if (bImgExist) {
			GenPassTable();
			SetTransNum();
			bImgExist = false;
		}


		m_iListRestoreDec.clear();

		m_enDataEnc = UniText;

		for (const wchar_t& item : strMsg) {
			for (j = 0; j < 4; j++) {
				b = (item >> (4 * (3 - j))) & 0xF;
				t = find(m_iListPassTable.begin(), m_iListPassTable.end(), b);

				for (i = 0; i < m_nTransNum; i++) {
					if (*t == m_iListPassTable.back()) {
						t = m_iListPassTable.begin();
					}
					else {
						t++;
					}
				}

				iListTransEnc.push_back(*t);
			}
		}

		SecTransEnc(iListTransEnc);
	}

	void CRandomTableEncDec::TransAnsiMsgEnc(string strMsg)
	{
		bool bImgExist = false;

		int i, j;
		BYTE b;
		list<int>::iterator t;

		list<int> iListTransEnc;

		if (strMsg.length() == 0) {
			return;
		}

		if (m_hBitMSG) {
			DeleteObject(m_hBitMSG);
			m_hBitMSG = NULL;

			bImgExist = true;
		}

		if (m_hBitMSG_Enc) {
			DeleteObject(m_hBitMSG_Enc);
			m_hBitMSG_Enc = NULL;

			bImgExist = true;
		}

		if (bImgExist) {
			GenPassTable();
			SetTransNum();
			bImgExist = false;
		}

		m_enDataEnc = AnsiText;

		for (const char& item : strMsg) {
			for (j = 0; j < 2; j++) {
				b = (item >> (4 * (1 - j))) & 0xF;
				t = find(m_iListPassTable.begin(), m_iListPassTable.end(), b);

				for (i = 0; i < m_nTransNum; i++) {
					if (*t == m_iListPassTable.back()) {
						t = m_iListPassTable.begin();
					}
					else {
						t++;
					}
				}

				iListTransEnc.push_back(*t);
			}
		}

		SecTransEnc(iListTransEnc);
	}

	void CRandomTableEncDec::TransImgEnc(HDC hDC)
	{
		int i, j, k;
		BYTE b;
		list<int>::iterator t;

		list<int> iListTransEnc;
		list<int>* piListTransEnc;

		int iTmp;
		BYTE bBuf = 0;

		if (m_hBitMSG == nullptr) {
			return;
		}

		// 저장할 bitmap 선언
		BITMAP bitmap;
		// hBitmap으로 bitmap을 가져온다.
		GetObject(m_hBitMSG, sizeof(bitmap), (LPSTR)&bitmap);
		m_lBitWidth = bitmap.bmWidth;
		m_lBitHeight = bitmap.bmHeight;
		m_wBitCnt = 24;

		// Bitmap Header 정보 설정
		BITMAPINFOHEADER bi;
		bi.biSize = sizeof(BITMAPINFOHEADER);
		bi.biWidth = m_lBitWidth;
		bi.biHeight = m_lBitHeight;
		bi.biPlanes = 1;
		bi.biBitCount = m_wBitCnt;
		bi.biCompression = BI_RGB;
		bi.biSizeImage = 0;
		bi.biXPelsPerMeter = 0;
		bi.biYPelsPerMeter = 0;
		bi.biClrUsed = 0;
		bi.biClrImportant = 0;

		int iPalSize = (bi.biBitCount == 24 ? 0 : 1 << bi.biBitCount) * sizeof(RGBQUAD);
		int iSize = bi.biSize + iPalSize + bi.biSizeImage;

		// 메모리 할당 (bitmap header)
		BITMAPINFO* header = (BITMAPINFO*)malloc(bi.biSize + iPalSize);
		header->bmiHeader = bi;
		// hBitmap으로 부터 해더를 가져온다.
		GetDIBits(hDC, m_hBitMSG, 0, m_lBitHeight, NULL, header, DIB_RGB_COLORS);
		
		// 이미지 전체 사이즈를 취득한다.
		bi = header->bmiHeader;

		if (bi.biSizeImage == 0) {
			// 해더 사이즈 설정이 안되면 강제 계산 설정
			bi.biSizeImage = ((m_lBitWidth * bi.biBitCount + 31) & ~31) / 8 * m_lBitHeight;
		}
		
		// 이미지 영역 메모리 할당
		iSize = bi.biSize + iPalSize + bi.biSizeImage;
		
		PBYTE body = (PBYTE)malloc(header->bmiHeader.biSizeImage);
		
		// hBitmap의 데이터를 저장
		GetDIBits(hDC, m_hBitMSG, 0, header->bmiHeader.biHeight, body, header, DIB_RGB_COLORS);

		for(k = 0; k < bi.biSizeImage; k++) {
			for (j = 0; j < 2; j++) {
				b = (body[k] >> (4 * (1 - j))) & 0xF;
				t = find(m_iListPassTable.begin(), m_iListPassTable.end(), b);

				for (i = 0; i < m_nTransNum; i++) {
					if (*t == m_iListPassTable.back()) {
						t = m_iListPassTable.begin();
					}
					else {
						t++;
					}
				}

				iListTransEnc.push_back(*t);
			}
		}

		SecTransEnc(iListTransEnc);

		if (m_enEncMode == NormalEnc) {
			piListTransEnc = &m_iListTransEnc;
		}
		else {
			piListTransEnc = &m_iListTransEnc2;
		}

		i = 0;
		j = 0;
		// 암호화된 이미지 데이터를 body에 다시 입력
		for (t = piListTransEnc->begin(); t != piListTransEnc->end(); t++) {
			iTmp = *t;

			switch (i % 2) {
			case 0:
				iTmp <<= 4;
				iTmp &= 0xF0;
				break;
			case 1:
				iTmp &= 0x0F;
				break;
			}

			bBuf += iTmp;

			if (i % 2 == 1) {
				body[j++] = bBuf;
				bBuf = 0;
			}

			i++;
		}

		if (m_hBitMSG_Enc) {
			DeleteObject(m_hBitMSG_Enc);
			m_hBitMSG_Enc = NULL;
		}

		m_hBitMSG_Enc = CreateCompatibleBitmap(hDC, bitmap.bmWidth, bitmap.bmHeight);

		// 비트맵 암호화 상태로 이미지 데이터 저장
		SetDIBits(hDC, m_hBitMSG_Enc, 0, header->bmiHeader.biHeight, body, header, DIB_RGB_COLORS);

		// 메모리 반환
		free(header);
		free(body);

		m_enDataEnc = Image_BMP;
	}

	const list<int> CRandomTableEncDec::GetTransEnc(int iPos)
	{
		if (m_hBitMSG || m_hBitMSG_Enc) {
			return list<int>();
		}

		switch (iPos) {
		default:
		case 1:
			return m_iListTransEnc;
		case 2:
			return m_iListTransEnc2;
		}
	}

	void CRandomTableEncDec::RestoreMsgDec()
	{
		int i;

		list<int>::iterator t;

		m_iListRestoreDec.clear();

		SecRestoreDec();

		for (const auto& EncVal : m_iListTransEnc) {
			t = find(m_iListPassTable.begin(), m_iListPassTable.end(), EncVal);

			for (i = 0; i < m_nTransNum; i++) {
				if (*t == m_iListPassTable.front()) {
					t = find(m_iListPassTable.begin(), m_iListPassTable.end(), m_iListPassTable.back());
				}
				else {
					t--;
				}
			}

			m_iListRestoreDec.push_back(*t);
		}
	}

	const list<int> CRandomTableEncDec::GetRestoreDec()
	{
		if (m_hBitMSG != nullptr) {
			return list<int>();
		}

		return m_iListRestoreDec;
	}

	const tstring CRandomTableEncDec::GetRestoreMsg()
	{
		tstring strMsg = _T("");

		if (m_hBitMSG != nullptr) {
			return _T("");
		}

#ifdef _UNICODE
		if (m_enDataEnc == UniText) {
			strMsg = GetRestoreWideMsg();
		}
		else {
			string ansiMsg = GetRestoreAnsiMsg();
			convert_ansi_to_unicode_string(strMsg, ansiMsg.c_str(), ansiMsg.length());
		}
#else
		if (m_enDataEnc == AnsiText) {
			strMsg = GetRestoreAnsiMsg();
		}
		else {
			wstring uniMsg = GetRestoreWideMsg();
			convert_unicode_to_ansi_string(strMsg, uniMsg.c_str(), uniMsg.length());
		}
#endif

		return strMsg;
	}

	void CRandomTableEncDec::GetRestoreImg(HDC hDC)
	{
		int i, j;
		int iTmp;
		BYTE bBuf = 0;
		list<int>::iterator t;

		if (m_enDataEnc != Image_BMP) {
			return;
		}

		RestoreMsgDec();

		if (m_hBitMSG) {
			DeleteObject(m_hBitMSG);
			m_hBitMSG = NULL;
		}

		SetImgEnc2HBitmap(hDC, m_lBitWidth, m_lBitHeight, m_wBitCnt, m_hBitMSG, &m_iListRestoreDec);

		m_enDataEnc = None;
	}

	const wstring CRandomTableEncDec::GetRestoreWideMsg()
	{
		wstring str;
		list<int>::iterator t;
		int i = 0;
		int iTmp;
		wchar_t wch = 0;


		for (t = m_iListRestoreDec.begin(); t != m_iListRestoreDec.end(); t++) {
			iTmp = *t;

			switch (i % 4) {
			case 0:
				iTmp <<= 12;
				iTmp &= 0xF000;
				break;
			case 1:
				iTmp <<= 8;
				iTmp &= 0x0F00;
				break;
			case 2:
				iTmp <<= 4;
				iTmp &= 0x00F0;
				break;
			case 3:
				iTmp &= 0x000F;
				break;
			}

			wch += iTmp;

			if (i % 4 == 3) {
				str += wch;
				wch = 0;
			}

			i++;
		}

		return str;
	}

	const string CRandomTableEncDec::GetRestoreAnsiMsg()
	{
		string str;
		list<int>::iterator t;
		int i = 0;
		int iTmp;
		char ch = 0;


		for (t = m_iListRestoreDec.begin(); t != m_iListRestoreDec.end(); t++) {
			iTmp = *t;

			switch (i % 2) {
			case 0:
				iTmp <<= 4;
				iTmp &= 0xF0;
				break;
			case 1:
				iTmp &= 0x0F;
				break;
			}

			ch += iTmp;

			if (i % 2 == 1) {
				str += ch;
				ch = 0;
			}

			i++;
		}

		return str;
	}

	int CRandomTableEncDec::convert_ansi_to_unicode_string(wstring& unicode, LPCSTR ansi, const size_t ansi_size)
	{
		DWORD error = 0;
		
		do {
			if ((nullptr == ansi) || (0 == ansi_size)) {
				error = ERROR_INVALID_PARAMETER;
				break;
			}
			
			unicode.clear();
			
			//
			// getting required cch.
			//
			
			int required_cch = ::MultiByteToWideChar(
				CP_ACP,
				0,
				ansi, static_cast<int>(ansi_size),
				nullptr, 0
			);
			
			if (0 == required_cch) {
				error = ::GetLastError();
				break;
			}
			
			unicode.resize(required_cch);
			
			//
			// convert.
			//

			if (0 == ::MultiByteToWideChar(
				CP_ACP,
				0,
				ansi, static_cast<int>(ansi_size),
				const_cast<wchar_t*>(unicode.c_str()),
				static_cast<int>(unicode.size())
			)) {
				error = ::GetLastError();
				break;
			}
		} while (false);
		
		return error;
	}

	int CRandomTableEncDec::convert_unicode_to_ansi_string(string& ansi, LPCWSTR unicode, const size_t unicode_size)
	{
		DWORD error = 0;

		do {
			if ((nullptr == unicode) || (0 == unicode_size)) {
				error = ERROR_INVALID_PARAMETER;
				break;
			}
			
			ansi.clear();
			//
			// getting required cch.
			//
			
			int required_cch = ::WideCharToMultiByte(
				CP_ACP,
				0,
				unicode, static_cast<int>(unicode_size),
				nullptr, 0,
				nullptr, nullptr
			);
			
			if (0 == required_cch) {
				error = ::GetLastError();
				break;
			}
			
			//
			// allocate.
			//
			
			ansi.resize(required_cch);
			
			//
			// convert.
			//
			
			if (0 == ::WideCharToMultiByte(
				CP_ACP,
				0,
				unicode, static_cast<int>(unicode_size),
				const_cast<char*>(ansi.c_str()), static_cast<int>(ansi.size()),
				nullptr, nullptr
			)) {
				error = ::GetLastError();
				break;
			}
		} while (false);
		
		return error;
	}

	bool CRandomTableEncDec::FileSaveEncData(const tstring strFullPath)
	{
		bool bRetVal = false;

		int find = strFullPath.rfind(_T("\\")) + 1;
		int ext = strFullPath.rfind(_T("."));

		if (ext == -1) {
			ext = strFullPath.length();
		}

		m_strPath = strFullPath.substr(0, find);
		m_strFileName = strFullPath.substr(find, ext - find);
		m_iExistsCnt = 0;

		if (SaveCRTFile()) {
			bRetVal = SaveCHSFile();
		}
		
		return bRetVal;
	}

	bool CRandomTableEncDec::FileLoadEncData(HWND hWnd, const tstring strFullPath)
	{
		bool bRetVal = false;

		int find = strFullPath.rfind(_T("\\")) + 1;
		int ext = strFullPath.rfind(_T("."));

		if (ext == -1) {
			ext = strFullPath.length();
		}

		m_strPath = strFullPath.substr(0, find);
		m_strFileName = strFullPath.substr(find, ext - find);
		m_iExistsCnt = 0;

		m_iListTransEnc.clear();
		m_iListTransEnc2.clear();
		m_iListRestoreDec.clear();

		if (LoadCRTFile()) {
			bRetVal = LoadCHSFile(hWnd);
		}

		return bRetVal;
	}

	bool CRandomTableEncDec::DrawImgMsg(HDC hDC, int x, int y)
	{
		bool bRetVal = false;
		HDC hMemDC;
		BITMAP bm;
		HBITMAP hBitOld;

		HBITMAP hBitImg;

		if (m_enDataEnc == Image_BMP) {
			hBitImg = m_hBitMSG_Enc;
		}
		else {
			hBitImg = m_hBitMSG;
		}

		if (hBitImg) {
			hMemDC = CreateCompatibleDC(hDC);
			GetObject(hBitImg, sizeof(bm), &bm);

			hBitOld = (HBITMAP)SelectObject(hMemDC, hBitImg);

			BitBlt(hDC, x, y, bm.bmWidth, bm.bmHeight, hMemDC, 0, 0, SRCCOPY);

			SelectObject(hMemDC, hBitOld);
			DeleteDC(hMemDC);

			bRetVal = true;
		}

		return bRetVal;
	}

	int CRandomTableEncDec::GetImgStrHeight()
	{
		BITMAP bm;

		int iRetVal = 0;

		if (m_enDataEnc == Image_BMP || m_enDataEnc == None) {
			if (m_hBitMSG) {
				GetObject(m_hBitMSG, sizeof(bm), &bm);
				iRetVal = bm.bmHeight;
			}
			else if (m_hBitMSG_Enc) {
				GetObject(m_hBitMSG_Enc, sizeof(bm), &bm);
				iRetVal = bm.bmHeight;
			}
		}

		return iRetVal;
	}

	const int CRandomTableEncDec::GetEncRepeatCnt()
	{
		return m_iEncRepeatCnt;
	}

	void CRandomTableEncDec::SetEncRepeatCnt(int iEncRepeatCnt)
	{
		m_iEncRepeatCnt = iEncRepeatCnt;
	}

	void CRandomTableEncDec::ReleaseImgHandle()
	{
		if (m_hBitMSG) {
			DeleteObject(m_hBitMSG);
			m_hBitMSG = NULL;
		}

		if (m_hBitMSG_Enc) {
			DeleteObject(m_hBitMSG_Enc);
			m_hBitMSG_Enc = NULL;
		}
	}

	void CRandomTableEncDec::SecTransEnc(list<int> iListTransEnc)
	{
		m_iListTransEnc.clear();
		m_iListTransEnc = iListTransEnc;

		// 1차 암호화가 아니면 2번 3번 암호화 테이블을 생성한다.
		if (m_enEncMode != NormalEnc) {
			GenPassTable(2);
			GenPassTable(3);
		}

		list<int> iSecListTransEnc;
		list<int> iSecListTransEnc2;

		// 홀수 앞 짝수 뒤 또는 짝수 앞 홀수 뒤 정렬 링크드 리스트 변수 선언
		list<int> iSecListTransEncOdd;		// 홀수
		list<int> iSecListTransEncEven;		// 짝수

		list<int>* iListPassTable;
		list<int>* piListTransEnc;

		list<int>::iterator t;

		int i, j;

		int iPos = 0;

		if (m_enEncMode == HalfEnc) {				// 2차 난수화(반반)
			int nFirEncSize = iListTransEnc.size();
			int nHalfSize = nFirEncSize / 2;	// 만약 크기가 홀수라면 마지막 반이 앞의 반보다 1만큼 크게 된다.

			for (const auto& EncVal : iListTransEnc) {
				if (iPos < nHalfSize) {
					iListPassTable = &m_iListPassTable2;
				}

				else {
					iListPassTable = &m_iListPassTable3;
				}

				t = find(iListPassTable->begin(), iListPassTable->end(), EncVal);


				for (i = 0; i < m_nTransNum; i++) {
					if (*t == iListPassTable->back()) {
						t = find(iListPassTable->begin(), iListPassTable->end(), iListPassTable->front());
					}
					else {
						t++;
					}
				}

				iSecListTransEnc.push_back(*t);

				iPos++;
			}
		}
		else if (m_enEncMode == SlurpsEnc) {		// 2차 난수화(홀짝)
			for (const auto& EncVal : iListTransEnc) {
				if (iPos % 2 == 0) {
					iListPassTable = &m_iListPassTable2;
				}
				else {
					iListPassTable = &m_iListPassTable3;
				}

				t = find(iListPassTable->begin(), iListPassTable->end(), EncVal);


				for (i = 0; i < m_nTransNum; i++) {
					if (*t == iListPassTable->back()) {
						t = find(iListPassTable->begin(), iListPassTable->end(), iListPassTable->front());
					}
					else {
						t++;
					}
				}

				iSecListTransEnc.push_back(*t);

				iPos++;
			}
		}
		else if (m_enEncMode == OddFirstHalf || m_enEncMode == EvenFirstHalf) {

			piListTransEnc = &iListTransEnc;
			for (j = 0; j < m_iEncRepeatCnt; j++) {
				for (const auto& EncVal : *piListTransEnc) {
					if (iPos % 2 == 0) {
						iListPassTable = &m_iListPassTable2;
					}
					else {
						iListPassTable = &m_iListPassTable3;
					}

					t = find(iListPassTable->begin(), iListPassTable->end(), EncVal);


					for (i = 0; i < m_nTransNum; i++) {
						if (*t == iListPassTable->back()) {
							t = find(iListPassTable->begin(), iListPassTable->end(), iListPassTable->front());
						}
						else {
							t++;
						}
					}

					if (iPos % 2 == 0) {
						iSecListTransEncOdd.push_back(*t);
					}
					else {
						iSecListTransEncEven.push_back(*t);
					}

					iPos++;
				}

				if (m_enEncMode == OddFirstHalf) {
					iSecListTransEnc.assign(iSecListTransEncOdd.begin(), iSecListTransEncOdd.end());

					for (const auto& EncVal : iSecListTransEncEven) {
						iSecListTransEnc.push_back(EncVal);
					}
				}
				else {
					iSecListTransEnc.assign(iSecListTransEncEven.begin(), iSecListTransEncEven.end());

					for (const auto& EncVal : iSecListTransEncOdd) {
						iSecListTransEnc.push_back(EncVal);
					}
				}

				iPos = 0;

				iSecListTransEncOdd.clear();
				iSecListTransEncEven.clear();

				iSecListTransEnc2.clear();
				iSecListTransEnc2.assign(iSecListTransEnc.begin(), iSecListTransEnc.end());
				iSecListTransEnc.clear();

				piListTransEnc = &iSecListTransEnc2;
			}

			iSecListTransEnc = *piListTransEnc;
		}
		else {
			iSecListTransEnc.clear();
		}

		m_iListTransEnc2.clear();
		m_iListTransEnc2 = iSecListTransEnc;
	}

	void CRandomTableEncDec::SecRestoreDec()
	{
		list<int> iSecListRestoreDec;
		list<int> iSecListRestoreDec2;
		list<int>* piListPassTable = nullptr;

		list<int> iListTransEnc;
		list<int>* piListTransEnc;

		// 홀수 앞 짝수 뒤 또는 짝수 앞 홀수 뒤 정렬 링크드 리스트 변수 선언
		list<int> iSecListTransEncOdd;		// 홀수
		list<int> iSecListTransEncEven;		// 짝수

		list<int>::iterator t;

		list<int>::iterator tOdd;
		list<int>::iterator tEven;

		int i, j;

		int iPos = 0;

		int iTotalSize;

		if (m_enEncMode != NormalEnc) {
			iListTransEnc = m_iListTransEnc2;
		}
		else {
			iListTransEnc = m_iListTransEnc;
		}

		if (m_enEncMode == HalfEnc) {				// 2차 복호화(반반)
			for (const auto& EncVal : iListTransEnc) {
				int nFirEncSize = iListTransEnc.size();
				int nHalfSize = nFirEncSize / 2;	// 만약 크기가 홀수라면 마지막 반이 앞의 반보다 1만큼 크게 된다.

				if (iPos < nHalfSize) {
					piListPassTable = &m_iListPassTable2;
				}

				else {
					piListPassTable = &m_iListPassTable3;
				}

				t = find(piListPassTable->begin(), piListPassTable->end(), EncVal);

				for (i = 0; i < m_nTransNum; i++) {
					if (*t == piListPassTable->front()) {
						t = find(piListPassTable->begin(), piListPassTable->end(), piListPassTable->back());
					}
					else {
						t--;
					}
				}

				iSecListRestoreDec.push_back(*t);

				iPos++;
			}
		}
		else if (m_enEncMode == SlurpsEnc) {		// 2차 복호화(홀짝)
			for (const auto& EncVal : iListTransEnc) {
				if (iPos % 2 == 0) {
					piListPassTable = &m_iListPassTable2;
				}
				else {
					piListPassTable = &m_iListPassTable3;
				}

				t = find(piListPassTable->begin(), piListPassTable->end(), EncVal);

				for (i = 0; i < m_nTransNum; i++) {
					if (*t == piListPassTable->front()) {
						t = find(piListPassTable->begin(), piListPassTable->end(), piListPassTable->back());
					}
					else {
						t--;
					}
				}

				iSecListRestoreDec.push_back(*t);

				iPos++;
			}
		}
		else if (m_enEncMode == OddFirstHalf) {
			iTotalSize = iListTransEnc.size();

			piListTransEnc = &iListTransEnc;

			for (j = 0; j < m_iEncRepeatCnt; j++) {
				for (const auto& EncVal : *piListTransEnc) {
					if (iTotalSize / 2 > iPos) {
						piListPassTable = &m_iListPassTable2;
					}
					else {
						piListPassTable = &m_iListPassTable3;
					}

					t = find(piListPassTable->begin(), piListPassTable->end(), EncVal);

					for (i = 0; i < m_nTransNum; i++) {
						if (*t == piListPassTable->front()) {
							t = find(piListPassTable->begin(), piListPassTable->end(), piListPassTable->back());
						}
						else {
							t--;
						}
					}

					if (iTotalSize / 2 > iPos) {
						iSecListTransEncOdd.push_back(*t);
					}
					else {
						iSecListTransEncEven.push_back(*t);
					}

					iPos++;
				}

				iTotalSize = iSecListTransEncOdd.size();
				iTotalSize += iSecListTransEncEven.size();

				tOdd = iSecListTransEncOdd.begin();
				tEven = iSecListTransEncEven.begin();

				for (i = 0; i < iTotalSize; i++) {
					if (i % 2 == 0) {
						iSecListRestoreDec.push_back(*tOdd);
						tOdd++;
					}
					else {
						iSecListRestoreDec.push_back(*tEven);
						tEven++;
					}
				}

				iPos = 0;
				iSecListTransEncOdd.clear();
				iSecListTransEncEven.clear();

				iSecListRestoreDec2.clear();
				iSecListRestoreDec2.assign(iSecListRestoreDec.begin(), iSecListRestoreDec.end());
				iSecListRestoreDec.clear();

				piListTransEnc = &iSecListRestoreDec2;
			}

			iSecListRestoreDec = *piListTransEnc;
		}
		else if (m_enEncMode == EvenFirstHalf) {
			iTotalSize = iListTransEnc.size();

			piListTransEnc = &iListTransEnc;

			for (j = 0; j < m_iEncRepeatCnt; j++) {
				for (const auto& EncVal : *piListTransEnc) {
					if (iTotalSize / 2 > iPos) {
						piListPassTable = &m_iListPassTable3;
					}
					else {
						piListPassTable = &m_iListPassTable2;
					}

					t = find(piListPassTable->begin(), piListPassTable->end(), EncVal);

					for (i = 0; i < m_nTransNum; i++) {
						if (*t == piListPassTable->front()) {
							t = find(piListPassTable->begin(), piListPassTable->end(), piListPassTable->back());
						}
						else {
							t--;
						}
					}

					if (iTotalSize / 2 > iPos) {
						iSecListTransEncOdd.push_back(*t);
					}
					else {
						iSecListTransEncEven.push_back(*t);
					}

					iPos++;
				}

				iTotalSize = iSecListTransEncOdd.size();
				iTotalSize += iSecListTransEncEven.size();

				tOdd = iSecListTransEncOdd.begin();
				tEven = iSecListTransEncEven.begin();

				for (i = 0; i < iTotalSize; i++) {
					if (i % 2 == 0) {
						iSecListRestoreDec.push_back(*tEven);
						tEven++;
					}
					else {
						iSecListRestoreDec.push_back(*tOdd);
						tOdd++;
					}
				}

				iPos = 0;
				iSecListTransEncOdd.clear();
				iSecListTransEncEven.clear();

				iSecListRestoreDec2.clear();
				iSecListRestoreDec2.assign(iSecListRestoreDec.begin(), iSecListRestoreDec.end());
				iSecListRestoreDec.clear();

				piListTransEnc = &iSecListRestoreDec2;
			}

			iSecListRestoreDec = *piListTransEnc;
		}
		else {
			iSecListRestoreDec = iListTransEnc;
		}

		m_iListTransEnc.clear();
		m_iListTransEnc = iSecListRestoreDec;
	}

	tstring CRandomTableEncDec::strFullPathAndFile()
	{
		return m_strPath + m_strFileName + (m_iExistsCnt != 0 ? to_tstring(m_iExistsCnt) : _T(""));
	}

	bool CRandomTableEncDec::SaveCRTFile()
	{
		bool bRetVal = false;

		ifstream read;
		ofstream ofs;
		int i;

		// 난수화 된 테이블이 생성되었는지 확인
		// 생성이 되지 않았다면 false를 반환
		if (m_enEncMode == NormalEnc && m_iListPassTable.size() == 0) {
			return false;
		}
		else if(m_enEncMode != NormalEnc) {
			if (m_iListPassTable.size() == 0 || m_iListPassTable2.size() == 0 || m_iListPassTable3.size() == 0) {
				return false;
			}
		}

		try {
			// 기존의 파일이 있는지 검증
			bool bExists = true;
			tstring strFullFileName = _T("");

			m_iExistsCnt = 0;

			while (bExists) {
				strFullFileName = strFullPathAndFile() + _T(".") + EXT_CRT;

				read.open(strFullFileName);
				if (read) {
					++m_iExistsCnt;
					read.close();
				}
				else {
					bExists = false;
				}
			}

			//직렬화를 위한 파일 스트림 생성 
			ofs = ofstream(strFullFileName, ios_base::binary);

			//text_oaarchive에 출력 파일 스트림 지정 
			archive::text_oarchive oa(ofs);

			// 직렬화 수행

			// 난수 테이블에서 이동값 저장(1 ~ 15 사이의 값)
			oa << m_nTransNum;

			// 난수화 모드 저장
			oa << m_enEncMode;

			if (m_enEncMode > SlurpsEnc) {
				oa << m_iEncRepeatCnt;
			}

			// 기초적인 난수 테이블1 저장
			for (const auto& iPassTValue : m_iListPassTable) {
				oa << iPassTValue;
			}

			if (m_enEncMode != NormalEnc) {
				for (const auto& iPassTValue : m_iListPassTable2) {
					oa << iPassTValue;
				}

				for (const auto& iPassTValue : m_iListPassTable3) {
					oa << iPassTValue;
				}
			}

			if (ofs) {
				ofs.close();
			}
			if (read) {
				read.close();
			}

			bRetVal = true;
		}
		catch (...) {
			if (ofs) {
				ofs.close();
			}
			if (read) {
				read.close();
			}
		}

		return bRetVal;
	}

	bool CRandomTableEncDec::SaveCHSFile()
	{
		bool bRetVal = false;
		BITMAP bitmap;

#ifdef UNICODE
		unsigned short mark = 0xFEFF;
#endif

		ifstream read;
		ofstream ofs;
		int i;

		list<int>* piListTransEnc;
		list<int>::iterator t;
		int iTmp;
		BYTE bByte;

		// 난수화 된 테이블이 생성되었는지 확인
		// 생성이 되지 않았다면 false를 반환
		if (m_enEncMode == NormalEnc && m_iListTransEnc.size() == 0) {
			return false;
		}
		else if (m_enEncMode != NormalEnc && m_iListTransEnc2.size() == 0) {
				return false;
		}

		try {
			// 기존의 파일이 있는지 확인 하여
			// 있으면 삭제 처리
			tstring strFullFileName = _T("");

			strFullFileName = strFullPathAndFile() + _T(".") + EXT_CHS;

			read.open(strFullFileName);

			if (read) {
				read.close();
				// 파일 삭제 처리
				DeleteFile(strFullFileName.c_str());
			}

			//직렬화를 위한 파일 스트림 생성 
			ofs = ofstream(strFullFileName, ios_base::binary);

			//text_oaarchive에 출력 파일 스트림 지정
			archive::text_oarchive oa(ofs);

			// 직렬화 수행
			if (m_enEncMode == NormalEnc) {
				piListTransEnc = &m_iListTransEnc;
			}
			else {
				piListTransEnc = &m_iListTransEnc2;
			}

			i = 0;
			bByte = 0;

			// 암호화 된 파일 타입 저장
			if (m_hBitMSG_Enc) {
				oa << RandTableDataType::Image_BMP;

				// 비트맵 파일 해더를 파일에 기록한다.
				// hBitmap으로 bitmap을 가져온다.
				GetObject(m_hBitMSG_Enc, sizeof(bitmap), (LPSTR)&bitmap);

				oa << bitmap.bmWidth;
				oa << bitmap.bmHeight;
				oa << (WORD)24;			// 기본적인 비트 카운트는 24다
			}
			else {
				// 유니코드 문자열 확인 기호
#ifdef UNICODE
				// 기본적으로 텍스트로 지정한다.
				oa << RandTableDataType::UniText;
				oa << mark;
#else
				oa << RandTableDataType::AnsiText;
#endif
			}

			for (t = piListTransEnc->begin(); t != piListTransEnc->end(); t++) {
				iTmp = *t;

				switch (i % 2) {
				case 0:
					iTmp <<= 4;
					iTmp &= 0xF0;
					break;
				case 1:
					iTmp &= 0x0F;
					break;
				}

				bByte += iTmp;

				if (i % 2 == 1) {
					oa << (BYTE)bByte;
					bByte = 0;
				}

				i++;
			}

			if (ofs) {
				ofs.close();
			}
			if (read) {
				read.close();
			}

			bRetVal = true;
		}
		catch (...) {
			if (ofs) {
				ofs.close();
			}
			if (read) {
				read.close();
			}
		}

		return bRetVal;
	}
	bool CRandomTableEncDec::LoadCRTFile()
	{
		bool bRetVal = false;

		int i = 0;
		ifstream read;

		int iTmp;

		try {

			read.open(strFullPathAndFile() + _T(".") + EXT_CRT, ios_base::binary);
			if (!read) {
				return false;
			}

			archive::text_iarchive ia(read);

			// 난수 테이블에서 이동값 먼저 읽기(1 ~ 15 사이의 값)
			ia >> m_nTransNum;

			// 난수화 모드 읽기
			ia >> m_enEncMode;

			if (m_enEncMode > SlurpsEnc) {
				ia >> m_iEncRepeatCnt;
			}

			// 기초적인 난수 테이블1 읽기
			m_iListPassTable.clear();
			for (i = 0; i < 16; i++) {
				ia >> iTmp;
				m_iListPassTable.push_back(iTmp);
			}


			if (m_enEncMode != NormalEnc) {
				m_iListPassTable2.clear();
				for (i = 0; i < 16; i++) {
					ia >> iTmp;
					m_iListPassTable2.push_back(iTmp);
				}

				m_iListPassTable3.clear();
				for (i = 0; i < 16; i++) {
					ia >> iTmp;
					m_iListPassTable3.push_back(iTmp);
				}
			}

			if (read) {
				read.close();
			}

			bRetVal = true;
		}
		catch (...) {
			if (read) {
				read.close();
			}
		}

		return bRetVal;
	}
	bool CRandomTableEncDec::LoadCHSFile(HWND hWnd)
	{
		bool bRetVal = false;

		int i = 0;
		ifstream read;

		int iTmp;

		const unsigned short mark = 0xFEFF;
		unsigned short uUni;

		list<int>* piListTransEnc = nullptr;

		BYTE bByte;

		RandTableDataType enEncDataType;

		try {

			read.open(strFullPathAndFile() + _T(".") + EXT_CHS, ios_base::binary);
			if (!read) {
				return false;
			}
			
			if (m_enEncMode == NormalEnc) {
				piListTransEnc = &m_iListTransEnc;
			}
			else {
				piListTransEnc = &m_iListTransEnc2;
			}

			piListTransEnc->clear();

			archive::text_iarchive ia(read);

			// 암호화 된 파일 타입 읽기
			ia >> enEncDataType;
			if (RandTableDataType::Image_BMP == enEncDataType) {
				ia >> m_lBitWidth;
				ia >> m_lBitHeight;
				ia >> m_wBitCnt;
			}
			else {		// 기본적으로 텍스트로 인식
				// 초기 2바이트 읽기
				// 유니코드 문자 인지 확인
				ia >> uUni;

				if (uUni == mark) {
					m_enDataEnc = UniText;
				}
				else {
					m_enDataEnc = AnsiText;
				}

				if (m_enDataEnc == AnsiText) {
					iTmp = uUni;
					iTmp &= 0xF0;
					iTmp >>= 4;
					piListTransEnc->push_back(iTmp);

					iTmp = uUni;
					iTmp &= 0x0F;
					piListTransEnc->push_back(iTmp);
				}
			}

			bRetVal = true;

			// 읽다가 끝을 넘어서면 catch로 간다.
			// 일부러 무한 루프를 돌렸다.
			while(true) {
				ia >> bByte;

				iTmp = bByte;
				iTmp &= 0xF0;
				iTmp >>= 4;

				piListTransEnc->push_back(iTmp);

				iTmp = bByte;
				iTmp &= 0x0F;
				piListTransEnc->push_back(iTmp);
			}
		}
		catch (...) {
			if (read) {
				read.close();
			}
		}

		if (enEncDataType == Image_BMP) {
			HDC hDC = GetDC(hWnd);
			SetImgEnc2HBitmap(hDC, m_lBitWidth, m_lBitHeight, m_wBitCnt, m_hBitMSG_Enc, piListTransEnc);
			ReleaseDC(hWnd, hDC);

			m_enDataEnc = Image_BMP;
		}

		return bRetVal;
	}
	void CRandomTableEncDec::SetImgEnc2HBitmap(HDC hDC, LONG lWidth, LONG lHeight, WORD wBitCnt, HBITMAP& hBit, list<int>* piListImgData)
	{
		list<int>* piListTransEnc;
		list<int>::iterator t;
		int i, j;
		int iTmp;
		BYTE bBuf = 0;

		if (hDC == nullptr || piListImgData == nullptr) {
			return;
		}

		// Bitmap Header 정보 설정
		BITMAPINFOHEADER bi;
		bi.biSize = sizeof(BITMAPINFOHEADER);
		bi.biWidth = lWidth;
		bi.biHeight = lHeight;
		bi.biPlanes = 1;
		bi.biBitCount = wBitCnt;
		bi.biCompression = BI_RGB;
		bi.biSizeImage = 0;
		bi.biXPelsPerMeter = 0;
		bi.biYPelsPerMeter = 0;
		bi.biClrUsed = 0;
		bi.biClrImportant = 0;

		int iPalSize = (bi.biBitCount == 24 ? 0 : 1 << bi.biBitCount) * sizeof(RGBQUAD);
		int iSize = bi.biSize + iPalSize + bi.biSizeImage;

		if (bi.biSizeImage == 0) {
			// 해더 사이즈 설정이 안되면 강제 계산 설정
			bi.biSizeImage = ((lWidth * bi.biBitCount + 31) & ~31) / 8 * lHeight;
		}

		// 메모리 할당 (bitmap header)
		BITMAPINFO* header = (BITMAPINFO*)malloc(sizeof(BITMAPINFOHEADER) + iPalSize);
		header->bmiHeader = bi;

		PBYTE body = (PBYTE)malloc(header->bmiHeader.biSizeImage);

		i = 0;
		j = 0;
		// 암호화된 이미지 데이터를 body에 다시 입력
		for (t = piListImgData->begin(); t != piListImgData->end(); t++) {
			iTmp = *t;

			switch (i % 2) {
			case 0:
				iTmp <<= 4;
				iTmp &= 0xF0;
				break;
			case 1:
				iTmp &= 0x0F;
				break;
			}

			bBuf += iTmp;

			if (i % 2 == 1) {
				body[j++] = bBuf;
				bBuf = 0;
			}

			i++;
		}

		if (hBit) {
			DeleteObject(hBit);
			hBit = NULL;
		}

		hBit = CreateCompatibleBitmap(hDC, lWidth, lHeight);

		// 비트맵 암호화 상태로 이미지 데이터 저장
		SetDIBits(hDC, hBit, 0, lHeight, body, header, DIB_RGB_COLORS);

		// 메모리 반환
		free(header);
		free(body);
	}
	INT_PTR CRandomTableEncDec::DlgRT_ConfigProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		static CRandomTableEncDec* pObj;
		WPARAM wChkRTCfgEnable;
		WORD wmId;
		WORD wmEvent;

		bool bChkState;

		switch (uMsg) {
		case WM_INITDIALOG:
			pObj = (CRandomTableEncDec*)lParam;

			SetWindowText(hDlg, _T("Rand Table Config"));

			if (pObj->m_bRT_Cfg_Enable) {
				wChkRTCfgEnable = BST_CHECKED;
			}
			else {
				wChkRTCfgEnable = BST_UNCHECKED;
			}
			SendMessage(GetDlgItem(hDlg, IDC_CHK_RT_PASSIVITY_ENABLE), BM_SETCHECK, wChkRTCfgEnable, 0);

			pObj->SetShiftCmb(hDlg);

			pObj->SetRTCmbRecursive(hDlg, IDC_CMB_RT1_00, list<int>(), 1, 0);
			pObj->SetRTCmbRecursive(hDlg, IDC_CMB_RT2_00, list<int>(), 2, 0);
			pObj->SetRTCmbRecursive(hDlg, IDC_CMB_RT3_00, list<int>(), 3, 0);

			for (int i = 0; i < 16 * 3 + 1; i++) {
				EnableWindow(GetDlgItem(hDlg, IDC_CMB_RT1_00 + i), pObj->m_bRT_Cfg_Enable);
			}

			return TRUE;

		case WM_COMMAND:
			wmId = LOWORD(wParam);
			wmEvent = HIWORD(wParam);
			switch (wmId) {
			case IDOK:
				wChkRTCfgEnable = SendMessage(GetDlgItem(hDlg, IDC_CHK_RT_PASSIVITY_ENABLE), BM_GETCHECK, 0, 0);

				if (wChkRTCfgEnable == BST_UNCHECKED) {
					pObj->m_bRT_Cfg_Enable = false;
				}
				else {
					pObj->m_bRT_Cfg_Enable = true;

					pObj->SaveRandTable(hDlg);
				}
			case IDCANCEL:
				EndDialog(hDlg, wmId);
				return TRUE;
			case IDC_CHK_RT_PASSIVITY_ENABLE:
				wChkRTCfgEnable = SendMessage(GetDlgItem(hDlg, IDC_CHK_RT_PASSIVITY_ENABLE), BM_GETCHECK, 0, 0);

				if (wChkRTCfgEnable == BST_UNCHECKED) {
					bChkState = false;
				}
				else {
					bChkState = true;
				}

				for (int i = 0; i < 16 * 3 + 1; i++) {
					EnableWindow(GetDlgItem(hDlg, IDC_CMB_RT1_00 + i), bChkState);
				}

				return TRUE;
			}

			pObj->ChangeRTCmb(hDlg, wmId, wmEvent);
			break;

		}
		return FALSE;
	}

	INT_PTR CRandomTableEncDec::ShowDlgRTConfig(HWND hWnd)
	{
		return DialogBoxParam(GetDllHInst(), MAKEINTRESOURCE(IDD_RT_CONFIG), hWnd, DlgRT_ConfigProc, (LPARAM)this);
	}

	HINSTANCE CRandomTableEncDec::GetDllHInst()
	{
		return GetModuleHandle(TEXT("CHSRandT.dll"));
	}

	void CRandomTableEncDec::SetRTCmbRecursive(HWND hDlg, UINT uCMD_ID, list<int> iListRndTable, int iRndTableNum, int iDepth)
	{
		HWND hCmbItem = GetDlgItem(hDlg, uCMD_ID);
		TCHAR strHex[4] = { 0, };
		int iIdx = -1;

		if (iDepth == 0) {
			iListRndTable.clear();

			for (int i = 0; i < 16; i++) {
				iListRndTable.push_back(i);
			}
		}

		SendMessage(hCmbItem, CB_RESETCONTENT, 0, 0);

		for (const auto& iVal : iListRndTable) {
			wsprintf(strHex, _T("0x%1X"), iVal);

			SendMessage(hCmbItem, CB_ADDSTRING, 0, (LPARAM)strHex);
		}

		list<int>* piListRndT;
		list<int>::iterator t;
		
		switch (iRndTableNum) {
		case 1:
			piListRndT = &m_iListPassTable;
			break;
		case 2:
			piListRndT = &m_iListPassTable2;
			break;
		case 3:
			piListRndT = &m_iListPassTable3;
			break;
		default:
			return;
		}

		int iTargetVal = -1;

		if (piListRndT->size() > 0) {
			t = piListRndT->begin();

			for (int i = 0; i < iDepth; i++) {
				t++;
			}

			iTargetVal = *t;
		}
		else {
			iTargetVal = iDepth;
		}

		wsprintf(strHex, _T("0x%1X"), iTargetVal);
		iIdx = SendMessage(hCmbItem, CB_FINDSTRINGEXACT, -1, (LPARAM)strHex);

		if (iIdx != CB_ERR) {
			SendMessage(hCmbItem, CB_SETCURSEL, iIdx, 0);
		}
		else {
			return;
		}

		++iDepth;

		if (iDepth > 15) {
			return;
		}

		iListRndTable.remove(iTargetVal);

		SetRTCmbRecursive(hDlg, uCMD_ID + 1, iListRndTable, iRndTableNum, iDepth);

		return;
	}

	void CRandomTableEncDec::SetShiftCmb(HWND hDlg)
	{
		HWND hCmbItem = GetDlgItem(hDlg, IDC_CMB_SHIFT_VAL);
		TCHAR strNum[3] = { 0, };
		int iIdx;

		for (int i = 1; i < 16; i++) {
			wsprintf(strNum, _T("%02d"), i);

			SendMessage(hCmbItem, CB_ADDSTRING, 0, (LPARAM)strNum);
		}

		wsprintf(strNum, _T("%02d"), m_nTransNum);
		iIdx = SendMessage(hCmbItem, CB_FINDSTRINGEXACT, -1, (LPARAM)strNum);

		if (iIdx != CB_ERR) {
			SendMessage(hCmbItem, CB_SETCURSEL, iIdx, 0);
		}
		else {
			SendMessage(hCmbItem, CB_SETCURSEL, 0, 0);
		}
	}

	void CRandomTableEncDec::ChangeRTCmb(HWND hDlg, UINT uCMD_ID, WORD wNotify)
	{
		int i;
		int iStart, iEnd;

		HWND hWndCmb;

		int iIdx;
		static TCHAR strTargetHex[4];
		TCHAR strCmpHex[4];
		TCHAR strSetChange[4];

		// TCHAR strHex[4] = { 0, };

		if (wNotify == CBN_DROPDOWN) {

			hWndCmb = GetDlgItem(hDlg, uCMD_ID);
			iIdx = SendMessage(hWndCmb, CB_GETCURSEL, 0, 0);

			if (iIdx != CB_ERR) {
				SendMessage(hWndCmb, CB_GETLBTEXT, iIdx, (LPARAM)strTargetHex);
			}

			return;
		}
		else if (wNotify == CBN_SELCHANGE) {
			hWndCmb = GetDlgItem(hDlg, uCMD_ID);
			iIdx = SendMessage(hWndCmb, CB_GETCURSEL, 0, 0);

			if (iIdx != CB_ERR) {
				SendMessage(hWndCmb, CB_GETLBTEXT, iIdx, (LPARAM)strSetChange);
			}
			else {
				return;
			}
		}
		else {
			return;
		}

		iStart = uCMD_ID + 1;

		if (uCMD_ID >= IDC_CMB_RT1_00 && uCMD_ID <= IDC_CMB_RT1_15) {
			iEnd = IDC_CMB_RT1_15;
		}
		else if (uCMD_ID >= IDC_CMB_RT2_00 && uCMD_ID <= IDC_CMB_RT2_15) {
			iEnd = IDC_CMB_RT2_15;
		}
		else if (uCMD_ID >= IDC_CMB_RT3_00 && uCMD_ID <= IDC_CMB_RT3_15) {
			iEnd = IDC_CMB_RT3_15;
		}
		else {
			return;
		}

		for (i = iStart; i <= iEnd; i++) {
			hWndCmb = GetDlgItem(hDlg, i);

			iIdx = SendMessage(hWndCmb, CB_GETCURSEL, 0, 0);

			if (iIdx != CB_ERR) {
				SendMessage(hWndCmb, CB_GETLBTEXT, iIdx, (LPARAM)strCmpHex);
			}

			if (_tcscmp(strSetChange, strCmpHex) == 0) {
				SendMessage(hWndCmb, CB_DELETESTRING, iIdx, 0);

				SendMessage(hWndCmb, CB_ADDSTRING, 0, (LPARAM)strTargetHex);

				iIdx = SendMessage(hWndCmb, CB_FINDSTRINGEXACT, -1, (LPARAM)strTargetHex);

				SendMessage(hWndCmb, CB_SETCURSEL, iIdx, 0);

				break;
			}
			else {

				iIdx = SendMessage(hWndCmb, CB_FINDSTRINGEXACT, -1, (LPARAM)strSetChange);

				if (iIdx != CB_ERR) {
					SendMessage(hWndCmb, CB_DELETESTRING, iIdx, 0);
					SendMessage(hWndCmb, CB_ADDSTRING, 0, (LPARAM)strTargetHex);
				}
			}
		}
	}

	void CRandomTableEncDec::SaveRandTable(HWND hDlg)
	{
		HWND hWndCmb;
		TCHAR strHex[4] = { 0, };
		int iNum;
		int iIdx;

		m_iListPassTable.clear();

		for (int i = 0; i < 16; i++) {
			hWndCmb = GetDlgItem(hDlg, IDC_CMB_RT1_00 + i);
			iIdx = SendMessage(hWndCmb, CB_GETCURSEL, 0, 0);

			if (iIdx != CB_ERR) {
				SendMessage(hWndCmb, CB_GETLBTEXT, iIdx, (LPARAM)strHex);

				iNum = _tcstol(strHex, nullptr, 16);

				m_iListPassTable.push_back(iNum);
			}
			else {
				m_bRT_Cfg_Enable = false;

				return;
			}
		}

		m_iListPassTable2.clear();

		for (int i = 0; i < 16; i++) {
			hWndCmb = GetDlgItem(hDlg, IDC_CMB_RT2_00 + i);
			iIdx = SendMessage(hWndCmb, CB_GETCURSEL, 0, 0);

			if (iIdx != CB_ERR) {
				SendMessage(hWndCmb, CB_GETLBTEXT, iIdx, (LPARAM)strHex);

				iNum = _tcstol(strHex, nullptr, 16);

				m_iListPassTable2.push_back(iNum);
			}
			else {
				m_bRT_Cfg_Enable = false;

				return;
			}
		}

		m_iListPassTable3.clear();

		for (int i = 0; i < 16; i++) {
			hWndCmb = GetDlgItem(hDlg, IDC_CMB_RT3_00 + i);
			iIdx = SendMessage(hWndCmb, CB_GETCURSEL, 0, 0);

			if (iIdx != CB_ERR) {
				SendMessage(hWndCmb, CB_GETLBTEXT, iIdx, (LPARAM)strHex);

				iNum = _tcstol(strHex, nullptr, 16);

				m_iListPassTable3.push_back(iNum);
			}
			else {
				m_bRT_Cfg_Enable = false;

				return;
			}
		}

		iIdx = SendMessage(GetDlgItem(hDlg, IDC_CMB_SHIFT_VAL), CB_GETCURSEL, 0, 0);

		if (iIdx != CB_ERR) {
			SendMessage(GetDlgItem(hDlg, IDC_CMB_SHIFT_VAL), CB_GETLBTEXT, iIdx, (LPARAM)strHex);
			m_nTransNum = _ttoi(strHex);
		}
	}

	void CRandomTableEncDec::TextToImg(HDC hDC, tstring strMsg, int iMaxWidth, int iFontHeight, tstring strFontName)
	{
		HFONT ftSet, ftOld;
		LOGFONT lfSet;

		int iMaxCount = 0;
		SIZE szStrArea;
		int iX;

		HDC hMemDC;
		HBITMAP hBitOld;

		RECT rtBitmap, rtMsg;

		COLORREF clrYellow = RGB(255, 255, 0);
		HBRUSH hBrYellow, hBrOld;

		if (strMsg.length() == 0) {
			return;
		}

		if (m_hBitMSG) {
			DeleteObject(m_hBitMSG);
			m_hBitMSG = NULL;
		}

		hMemDC = CreateCompatibleDC(hDC);

		ZeroMemory(&lfSet, sizeof(lfSet));

		lfSet.lfHeight = iFontHeight;
		lfSet.lfCharSet = HANGEUL_CHARSET;
		_tcscpy_s(lfSet.lfFaceName, _countof(lfSet.lfFaceName), strFontName.c_str());
		ftSet = CreateFontIndirect(&lfSet);

		ftOld = (HFONT)SelectObject(hMemDC, ftSet);

		GetTextExtentExPoint(hMemDC, strMsg.c_str(), strMsg.length(), iMaxWidth, &iMaxCount, nullptr, &szStrArea);

		if (szStrArea.cx > iMaxWidth) {
			iX = szStrArea.cx / iMaxWidth;
			szStrArea.cy *= (iX + 1);
			szStrArea.cx = iMaxWidth;
		}

		SetRect(&rtBitmap, 0, 0, szStrArea.cx + IMG_WHITE_SPACE * 2, szStrArea.cy + IMG_WHITE_SPACE * 2);
		SetRect(&rtMsg, IMG_WHITE_SPACE, IMG_WHITE_SPACE, IMG_WHITE_SPACE + szStrArea.cx, IMG_WHITE_SPACE + szStrArea.cy);

		m_hBitMSG = CreateCompatibleBitmap(hDC, rtBitmap.right - rtBitmap.left, rtBitmap.bottom - rtBitmap.top);

		hBitOld = (HBITMAP)SelectObject(hMemDC, m_hBitMSG);

		FillRect(hMemDC, &rtBitmap, (HBRUSH)GetStockObject(WHITE_BRUSH));

		hBrYellow = CreateSolidBrush(clrYellow);
		hBrOld = (HBRUSH)SelectObject(hMemDC, hBrYellow);
		RoundRect(hMemDC, rtBitmap.left, rtBitmap.top, rtBitmap.right, rtBitmap.bottom, IMG_WHITE_SPACE / 2, IMG_WHITE_SPACE / 2);
		SelectObject(hMemDC, hBrOld);

		SetBkMode(hMemDC, TRANSPARENT);

		DrawText(hMemDC, strMsg.c_str(), -1, &rtMsg, DT_LEFT | DT_TOP | DT_WORDBREAK | DT_EDITCONTROL);

		SelectObject(hMemDC, hBitOld);
		DeleteDC(hMemDC);

		SelectObject(hMemDC, ftOld);
		DeleteObject(ftSet);

		m_enDataEnc = None;
	}
}
