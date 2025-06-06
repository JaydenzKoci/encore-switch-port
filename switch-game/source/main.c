#include <switch.h>
#include "raylib.h"

int main(void) {
    socketInitializeDefault();
    hidInitialize();
    hidInitializeKeyboard();

    // Initialize pad
    PadState pad;
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    padInitializeDefault(&pad);

    InitWindow(1280, 720, "Encore");
    SetTargetFPS(60);

    while (!WindowShouldClose() && appletMainLoop()) {
        // Update pad state
        padUpdate(&pad);

        // Get controller input
        u64 kDown = padGetButtonsDown(&pad);

        // Get keyboard input
        HidKeyboardState kbState = {0};
        size_t kbCount = hidGetKeyboardStates(&kbState, 1);

        BeginDrawing();
        ClearBackground(RAYWHITE);
        if (kDown & HidNpadButton_A) {
            DrawText("Controller: A Pressed!", 190, 200, 20, BLACK);
        } else if (kbCount > 0 && hidKeyboardStateGetKey(&kbState, HidKeyboardKey_A)) {
            DrawText("Keyboard: A Key Pressed!", 190, 200, 20, BLACK);
        } else {
            DrawText("Encore For Nintendo Switch", 190, 200, 20, BLACK);
        }
        EndDrawing();
    }

    CloseWindow();
    socketExit();
    hidExit();
    return 0;
}