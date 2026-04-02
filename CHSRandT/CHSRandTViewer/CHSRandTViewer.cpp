// CHSRandTViewer.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "CHSRandTViewer.h"

#include "CRandomTableEncDec.h"

using namespace CHSRandT_Library;

#define MAX_LOADSTRING 100

typedef enum en_Ctrl_IDS {
    ID_EDIT_MSG = 1000,
    ID_CMB_ENC_MODE,
    ID_EDIT_ENC_REPEAT_CNT,
    ID_UPDOWN_ENC_REPEAT_CNT,
    ID_BTN_TRANS_ENC,
    ID_BTN_RESTORE_DEC,
    ID_BTN_RESET,
    ID_BTN_IMG_STR_GEN,
    ID_BTN_IMG_STR_TRANS_ENC,
    ID_BTN_IMG_STR_RESTORE_DEC,
    ID_BTN_ENC_DATA_SAVE,
    ID_BTN_ENC_DATA_LOAD
}Ctrl_IDS;

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
TCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
TCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

LPCTSTR OFNDialog(HWND hWnd, bool bSave);

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 여기에 코드를 입력합니다.

    // 전역 문자열을 초기화합니다.
    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_CHSRANDTVIEWER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CHSRANDTVIEWER));

    MSG msg;

    // 기본 메시지 루프입니다:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CHSRANDTVIEWER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_BTNFACE+1);
    wcex.lpszMenuName   = MAKEINTRESOURCE(IDC_CHSRANDTVIEWER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassEx(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

   HWND hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 애플리케이션 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static CRandomTableEncDec objRandom;

    // 화면 출력을 위한 코드 수정
    // WM_SIZE에서 클라이언트 영역을 받아와
    static SIZE szClientSize;

    // 다음 상수를 사용하여 출력 라인을 조절함
    const int NEXTLINE = 30;
    const int LINERIGHT_MIN = 30;
    // 화면 출력을 위한 코드 수정

    std::list<int> liTmp;
    int i;

    HDC hDC;
    int x, y;
    TCHAR szBuf[10] = { 0, };

    static HWND hEditMsg;
    static HWND hComboEncMode;
    static HWND hEditRepeatCnt;
    static HWND hUpDownRepeatCnt;

    TCHAR szMsg[256] = { 0, };

    tstring strRestoreMsg;

    LPCTSTR artszCmbModeEnc[] = { _T("일반"), _T("반반"), _T("홀짝"), _T("홀앞짝뒤"), _T("짝앞홀뒤") };
    LPCTSTR artszTitle[] = { _T("난수 테이블1 : "), _T("난수 테이블2 : "), _T("난수 테이블3 : "), _T("난수화1 16진 값(10진 표현) : "), _T("난수화2 16진 값(10진 표현) : "), _T("복원 메시지 : "), _T("복원된 메시지 16진 값(10진 표현) : "), _T("이미지 문자 : ") };
    LPCTSTR artszLangType[] = { _T("(Unicode)"), _T("(Ansi)") };

    tstring strMsg;
    tstring strFullPath;

    bool bRetVal;

    switch (message)
    {
    case WM_CREATE:
        InitCommonControls();

        GetWindowText(hWnd, szMsg, 256);

#ifdef _UNICODE
        _tcscat_s(szMsg, 256, artszLangType[0]);

#else
        _tcscat_s(szMsg, 256, artszLangType[1]);
#endif
        SetWindowText(hWnd, szMsg);
        szMsg[0] = NULL;

        objRandom.SetEncMode(objRandom.HalfEnc);
        objRandom.GenPassTable();

        hEditMsg = CreateWindow(_T("edit"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 10, 10, 200, 25, hWnd, (HMENU)ID_EDIT_MSG, hInst, nullptr);

        CreateWindow(_T("static"), _T("암호화 모드: "), WS_CHILD | WS_VISIBLE, 220, 14, 100, 25, hWnd, (HMENU)-1, hInst, NULL);
        hComboEncMode = CreateWindow(_T("combobox"), NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWN, 320, 10, 100, 200, hWnd, (HMENU)ID_CMB_ENC_MODE, hInst, NULL);
        for (i = 0; i < _countof(artszCmbModeEnc); i++) {
            SendMessage(hComboEncMode, CB_ADDSTRING, 0, (LPARAM)artszCmbModeEnc[i]);
        }

        SendMessage(hComboEncMode, CB_SETCURSEL, objRandom.GetEncMode(), 0);

        CreateWindow(_T("static"), _T("반복 횟수: "), WS_CHILD | WS_VISIBLE, 440, 14, 90, 25, hWnd, (HMENU)-1, hInst, NULL);
        hEditRepeatCnt = CreateWindow(_T("edit"), _T("1"), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_NUMBER, 520, 10, 40, 25, hWnd, (HMENU)ID_EDIT_ENC_REPEAT_CNT, hInst, nullptr);

        hUpDownRepeatCnt = CreateWindow(UPDOWN_CLASS, NULL, WS_CHILD | WS_VISIBLE | UDS_SETBUDDYINT | UDS_ALIGNRIGHT, 0, 0, 0, 0, hWnd, (HMENU)ID_UPDOWN_ENC_REPEAT_CNT, hInst, nullptr);

        SendMessage(hUpDownRepeatCnt, UDM_SETBUDDY, (WPARAM)hEditRepeatCnt, 0);
        SendMessage(hUpDownRepeatCnt, UDM_SETRANGE, 0, MAKELPARAM(1, 8));

        if (objRandom.GetEncMode() >= objRandom.OddFirstHalf) {
            SetDlgItemInt(hWnd, ID_EDIT_ENC_REPEAT_CNT, objRandom.GetEncRepeatCnt(), true);
            EnableWindow(hEditRepeatCnt, true);
        }
        else {
            SetWindowText(hEditRepeatCnt, _T("1"));
            EnableWindow(hEditRepeatCnt, false);
        }

        CreateWindow(_T("button"), _T("암호화 변환"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 570, 10, 120, 25, hWnd, (HMENU)ID_BTN_TRANS_ENC, hInst, nullptr);
        CreateWindow(_T("button"), _T("복호화 실행"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 700, 10, 120, 25, hWnd, (HMENU)ID_BTN_RESTORE_DEC, hInst, nullptr);
        CreateWindow(_T("button"), _T("암호화 테이블 재 설정"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 840, 10, 200, 25, hWnd, (HMENU)ID_BTN_RESET, hInst, nullptr);
        CreateWindow(_T("button"), _T("이미지 문자열 생성"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 500, 45, 160, 25, hWnd, (HMENU)ID_BTN_IMG_STR_GEN, hInst, nullptr);
        CreateWindow(_T("button"), _T("이미지 암호화 변환"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 670, 45, 160, 25, hWnd, (HMENU)ID_BTN_IMG_STR_TRANS_ENC, hInst, nullptr);
        CreateWindow(_T("button"), _T("이미지 복호화 실행"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 840, 45, 160, 25, hWnd, (HMENU)ID_BTN_IMG_STR_RESTORE_DEC, hInst, nullptr);
        CreateWindow(_T("button"), _T("암호화된 데이터 파일 저장"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 570, 75, 220, 25, hWnd, (HMENU)ID_BTN_ENC_DATA_SAVE, hInst, nullptr);
        CreateWindow(_T("button"), _T("암호화된 데이터 파일 읽기"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 570, 105, 220, 25, hWnd, (HMENU)ID_BTN_ENC_DATA_LOAD, hInst, nullptr);
        break;
    case WM_SIZE:
        szClientSize.cx = LOWORD(lParam);
        szClientSize.cy = HIWORD(lParam);
        break;
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // 메뉴 선택을 구문 분석합니다:
        switch (wmId)
        {
        case ID_CMB_ENC_MODE:
            // 2024-0515_1055 최효석 추가
            // 콤보 박스 선택 후 난수 테이블 값을 재설정 함
            if (HIWORD(wParam) == CBN_SELCHANGE) {
                i = SendMessage(hComboEncMode, CB_GETCURSEL, 0, 0);
                objRandom.SetEncMode(CRandomTableEncDec::RandTableEncMode(i));

                if (objRandom.GetEncMode() >= objRandom.OddFirstHalf) {
                    SetDlgItemInt(hWnd, ID_EDIT_ENC_REPEAT_CNT, objRandom.GetEncRepeatCnt(), true);
                    EnableWindow(hEditRepeatCnt, true);
                }
                else {
                    SetWindowText(hEditRepeatCnt, _T("1"));
                    EnableWindow(hEditRepeatCnt, false);
                }

                InvalidateRect(hWnd, nullptr, true);
            }
            break;
        case ID_BTN_TRANS_ENC:
            GetWindowText(hEditMsg, szMsg, 256);

            objRandom.SetEncRepeatCnt(GetDlgItemInt(hWnd, ID_EDIT_ENC_REPEAT_CNT, nullptr, true));
#ifdef _UNICODE
            objRandom.TransWideMsgEnc(szMsg);
#else
            objRandom.TransAnsiMsgEnc(szMsg);
#endif
            InvalidateRect(hWnd, nullptr, true);
            break;
        case ID_BTN_RESTORE_DEC:
            objRandom.RestoreMsgDec();
            InvalidateRect(hWnd, nullptr, true);
            break;
        case ID_BTN_RESET:
            objRandom.GenPassTable();
            objRandom.SetTransNum();      // 인자값이 없으면 0이 넣어지고 랜덤으로 변형값이 설정된다.
            objRandom.ReleaseImgHandle();
            InvalidateRect(hWnd, nullptr, true);
            break;
        case ID_BTN_IMG_STR_GEN:
            hDC = GetDC(hWnd);
            GetWindowText(hEditMsg, szMsg, 256);

            objRandom.TextToImg(hDC, szMsg, 200, 16, _T("굴림"));
            
            ReleaseDC(hWnd, hDC);

            InvalidateRect(hWnd, nullptr, true);
            break;
        case ID_BTN_IMG_STR_TRANS_ENC:
            hDC = GetDC(hWnd);

            objRandom.SetEncRepeatCnt(GetDlgItemInt(hWnd, ID_EDIT_ENC_REPEAT_CNT, nullptr, true));
            objRandom.TransImgEnc(hDC);

            ReleaseDC(hWnd, hDC);
            InvalidateRect(hWnd, nullptr, true);
            break;
        case ID_BTN_IMG_STR_RESTORE_DEC:
            hDC = GetDC(hWnd);

            objRandom.GetRestoreImg(hDC);

            ReleaseDC(hWnd, hDC);
            InvalidateRect(hWnd, nullptr, true);
            break;
        case ID_BTN_ENC_DATA_SAVE:
            strFullPath = OFNDialog(hWnd, true);
            if (strFullPath[0] != 0 && objRandom.FileSaveEncData(strFullPath)) {
                strMsg = _T("파일 생성에 성공했습니다.");
            }
            else {
                strMsg = _T("파일 생성에 실패했습니다.");
            }
            MessageBox(hWnd, strMsg.c_str(), szTitle, MB_OK);

            break;
        case ID_BTN_ENC_DATA_LOAD:
            strFullPath = OFNDialog(hWnd, false);
            if (strFullPath[0] != 0 && objRandom.FileLoadEncData(hWnd, strFullPath)) {
                
                SendMessage(hComboEncMode, CB_SETCURSEL, objRandom.GetEncMode(), 0);

                if (objRandom.GetEncMode() >= objRandom.OddFirstHalf) {
                    SetDlgItemInt(hWnd, ID_EDIT_ENC_REPEAT_CNT, objRandom.GetEncRepeatCnt(), true);
                    EnableWindow(hEditRepeatCnt, true);
                }
                else {
                    SetWindowText(hEditRepeatCnt, _T("1"));
                    EnableWindow(hEditRepeatCnt, false);
                }
                strMsg = _T("파일 읽기에 성공했습니다.");
            }
            else {
                strMsg = _T("파일 읽기에 실패했습니다.");
            }
            InvalidateRect(hWnd, nullptr, true);
            MessageBox(hWnd, strMsg.c_str(), szTitle, MB_OK);
            
            break;
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
            // 난수 테이블 설정 대화상자 모달로 호출 및 설정값의 화면 갱신 코드
        case IDM_CFG_RANDTABLE:
            if (objRandom.ShowDlgRTConfig(hWnd) == IDOK) {
                InvalidateRect(hWnd, nullptr, true);
            }
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;

    // 클라이언트의 크기에 맞추어 화면을 출력하게 만들었음.
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        hDC = BeginPaint(hWnd, &ps);
        // TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다...

        SetBkMode(hDC, TRANSPARENT);

        x = 10, y = 50;

        if (objRandom.GetImgStrHeight() > 0) {
            
            TextOut(hDC, x, y, artszTitle[7], _tcslen(artszTitle[7]));

            x += 110;

            objRandom.DrawImgMsg(hDC, x, y);

            x = 10;
            y += objRandom.GetImgStrHeight() + 10;
        }

        liTmp = objRandom.GetPassTable(1);

        TextOut(hDC, x, y, artszTitle[0], _tcslen(artszTitle[0]));

        x = 120;
        for (auto p : liTmp) {
            wsprintf(szBuf, _T("%02d"), p);
            TextOut(hDC, x, y, szBuf, _tcslen(szBuf));
            x += 20;
        }

        wsprintf(szBuf, _T("(%02d)"), objRandom.GetTransNum());;
        TextOut(hDC, x, y, szBuf, _tcslen(szBuf));

        liTmp = objRandom.GetPassTable(2);

        x = 10, y += NEXTLINE;
        TextOut(hDC, x, y, artszTitle[1], _tcslen(artszTitle[1]));

        x = 120;
        for (auto p : liTmp) {
            wsprintf(szBuf, _T("%02d"), p);
            TextOut(hDC, x, y, szBuf, _tcslen(szBuf));
            x += 20;
        }

        liTmp = objRandom.GetPassTable(3);

        x = 10, y += NEXTLINE;
        TextOut(hDC, x, y, artszTitle[2], _tcslen(artszTitle[2]));

        x = 120;
        for (auto p : liTmp) {
            wsprintf(szBuf, _T("%02d"), p);
            TextOut(hDC, x, y, szBuf, _tcslen(szBuf));
            x += 20;
        }

        x = 10, y += NEXTLINE;
        TextOut(hDC, x, y, artszTitle[3], _tcslen(artszTitle[3]));

        x = 220;
        liTmp = objRandom.GetTransEnc(1);
        for (auto p : liTmp) {
            wsprintf(szBuf, _T("%02d"), p);
            TextOut(hDC, x, y, szBuf, _tcslen(szBuf));
            x += 20;

            if (x > szClientSize.cx - LINERIGHT_MIN) {
                y += NEXTLINE;
                x = 220;
            }
        }

        x = 10, y += NEXTLINE;
        TextOut(hDC, x, y, artszTitle[4], _tcslen(artszTitle[4]));

        x = 220;
        liTmp = objRandom.GetTransEnc(2);
        for (auto p : liTmp) {
            wsprintf(szBuf, _T("%02d"), p);
            TextOut(hDC, x, y, szBuf, _tcslen(szBuf));
            x += 20;

            if (x > szClientSize.cx - LINERIGHT_MIN) {
                y += NEXTLINE;
                x = 220;
            }
        }

        x = 10, y += NEXTLINE;
        TextOut(hDC, x, y, artszTitle[5], _tcslen(artszTitle[5]));
        x = 120;

        strRestoreMsg = objRandom.GetRestoreMsg();

        TextOut(hDC, x, y, strRestoreMsg.c_str(), strRestoreMsg.length());

        x = 10, y += NEXTLINE;
        TextOut(hDC, x, y, artszTitle[6], _tcslen(artszTitle[6]));
        x = 270;
        liTmp = objRandom.GetRestoreDec();
        for (auto p : liTmp) {
            wsprintf(szBuf, _T("%02d"), p);
            TextOut(hDC, x, y, szBuf, _tcslen(szBuf));
            x += 20;

            if (x > szClientSize.cx - LINERIGHT_MIN) {
                y += NEXTLINE;
                x = 270;
            }
        }

        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// 정보 대화 상자의 메시지 처리기입니다.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

LPCTSTR OFNDialog(HWND hWnd, bool bSave) {
    OPENFILENAME OFN;
    static TCHAR lpstrFile[MAX_PATH];
    static TCHAR filter[] = _T("모든 파일\0*.*\0CHS 암호화된 데이터 파일\0*.chs\0CRT 암호화 키 파일\0*.CRT");
    lpstrFile[0] = 0;

    ZeroMemory(&OFN, sizeof(OPENFILENAME));
    OFN.lStructSize = sizeof(OPENFILENAME);
    OFN.hwndOwner = hWnd;
    OFN.lpstrFilter = filter;
    OFN.lpstrFile = lpstrFile;
    OFN.nMaxFile = MAX_PATH;
    OFN.lpstrInitialDir = _T(".");

    if (bSave) {
        GetSaveFileName(&OFN);
    }
    else {
        GetOpenFileName(&OFN);
    }
    

    return lpstrFile;
}
