#pragma once

// 초기 작업...
// 제목 : 난수화 테이블 참조 버전
// 만든이 : 최효석(Chaos kr goto)
// 전화번호 : 010-7121-6633
// 날짜 : 2024-0426_2156 (1차)
//        2024-0513_1153 (2차)
//		  2024-0513_1321
//		  2024-0515_1738 (3차) : 마지막 공개 코드
// 비공개 버전 시작
//		  2024-0517_1354 (CloseVer 0.2)
//		  2024-0517_2003 (CloseVer 0.3)
//		  2024-0524_1203 (CloseVer 0.4)
//		  2024-1031_0103 (CloseVer 0.5)
// 설명 : 문자열의 난수화와	복호화 검증 프로그램
// 실행환경 : Win32 API의 x64 비트에서 검증확인(WIN10에서 확인함)
// 유니코드 환경에서는 검증하였음.
// 멀티 바이트(Ansi)는 검증 속성에서 멀티바이트로 문자셋 변경후 테스트 확인 완료
// 2차 설명 : 1차 난수화(일반) 에서 2차 난수화인 반반 모드와, 홀짝 모드로 한번더 난수화를 만들고 또 복호화도 되게 만들었음)
// 2-1차 설명 : 유니코드와 멀티바이트 둘다 되는것 확인(Viewer의 #ifdef 를 확인)
//				화면출력을 위한 결과창의 데이터 표시 줄바꿈도 구현함.(이건 Viewer 에서 코드를 확인해야 함.)
// 3차 설명 : dll을 사용하는 테이블 난수화 첫 프로그램(기초 프로젝트)
//			  About 설명과 암호화 설정 콤보박스 선택시 오류 수정(클릭과 선택 때 2번 난수 테이블이 바뀌는 것을 선택후 한번만 바뀌게 수정)
// CloseVer 0.2 설명 : Boost 이용 파일 직렬화로 암호화 데이터 파일화와 저장된 암호화 파일을 불러오는 로직 구현
// CloseVer 0.3 설명 : 난수 모드 2개 추가(홀앞짝뒤, 짝앞홀뒤) = 파일화도 문제없음
// CloseVer 0.4 설명 : 문자열을 이미지화 한 후 홀앞짝뒤 또는 짝앞홀뒤를 선택 후 반복 카운트를 2 이상 주어 이미지를 깨지게 만들지만 복원하면 원래 이미지로 돌아오게 만듬
//		 			   이미지 난수화도 파일 출력을 지원
//					   일반, 반반, 홀짝은 문자가 있는 이미지의 색만 바꾸는 것으로 내용을 숨길수는 없음.
// CloseVer 0.5 설명 : 난수 테이블을 수동으로 조작할수 있는 대화상자 추가
//                     멀티바이트 컴파일 오류 수정(복원된 문자열 출력 코드 이상)
// 다음 예상 작업 : 일반 파일의 난수화 및 복원 작업 이지만... 이것은 렌섬웨어의 길 이므로 한번에 3개 이상 못하게 할 예정이고 4개 시도시 재부팅을 요구할 예정

#ifndef _CODE_
#define _CODE_

#include "framework.h"
#include <list>
#include <string>
#include <tchar.h>

// 선언할 코드 전체 작성
	// Tchar 를 위한 c++ 이 프로그램에 필수 문자열 라이브러리 원형 연결
#if defined(UNICODE) || defined(_UNICODE)
#define tstring wstring
#define to_tstring to_wstring
#else
#define tstring string
#define to_tstring to_string
#endif

using namespace std;
#endif

// CHSRANDT_API 를 선언해야 함.
#ifdef CHSRANDT_EXPORTS
#define CHSRANDT_API __declspec(dllexport)
#else
#define CHSRANDT_API __declspec(dllimport)
#endif

// 네임 스페이스 적용
namespace CHSRandT_Library
{
	// CHSRANDT_API 를 선언해야 함.
	class CHSRANDT_API CRandomTableEncDec
	{
	public:
		typedef enum en_RandTableEncMode {
			NormalEnc,			// 단순 1차 난수 테이블 사용
			HalfEnc,			// 1차 난수 테이블 암호화 상태 값에서 앞반의 2차 난수 테이블 변환과 뒤반으로 3차 난수 테이블 변환을 사용
			SlurpsEnc,			// 1차 난수 테이블에서 홀수의 2차 난수 테이블 변환과 짝으로 3차 난수 테이블 변환으로 암호화 적용
			OddFirstHalf,		// 1차 난수 테이블에서 홀수부분을 2차 난수화 하여 앞 반으로 뒤반은 짝수부분을 2차 난수화 하여 뒤쪽에 배치
			EvenFirstHalf		// 1차 난수 테이블에서 짝수부분을 2차 난수화 하여 앞 반으로 뒤반은 홀수부분을 2차 난수화 하여 뒤쪽에 배치
		} RandTableEncMode;

		typedef enum en_RandTableDataType {
			AnsiText,
			UniText,
			Image_BMP,
			Image_JPG,
			Image_PNG,
			File,
			None
		} RandTableDataType;

		CRandomTableEncDec();
		~CRandomTableEncDec();

		void SetEncMode(RandTableEncMode enEncMode = NormalEnc);
		RandTableEncMode GetEncMode();

		// void SetNumFormat(int nNumFormat);	// 사용 안함...
		void SetTransNum(int nTransNum = 0);
		int GetNumFormat();
		int GetTransNum();
		void GenPassTable(int iPassTarget = 1);

		const list<int> GetPassTable(int iPassTarget = 1);

		void TransWideMsgEnc(wstring strMsg);
		void TransAnsiMsgEnc(string strMsg);

		void TransImgEnc(HDC hDC);

		const list<int> GetTransEnc(int iPos = 1);

		void RestoreMsgDec();

		const list<int> GetRestoreDec();

		const tstring GetRestoreMsg();

		void GetRestoreImg(HDC hDC);

		bool FileSaveEncData(const tstring strFullPath);
		bool FileLoadEncData(HWND hWnd, const tstring strFullPath);

		void TextToImg(HDC hDC, tstring strMsg, int iMaxWidth, int iFontHeight, tstring strFontName);			// 데이터 난수화 시킬 텍스트 이미지 생성
		bool DrawImgMsg(HDC hDC, int x, int y);
		int GetImgStrHeight();

		const int GetEncRepeatCnt();
		void SetEncRepeatCnt(int iEncRepeatCnt);

		void ReleaseImgHandle();

		void SetImgEnc2HBitmap(HDC hDC, LONG lWidth, LONG lHeight, WORD wBitCnt, HBITMAP& hBit, list<int>* piListImgData);

		static INT_PTR DlgRT_ConfigProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);		// 난수 테이블 수동 조작 대화상자 처리 Proc

		INT_PTR ShowDlgRTConfig(HWND hWnd);		// 대화상자 출력 메소드

		HINSTANCE GetDllHInst();				// dll 인스턴스 반환 메소드

	private:
		int m_nNumFormat;		// 일반적으로 16진으로 고정
		int m_nTransNum;		// 16진 난수 테이블에서 고정된 이동 숫자
		RandTableEncMode m_enEncMode;	// 난수화 모드 1. 일반, 2. 반반, 3. 홀짝

		list<int> m_iListPassTable;		// 1차 난수 테이블
		list<int> m_iListPassTable2;	// 2차 중 하나의 난수 테이블
		list<int> m_iListPassTable3;	// 2차 중 두번째의 난수 테이블

		list<int> m_iListTransEnc;		// 1차 난수화 상태 리스트
		list<int> m_iListTransEnc2;		// 2차 난수화 상태 리스트
		list<int> m_iListRestoreDec;	// 복호화된 상태의 리스트

		int m_iEncRepeatCnt;

		void SecTransEnc(list<int> iListTransEnc);		// 2차 난수화를 시도해 주는 메서드
		void SecRestoreDec();							// 2차 난수화를 복원해 주는 메서드

		const wstring GetRestoreWideMsg();
		const string GetRestoreAnsiMsg();

		int convert_ansi_to_unicode_string(wstring& unicode, LPCSTR ansi, const size_t ansi_size);
		int convert_unicode_to_ansi_string(string& ansi, LPCWSTR unicode, const size_t unicode_size);

		// 저장하거나 불러올 암호화 파일명, 경로, 중복파일명 처리 카운트
		tstring m_strPath;
		tstring m_strFileName;
		int m_iExistsCnt;

		tstring strFullPathAndFile();			// 확장자를 뺀 나머지 경로와 이름

		// 저장하거나 불러올 파일의 확장자 상수 선언
		const tstring EXT_CRT = _T("crt");
		const tstring EXT_CHS = _T("chs");

		// 암호화 파일 저장
		bool SaveCRTFile();
		bool SaveCHSFile();

		// 암호화 파일 불러오기
		bool LoadCRTFile();
		bool LoadCHSFile(HWND hWnd);

		const int IMG_WHITE_SPACE = 10;
		RandTableDataType m_enDataEnc;
		HBITMAP m_hBitMSG;
		HBITMAP m_hBitMSG_Enc;
		LONG m_lBitWidth, m_lBitHeight;
		WORD m_wBitCnt;

		bool m_bRT_Cfg_Enable;		// 난수 테이블 수동 조작 활성화 멤버 변수

		void SetRTCmbRecursive(HWND hDlg, UINT uCMD_ID, list<int> iListRndTable, int iRndTableNum, int iDepth);		// 난수 테이블의 난수 콤보박스 조작 재귀 메서드

		void SetShiftCmb(HWND hDlg);		// 난수 테이블에서 자리 이동할때 사용되는 1 ~ 15 사이의 수 콤보박스 설정 메서드

		void ChangeRTCmb(HWND hDlg, UINT uCMD_ID, WORD wNotify);		// 난수 테이블에서 사용되는 콤보박스의 변경시 하위 콤보박스의 내용을 갱신해 주는 메서드

		void SaveRandTable(HWND hDlg);		// 대화상자를 닫을 때 "확인"을 누르면 설정된 난수값 상태를 저장할때 사용되는 메서드
	};
}
