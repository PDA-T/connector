#include <iostream>
#include <thread>
#include <windows.h>

bool running = false;
bool rightClick = false; // 添加一个标志位来控制是否为右键点击，true为右键
int interval = 10; // 默认点击间隔为100毫秒
HHOOK keyboardHook;

void clicker() {
    while (true) {
        if (running) {
            INPUT input = {0};
            input.type = INPUT_MOUSE;
            if (rightClick) {
                // 模拟右键点击
                input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
                SendInput(1, &input, sizeof(INPUT));
                input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
                SendInput(1, &input, sizeof(INPUT));
            } else {
                // 模拟左键点击
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

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)) {
        KBDLLHOOKSTRUCT *p = (KBDLLHOOKSTRUCT *) lParam;
        if (p->vkCode == VK_NUMPAD9) {
            running = !running;
            std::cout << (running ? "连点器已开启。" : "连点器已关闭。") << std::endl;
        } else if (p->vkCode == VK_ADD) {
            interval = std::max(10, interval - 10);
            std::cout << "当前点击间隔: " << interval << " 毫秒" << std::endl;
        } else if (p->vkCode == VK_SUBTRACT) {
            interval += 10;
            std::cout << "当前点击间隔: " << interval << " 毫秒" << std::endl;
        } else if (p->vkCode == VK_NUMPAD0) { // 添加切换右键点击的按键
            rightClick = !rightClick;
            std::cout << (rightClick ? "右键点击模式。" : "左键点击模式。") << std::endl;
        }
    }
    return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
}

int main() {
    std::thread t(clicker);

    std::cout << "按 '+' 增加点击速度，按 '-' 减慢点击速度。" << std::endl;
    std::cout << "按 '9' 键盘开启/关闭连点器。" << std::endl;
    std::cout << "按 '0' 键盘切换左键/右键点击模式。" << std::endl;

    HINSTANCE hInstance = GetModuleHandle(NULL);
    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, hInstance, 0);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(keyboardHook);
    t.join();
    return 0;
}