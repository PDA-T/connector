#include <iostream>
#include <thread>
#include <windows.h>

bool running = false;
bool rightClick = false; // false为左键
int interval = 10; // 默认点击间隔
HHOOK keyboardHook;
HWND hwnd;
HWND hText;

// 点击模拟函数
void clicker() {
    while (true) {
        if (running) {
            INPUT input = {0};
            input.type = INPUT_MOUSE;
            if (rightClick) {
                input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
                SendInput(1, &input, sizeof(INPUT));
                input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
                SendInput(1, &input, sizeof(INPUT));
            } else {
                input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
                SendInput(1, &input, sizeof(INPUT));
                input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
                SendInput(1, &input, sizeof(INPUT));
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

// 更新提示消息
void updateStatus(const char *status) {
    SetWindowText(hText, status);
}

// 键盘钩子回调
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)) {
        KBDLLHOOKSTRUCT *p = (KBDLLHOOKSTRUCT *) lParam;
        if (p->vkCode == VK_NUMPAD9) {
            running = !running;
            updateStatus(running ? "连点器已开启。" : "连点器已关闭。");
        } else if (p->vkCode == VK_ADD) {
            interval = std::max(10, interval - 10);
            std::string msg = "当前点击间隔: " + std::to_string(interval) + " 毫秒";
            updateStatus(msg.c_str());
        } else if (p->vkCode == VK_SUBTRACT) {
            interval += 10;
            std::string msg = "当前点击间隔: " + std::to_string(interval) + " 毫秒";
            updateStatus(msg.c_str());
        } else if (p->vkCode == VK_NUMPAD0) {
            rightClick = !rightClick;
            updateStatus(rightClick ? "右键点击模式。" : "左键点击模式。");
        }
    }
    return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
}

// 主窗口消息处理
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

// WinMain函数创建窗口
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "Simple Window Class";

    WNDCLASS wc = {};

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    hwnd = CreateWindowEx(
            0,
            CLASS_NAME,
            "点击器",
            WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
            CW_USEDEFAULT, CW_USEDEFAULT, 300, 100,
            NULL,
            NULL,
            hInstance,
            NULL
    );

    if (hwnd == NULL) {
        return 0;
    }

    // 创建静态文本控件显示状态信息
    hText = CreateWindow(
            "STATIC", "",
            WS_VISIBLE | WS_CHILD | SS_CENTER,
            10, 10, 260, 40,
            hwnd, NULL, hInstance, NULL
    );

    ShowWindow(hwnd, nCmdShow);

    // 初始提示
    updateStatus(
            "按 '+' 增加点击速度，按 '-' 减慢点击速度。按 '9' 键盘开启/关闭连点器。按 '0' 键盘切换左键/右键点击模式。");

    // 开启线程执行点击器功能
    std::thread t(clicker);

    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, hInstance, 0);

    // 运行窗口消息循环
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(keyboardHook);
    t.join();
    return 0;
}