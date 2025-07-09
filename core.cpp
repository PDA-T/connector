#include <windows.h>
#include <stdio.h>
#include "ui.h"

// 热键
#define HOTKEY 0x39

int main() {
    // 启动UI
    StartUI();

    bool toggle = false;
    bool keyDown = false;
    LARGE_INTEGER freq, start, now;
    QueryPerformanceFrequency(&freq);

    while (true) {
        // 获取UI设置
        bool useLeftButton = GetUseLeftButton();
        int cps = GetCPS();
        double interval_sec = 1.0 / (double) cps;

        DWORD downFlag = useLeftButton ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_RIGHTDOWN;
        DWORD upFlag = useLeftButton ? MOUSEEVENTF_LEFTUP : MOUSEEVENTF_RIGHTUP;

        // 热键切换
        if (GetAsyncKeyState(HOTKEY) & 0x8000) {
            if (!keyDown) {
                toggle = !toggle;
                keyDown = true;
            }
        } else {
            keyDown = false;
        }

        if (toggle) {
            QueryPerformanceCounter(&start);

            // 发送点击事件
            mouse_event(downFlag, 0, 0, 0, 0);
            mouse_event(upFlag, 0, 0, 0, 0);

            // 精确延迟
            do {
                QueryPerformanceCounter(&now);
            } while ((now.QuadPart - start.QuadPart) < (LONGLONG) (freq.QuadPart * interval_sec));
        } else {
            Sleep(1);
        }
    }
    return 0;
}
