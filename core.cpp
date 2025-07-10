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

    INPUT inputs[2];
    ZeroMemory(inputs, sizeof(inputs));

    // 初始化
    bool useLeftButton = GetUseLeftButton();
    int cps = GetCPS();
    double interval_sec = 1.0 / (double) cps;
    DWORD downFlag = useLeftButton ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_RIGHTDOWN;
    DWORD upFlag = useLeftButton ? MOUSEEVENTF_LEFTUP : MOUSEEVENTF_RIGHTUP;
    DWORD lastUpdate = GetTickCount();

    while (true) {
        DWORD nowTick = GetTickCount();
        if (nowTick - lastUpdate > 100) {
            useLeftButton = GetUseLeftButton();
            cps = GetCPS();
            interval_sec = 1.0 / (double) cps;
            downFlag = useLeftButton ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_RIGHTDOWN;
            upFlag = useLeftButton ? MOUSEEVENTF_LEFTUP : MOUSEEVENTF_RIGHTUP;
            lastUpdate = nowTick;
        }

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

            inputs[0].type = INPUT_MOUSE;
            inputs[0].mi.dwFlags = downFlag;
            inputs[1].type = INPUT_MOUSE;
            inputs[1].mi.dwFlags = upFlag;

            SendInput(2, inputs, sizeof(INPUT));

            do {
                QueryPerformanceCounter(&now);
            } while ((now.QuadPart - start.QuadPart) < (LONGLONG) (freq.QuadPart * interval_sec));
        } else {
            Sleep(1);
        }
    }
    return 0;
}
