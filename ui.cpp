#define UNICODE
#define _UNICODE
#include <windows.h>
#include <dwmapi.h>
#pragma comment(lib, "Dwmapi.lib")

static bool g_useLeftButton = true;
static int g_cps = 200;

#define IDC_EDIT_CPS     1001

RECT g_rcLeftBtn, g_rcRightBtn, g_rcEditCps;
bool g_hoverLeft = false, g_hoverRight = false;
POINT g_dragStart = {0};
bool g_dragging = false;

// 圆角矩形
void DrawRoundRect(HDC hdc, RECT rc, int radius, COLORREF color, COLORREF bg, int penWidth = 1) {
    HBRUSH hBrush = CreateSolidBrush(bg);
    HPEN hPen = CreatePen(PS_SOLID, penWidth, color);
    HGDIOBJ oldBrush = SelectObject(hdc, hBrush);
    HGDIOBJ oldPen = SelectObject(hdc, hPen);
    RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, radius, radius);
    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(hBrush);
    DeleteObject(hPen);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HWND hEditCps = nullptr;
    switch (msg) {
        case WM_CREATE: {
            // CPS输入框
            hEditCps = CreateWindowW(L"EDIT", L"200", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | ES_CENTER,
                                     0, 0, 1, 1, hwnd, (HMENU)IDC_EDIT_CPS, 0, 0);
            SendMessageW(hEditCps, EM_SETLIMITTEXT, 5, 0);
            break;
        }
        case WM_SIZE: {
            // 布局
            int w = LOWORD(lParam), h = HIWORD(lParam);
            int centerX = w / 2, btnW = 90, btnH = 40, spacing = 54;
            int yBtn = 100;
            g_rcLeftBtn = {centerX - btnW - spacing / 2, yBtn, centerX - spacing / 2, yBtn + btnH};
            g_rcRightBtn = {centerX + spacing / 2, yBtn, centerX + btnW + spacing / 2, yBtn + btnH};

            int cpsW = 110, cpsH = 38, yCps = yBtn + btnH + 32;
            g_rcEditCps = {centerX - cpsW / 2, yCps, centerX + cpsW / 2, yCps + cpsH};

            MoveWindow(hEditCps, g_rcEditCps.left, g_rcEditCps.top, cpsW, cpsH, TRUE);
            break;
        }
        case WM_MOUSEMOVE: {
            int x = LOWORD(lParam), y = HIWORD(lParam);
            bool hl = PtInRect(&g_rcLeftBtn, {x, y});
            bool hr = PtInRect(&g_rcRightBtn, {x, y});
            if (hl != g_hoverLeft || hr != g_hoverRight) {
                g_hoverLeft = hl;
                g_hoverRight = hr;
                InvalidateRect(hwnd, NULL, FALSE);
            }
            if (g_dragging) {
                POINT pt;
                GetCursorPos(&pt);
                RECT winRect;
                GetWindowRect(hwnd, &winRect);
                int dx = pt.x - g_dragStart.x;
                int dy = pt.y - g_dragStart.y;
                SetWindowPos(hwnd, nullptr, winRect.left + dx, winRect.top + dy, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
                g_dragStart = pt;
            }
            break;
        }
        case WM_LBUTTONDOWN: {
            int x = LOWORD(lParam), y = HIWORD(lParam);
            RECT rcTitle = {0, 0, 600, 56};
            if (PtInRect(&rcTitle, {x, y})) {
                SetCapture(hwnd);
                g_dragging = true;
                GetCursorPos(&g_dragStart);
            } else if (PtInRect(&g_rcLeftBtn, {x, y})) {
                g_useLeftButton = true;
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (PtInRect(&g_rcRightBtn, {x, y})) {
                g_useLeftButton = false;
                InvalidateRect(hwnd, NULL, FALSE);
            }
            break;
        }
        case WM_LBUTTONUP:
            if (g_dragging) {
                g_dragging = false;
                ReleaseCapture();
            }
            break;
        case WM_COMMAND:
            if (LOWORD(wParam) == IDC_EDIT_CPS && HIWORD(wParam) == EN_CHANGE) {
                wchar_t buf[16] = {0};
                GetWindowTextW((HWND) lParam, buf, 15);
                int val = _wtoi(buf);
                if (val > 0 && val < 10001)
                    g_cps = val;
            }
            break;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            HBRUSH bgBrush = CreateSolidBrush(RGB(33, 36, 41));
            FillRect(hdc, &ps.rcPaint, bgBrush);
            DeleteObject(bgBrush);

            SetBkMode(hdc, TRANSPARENT);

            // 顶部标题
            HFONT hFontTitle = CreateFontW(32, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, L"微软雅黑");
            HGDIOBJ oldFont = SelectObject(hdc, hFontTitle);
            SetTextColor(hdc, RGB(130, 180, 255));
            DrawTextW(hdc, L"自动点击器", -1, new RECT{0, 14, 600, 58}, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
            SelectObject(hdc, oldFont);
            DeleteObject(hFontTitle);

            // 鼠标按钮左右并排
            HFONT hFontBtn = CreateFontW(20, 0, 0, 0, FW_SEMIBOLD, 0, 0, 0, 0, 0, 0, 0, 0, L"微软雅黑");
            oldFont = SelectObject(hdc, hFontBtn);
            COLORREF leftCol = g_useLeftButton
                                   ? RGB(65, 180, 250)
                                   : (g_hoverLeft ? RGB(80, 110, 140) : RGB(48, 55, 70));
            COLORREF rightCol = !g_useLeftButton
                                    ? RGB(130, 100, 255)
                                    : (g_hoverRight ? RGB(80, 110, 140) : RGB(48, 55, 70));
            DrawRoundRect(hdc, g_rcLeftBtn, 20, leftCol, leftCol, 0);
            DrawRoundRect(hdc, g_rcRightBtn, 20, rightCol, rightCol, 0);

            SetTextColor(hdc, RGB(255, 255, 255));
            DrawTextW(hdc, L"左键", -1, &g_rcLeftBtn, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
            DrawTextW(hdc, L"右键", -1, &g_rcRightBtn, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
            SelectObject(hdc, oldFont);
            DeleteObject(hFontBtn);

            // CPS输入框上文字
            HFONT hFontCps = CreateFontW(18, 0, 0, 0, FW_MEDIUM, 0, 0, 0, 0, 0, 0, 0, 0, L"微软雅黑");
            oldFont = SelectObject(hdc, hFontCps);
            SetTextColor(hdc, RGB(200, 220, 255));
            RECT rcCpsText = {g_rcEditCps.left, g_rcEditCps.top - 32, g_rcEditCps.right, g_rcEditCps.top - 4};
            DrawTextW(hdc, L"CPS", -1, &rcCpsText, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
            SelectObject(hdc, oldFont);
            DeleteObject(hFontCps);

            // 底部提示
            HFONT hFontHotkey = CreateFontW(28, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, L"微软雅黑");
            oldFont = SelectObject(hdc, hFontHotkey);
            SetTextColor(hdc, RGB(255, 220, 64));
            DrawTextW(hdc, L"数字键 9 开始/停止", -1, new RECT{0, 230, 600, 280}, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
            SelectObject(hdc, oldFont);
            DeleteObject(hFontHotkey);

            EndPaint(hwnd, &ps);
            break;
        }
        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLOREDIT: {
            HDC hdc = (HDC) wParam;
            SetBkColor(hdc, RGB(33, 36, 41));
            SetTextColor(hdc, RGB(220, 220, 255));
            return (LRESULT) CreateSolidBrush(RGB(33, 36, 41));
        }
        case WM_CLOSE:
            DestroyWindow(hwnd);
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
    return 0;
}

DWORD WINAPI UIThreadProc(LPVOID) {
    WNDCLASSW wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    wc.lpszClassName = L"MyDarkAutoClicker";
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    RegisterClassW(&wc);

    int winW = 600, winH = 320;
    HWND hwnd = CreateWindowExW(WS_EX_LAYERED | WS_EX_APPWINDOW | WS_EX_COMPOSITED,
                                L"MyDarkAutoClicker", L"", WS_POPUP,
                                (GetSystemMetrics(SM_CXSCREEN) - winW) / 2, (GetSystemMetrics(SM_CYSCREEN) - winH) / 2,
                                winW, winH, 0, 0, 0, 0);

    // 圆角
    if (hwnd) {
        DWM_WINDOW_CORNER_PREFERENCE pref = DWMWCP_ROUND;
        DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &pref, sizeof(pref));
        SetLayeredWindowAttributes(hwnd, 0, 245, LWA_ALPHA);
        ShowWindow(hwnd, SW_SHOWNORMAL);
        UpdateWindow(hwnd);
    }

    MSG msg;
    while (GetMessageW(&msg, 0, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return 0;
}

void StartUI() {
    CreateThread(0, 0, UIThreadProc, 0, 0, 0);
}

bool GetUseLeftButton() { return g_useLeftButton; }
int GetCPS() { return g_cps; }
