#include <iostream>
#include <thread>
#include <windows.h>

bool running = false;
bool rightClick = false; // ����һ����־λ�������Ƿ�Ϊ�Ҽ������trueΪ�Ҽ�
int interval = 10; // Ĭ�ϵ�����Ϊ100����
HHOOK keyboardHook;

void clicker() {
    while (true) {
        if (running) {
            INPUT input = {0};
            input.type = INPUT_MOUSE;
            if (rightClick) {
                // ģ���Ҽ����
                input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
                SendInput(1, &input, sizeof(INPUT));
                input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
                SendInput(1, &input, sizeof(INPUT));
            } else {
                // ģ��������
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
            std::cout << (running ? "�������ѿ�����" : "�������ѹرա�") << std::endl;
        } else if (p->vkCode == VK_ADD) {
            interval = std::max(10, interval - 10);
            std::cout << "��ǰ������: " << interval << " ����" << std::endl;
        } else if (p->vkCode == VK_SUBTRACT) {
            interval += 10;
            std::cout << "��ǰ������: " << interval << " ����" << std::endl;
        } else if (p->vkCode == VK_NUMPAD0) { // �����л��Ҽ�����İ���
            rightClick = !rightClick;
            std::cout << (rightClick ? "�Ҽ����ģʽ��" : "������ģʽ��") << std::endl;
        }
    }
    return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
}

int main() {
    std::thread t(clicker);

    std::cout << "�� '+' ���ӵ���ٶȣ��� '-' ��������ٶȡ�" << std::endl;
    std::cout << "�� '9' ���̿���/�ر���������" << std::endl;
    std::cout << "�� '0' �����л����/�Ҽ����ģʽ��" << std::endl;

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