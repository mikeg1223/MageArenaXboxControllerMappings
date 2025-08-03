#include <windows.h>
#include <thread>
#include "xbox_controller_mapping.h"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    timeBeginPeriod(1);  // 1 ms timer resolution for Sleep()

    // Spawn high‑priority polling thread
    std::jthread pollThread(xbox_mapping::pollLoop);

    MessageBoxA(nullptr,
        "Xbox to KB/M mapper running.\n"
        "Press OK or close window to quit.",
        "Xbox Mapper", MB_OK);

    // jthread joins on destruction
    return 0;
}