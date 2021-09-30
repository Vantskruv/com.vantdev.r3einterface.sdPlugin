#pragma once
#define WIN32_LEAN_AND_MEAN
#include <thread>
#include <Windows.h>
#include <array>

class FKeyCommand
{
private:

    static constexpr std::array<unsigned short, 9> extendedKeys
    {   
        0x26, //KEY_UP
        0x28, //KEY_DOWN
        0x25, //KEY_LEFT
        0x27, //KEY_RIGHT
        0x24, //KEY_HOME
        0x21, //KEY_PAGE_UP
        0x22, //KEY_PAGE_DOWN
        0x2e, //KEY_DEL
        0x23  //KEY_END
    };

public:
   
    static void SendScanCodeKeyPress(unsigned short keyCode, int holdTimeMillis)
	{
        unsigned short scanCode = MapVirtualKey((unsigned short)keyCode, 0);
        bool extended = std::find(extendedKeys.begin(), extendedKeys.end(), keyCode) != extendedKeys.end();
        press(scanCode, extended);
        std::this_thread::sleep_for(std::chrono::milliseconds(holdTimeMillis));
        release(scanCode, extended);
	}

    static void press(unsigned short scanCode, bool extended)
    {
        unsigned int eventScanCode = extended ? (KEYEVENTF_SCANCODE | KEYEVENTF_EXTENDEDKEY) : (KEYEVENTF_SCANCODE);

        INPUT input[1];
        ZeroMemory(input, sizeof(input));
        input[0].type = INPUT_KEYBOARD;
        input[0].ki.wVk = 0;
        input[0].ki.wScan = scanCode;
        input[0].ki.dwFlags = eventScanCode;
        input[0].ki.dwExtraInfo = GetMessageExtraInfo();

        SendInput(ARRAYSIZE(input), input, sizeof(INPUT));
    }

    static void release(unsigned short scanCode, bool extended)
    {
        unsigned int eventScanCode = extended ? (KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP | KEYEVENTF_EXTENDEDKEY) : (KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP);

        INPUT input[1];
        ZeroMemory(input, sizeof(input));
        input[0].type = INPUT_KEYBOARD;
        input[0].ki.wVk = 0;
        input[0].ki.wScan = scanCode;
        input[0].ki.dwFlags = eventScanCode;
        input[0].ki.dwExtraInfo = GetMessageExtraInfo();

        SendInput(ARRAYSIZE(input), input, sizeof(INPUT));
    }
};

