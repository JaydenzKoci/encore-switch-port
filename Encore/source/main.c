#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <switch.h>
#include <raylib.h>
#include "song.h"

int main(int argc, char* argv[]) {

    fsdevMountSdmc();

    SetAudioStreamBufferSizeDefault(4096);
    InitAudioDevice();
    InitWindow(1280, 720, "Encore");
    SetTargetFPS(120);

    Song songs[MAX_FILES];
    int song_count = 0;
    bool dir_error = !init_songs(songs, &song_count);
    printf("Loaded %d songs\n", song_count);

    int highlighted_song = 0;
    int detailed_song = 0;
    bool playing = false;
    int playing_song_index = -1;
    bool debug_enabled = true;

    int dpad_hold_counter = 0;
    const int hold_delay = 8;
    const int hold_repeat = 2;
    const float joystick_deadzone = 0.15f;
    float last_joystick_time = 0.0f;
    const float joystick_delay = 0.1f;

    // Main loop
    while (!WindowShouldClose()) {
        float current_time = GetTime();

        float joystick_y = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y);

        if (debug_enabled && !playing) {
            printf("Song Count: %d, Highlighted: %d, Detailed: %d, FPS: %d\n",
                   song_count, highlighted_song, detailed_song, GetFPS());
            if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_RIGHT)) {
                printf("D-pad Held: (Up: %d, Down: %d, Left: %d, Right: %d)\n",
                       IsKeyDown(KEY_UP), IsKeyDown(KEY_DOWN), IsKeyDown(KEY_LEFT), IsKeyDown(KEY_RIGHT));
            }
            if (joystick_y != 0.0f) {
                printf("Joystick Y: %f (Deadzone: %f)\n", joystick_y, joystick_deadzone);
            }
            if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) printf("Button B Pressed\n");
            if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)) printf("Button A Pressed\n");
            if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_UP)) printf("Button Y Pressed\n");
            if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_LEFT)) printf("Button X Pressed\n");
            if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE_RIGHT)) printf("Button Plus Pressed\n");
        }

        if (song_count > 0) {
            int prev_highlighted = highlighted_song;
            if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_RIGHT)) {
                dpad_hold_counter++;
                if (dpad_hold_counter == 1 || (dpad_hold_counter > hold_delay && (dpad_hold_counter - hold_delay) % hold_repeat == 0)) {
                    if (highlighted_song < song_count - 1) {
                        highlighted_song++;
                        if (debug_enabled) printf("D-pad Down/Right: %d (Counter: %d)\n", highlighted_song, dpad_hold_counter);
                    }
                }
            } else if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_LEFT)) {
                dpad_hold_counter++;
                if (dpad_hold_counter == 1 || (dpad_hold_counter > hold_delay && (dpad_hold_counter - hold_delay) % hold_repeat == 0)) {
                    if (highlighted_song > 0) {
                        highlighted_song--;
                        if (debug_enabled) printf("D-pad Up/Left: %d (Counter: %d)\n", highlighted_song, dpad_hold_counter);
                    }
                }
            } else {
                dpad_hold_counter = 0;
            }

            if (current_time - last_joystick_time > joystick_delay) {
                if (joystick_y > joystick_deadzone && highlighted_song < song_count - 1) {
                    highlighted_song++;
                    last_joystick_time = current_time;
                    if (debug_enabled) printf("Joystick Down: %d (Y: %f)\n", highlighted_song, joystick_y);
                } else if (joystick_y < -joystick_deadzone && highlighted_song > 0) {
                    highlighted_song--;
                    last_joystick_time = current_time;
                    if (debug_enabled) printf("Joystick Up: %d (Y: %f)\n", highlighted_song, joystick_y);
                }
            }

            if (highlighted_song != prev_highlighted) {
                detailed_song = highlighted_song;
                if (debug_enabled) printf("Updated Detailed Song: %d\n", detailed_song);
            }

            if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE_RIGHT)) {
                if (debug_enabled) printf("PLUS pressed: Exiting\n");
                break;
            }
            if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) {
                if (detailed_song != highlighted_song) {
                    if (playing && detailed_song != -1) {
                        toggle_song_playback(&songs[detailed_song], &playing);
                    }
                    detailed_song = highlighted_song;
                    playing_song_index = -1; // Reset playing index
                    if (debug_enabled) printf("B pressed: Selected song %d\n", detailed_song);
                }
            }
            if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT) && detailed_song != -1) {
                toggle_song_playback(&songs[detailed_song], &playing);
                playing_song_index = playing ? detailed_song : -1;
                if (debug_enabled) printf("A pressed: Play/Pause (Playing: %d, Index: %d)\n", playing, playing_song_index);
            }
            if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_LEFT) && detailed_song != -1) {
                if (playing && playing_song_index != -1) {
                    toggle_song_playback(&songs[playing_song_index], &playing);
                }
                play_song_preview(&songs[detailed_song], &playing);
                playing_song_index = playing ? detailed_song : -1;
                if (debug_enabled) printf("Y pressed: Preview\n");
            }
            if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_UP)) {
                if (playing && detailed_song != -1) {
                    toggle_song_playback(&songs[detailed_song], &playing);
                    playing_song_index = -1;
                }
                detailed_song = 0;
                if (debug_enabled) printf("X pressed: Reset\n");
            }
        }

        update_songs(songs, song_count, playing_song_index);

        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawText("Songs in /switch/encore/Songs:", 10, 20, 20, BLACK);
        if (dir_error) {
            DrawText("Failed to open directory: " FOLDER_PATH, 10, 60, 20, RED);
        } else if (song_count == 0) {
            DrawText("No valid songs found (missing info.json or MIDI).", 10, 60, 60, BLACK);
        } else {
            for (int i = 0; i < song_count; i++) {
                char display[256];
                snprintf(display, sizeof(display), "%s%d: %s - %s",
                         i == highlighted_song ? "> " : "  ", i + 1, songs[i].title, songs[i].artist);
                DrawText(display, 10, 60 + i * 30, 20, i == highlighted_song ? BLUE : BLACK);
            }
            DrawText("B: Select, A: Play/Pause, Y: Preview, X: Reset, PLUS: Exit", 10, 650, 20, BLACK);
        }

        if (detailed_song >= 0 && detailed_song < song_count) {
            char details[1024] = {0};
            get_song_details(&songs[detailed_song], details, sizeof(details));
            DrawText(details, 700, 100, 20, BLACK);
        } else {
            DrawText("Navigate to a song to view details.", 700, 150, 20, GRAY);
        }

        char fps_text[16];
        snprintf(fps_text, sizeof(fps_text), "FPS: %d", GetFPS());
        DrawText(fps_text, 1180, 10, 20, GREEN);

        EndDrawing();
    }

    free_songs(songs, song_count);
    fsdevUnmountAll();
    CloseAudioDevice();
    CloseWindow();

    return 0;
}