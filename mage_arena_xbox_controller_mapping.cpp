#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Xinput.h>
#include <array>
#include <chrono>
#include <thread>
#include <cmath>
#include <timeapi.h>
#include <stop_token>

#pragma comment(lib, "Xinput.lib")
#pragma comment(lib, "Winmm.lib")   // for timeBeginPeriod


// hex values for corresponding ascii keys
#define VK_B 0x42
#define VK_E 0x45
#define VK_G 0x47
#define VK_Q 0x51

#define VK_1 0x31
#define VK_2 0x32
#define VK_3 0x33
#define VK_4 0x34


// ---------------------------------------------------------------------
// Mapping tables & tuning ─ tweak these to taste
// ---------------------------------------------------------------------
constexpr WORD DEADZONE                       = 9600;    // 8689 per XInput docs
constexpr float MOUSE_SENSITIVITY             = 0.0017f;  // pixels per raw‑unit
constexpr int   LEFT_STICK_THRESH             = 15000;   // stick→key threshold (>deadzone)
constexpr int TRIGGER_THRESH                  = 30;      // Trigger -> lmb click deadzone

struct ButtonRule {
    WORD xinputMask;  // which controller button
    WORD vk;          // virtual‑key (ignored for mouse buttons)
    DWORD mouseFlag;  // e.g. MOUSEEVENTF_LEFTDOWN (0 if keyboard)
    int wheelDelta;

};

// Buttons → key / mouse
constexpr std::array BUTTON_MAP {
    // A, B, X, Y - > Jump, Drop, , Interact
    ButtonRule{ XINPUT_GAMEPAD_A,              VK_SPACE,   0,                0 }, // jump
    ButtonRule{ XINPUT_GAMEPAD_B,              VK_G,       0,                0 }, // drop, g
    ButtonRule{ XINPUT_GAMEPAD_Y,              VK_B,       0,                0 }, // map, q
    ButtonRule{ XINPUT_GAMEPAD_X,              VK_E,       0,                0 }, // interact, e

    // Left/right stick buttons -> Sprint and Crouch
    ButtonRule{ XINPUT_GAMEPAD_LEFT_THUMB,              VK_SHIFT,   0,                0 }, // sprint, shift
    ButtonRule{ XINPUT_GAMEPAD_RIGHT_THUMB,              VK_CONTROL, 0,                0 }, // crouch, ctrl

    // D‑Pad - > spell slots
    ButtonRule{ XINPUT_GAMEPAD_DPAD_UP,        VK_1,       0,                 0 }, // 1
    ButtonRule{ XINPUT_GAMEPAD_DPAD_RIGHT,     VK_2,       0,                 0 }, // 2
    ButtonRule{ XINPUT_GAMEPAD_DPAD_DOWN,      VK_3,       0,                 0 }, // 3
    ButtonRule{ XINPUT_GAMEPAD_DPAD_LEFT,      VK_4,       0,                 0 }, // 4

    // Start button -> escape
    ButtonRule{ XINPUT_GAMEPAD_START,          VK_ESCAPE,  0,                 0 }, // esc
    ButtonRule{ XINPUT_GAMEPAD_BACK,           VK_Q,       0,                 0 }, // esc

    // Shoulders → change active use (mouse scroll up and down)
    ButtonRule{ XINPUT_GAMEPAD_LEFT_SHOULDER,  0,          0,                 +WHEEL_DELTA },
    ButtonRule{ XINPUT_GAMEPAD_RIGHT_SHOULDER, 0,          0,                 -WHEEL_DELTA  },

};

// Right‑stick → WASD
struct AxisRule {
    enum class Dir { Up, Down, Left, Right } dir;
    WORD vk;
};
constexpr std::array AXIS_MAP {
    AxisRule{ AxisRule::Dir::Up,    'W' },
    AxisRule{ AxisRule::Dir::Down,  'S' },
    AxisRule{ AxisRule::Dir::Left,  'A' },
    AxisRule{ AxisRule::Dir::Right, 'D' },
};

// ---------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------
INPUT makeKey(WORD vk, bool down) noexcept {
    INPUT in{};
    in.type = INPUT_KEYBOARD;
    in.ki.wVk = vk;
    in.ki.dwFlags = down ? 0 : KEYEVENTF_KEYUP;
    return in;
}

INPUT makeMouse(DWORD flag) noexcept {
    INPUT in{};
    in.type = INPUT_MOUSE;
    in.mi.dwFlags = flag;
    return in;
}

void send(const std::initializer_list<INPUT>& list) {
    if (!list.size()) return;
    SendInput(static_cast<UINT>(list.size()), const_cast<INPUT*>(list.begin()), sizeof(INPUT));
}

// ---------------------------------------------------------------------
void pollLoop(std::stop_token st) {
    // Boost priority inside the thread (after creation)
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

    XINPUT_STATE prev{};
    XINPUT_STATE cur{};

    // Track axis‑to‑key state
    bool axisState[256]{};  // indexed by VK

    bool triggerDepressed = false;

    while (!st.stop_requested()) {
        if (XInputGetState(0, &cur) == ERROR_SUCCESS) {
            const auto& gp  = cur.Gamepad;
            const auto& ppg = prev.Gamepad;

            // --- Digital buttons
            for (const auto& rule : BUTTON_MAP) {
                bool now   = gp.wButtons  & rule.xinputMask;
                bool prior = ppg.wButtons & rule.xinputMask;
                //if (now == prior) continue;

                // Mouse‑wheel: fire only on button **press** (rising edge)
                if (rule.wheelDelta && now) {
                    INPUT wheel{};
                    wheel.type = INPUT_MOUSE;
                    wheel.mi.dwFlags    = MOUSEEVENTF_WHEEL;
                    wheel.mi.mouseData  = rule.wheelDelta;
                    send({ wheel });
                    continue;
                }

                if (rule.mouseFlag) {
                    DWORD flag = rule.mouseFlag;
                    if (!now) flag <<= 1;  // *_DOWN is even, *_UP is +0x2
                    send({ makeMouse(flag) });
                } else {
                    send({ makeKey(rule.vk, now) });
                }
            }

            // Right Trigger -> Left Mouse Click
            const auto rt = gp.bRightTrigger;
            if(rt > TRIGGER_THRESH && !triggerDepressed){
                send({ makeMouse(MOUSEEVENTF_LEFTDOWN) });
                triggerDepressed = true;
            } else if (rt < TRIGGER_THRESH && triggerDepressed){
                send({ makeMouse(MOUSEEVENTF_LEFTUP) });
                triggerDepressed = false;
            }

            // TODO: Review this section more carefully
            // Left stick - > WASD
            const auto lx = gp.sThumbLX;
            const auto ly = gp.sThumbLY;
            for (const auto& ar : AXIS_MAP) {
                bool active = false;
                switch (ar.dir) {
                    // 15000 is a threshold value for the stick. Our deadzone is 7849, so this is well outside it. 
                    // Making this farther than the deadzone reduces jittery input. Q: what is the max threshold the stick will return?
                    case AxisRule::Dir::Up   : active =  ly >  LEFT_STICK_THRESH; break;
                    case AxisRule::Dir::Down : active =  ly < -LEFT_STICK_THRESH; break;
                    case AxisRule::Dir::Right: active =  lx >  LEFT_STICK_THRESH; break;
                    case AxisRule::Dir::Left : active =  lx < -LEFT_STICK_THRESH; break;
                }
                if (active != axisState[ar.vk]) {
                    send({ makeKey(ar.vk, active) });
                    axisState[ar.vk] = active;
                }
            }


            // --- Right stick → mouse
            if (std::abs(gp.sThumbRX) > DEADZONE || std::abs(gp.sThumbRY) > DEADZONE) {
                LONG dx = static_cast<LONG>(gp.sThumbRX * MOUSE_SENSITIVITY);
                LONG dy = static_cast<LONG>(-gp.sThumbRY * MOUSE_SENSITIVITY);

                INPUT move{};
                move.type = INPUT_MOUSE;
                move.mi.dx = dx;
                move.mi.dy = dy;
                move.mi.dwFlags = MOUSEEVENTF_MOVE;
                send({ move });
            }

            prev = cur;
        }
        // 1 ms cadence (~1000 Hz)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    timeBeginPeriod(1);  // 1 ms timer resolution for Sleep()

    // Spawn high‑priority polling thread
    std::jthread pollThread(pollLoop);

    MessageBoxA(nullptr,
        "Xbox to KB/M mapper running.\n"
        "Press OK or close window to quit.",
        "Xbox Mapper", MB_OK);

    // jthread joins on destruction
    return 0;
}
