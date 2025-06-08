#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <switch.h>
#include <raylib.h>

#define MAX_FILES 100
#define FOLDER_PATH "sdmc:/switch/encore/Songs"

int main(int argc, char* argv[]) {
    fsdevMountSdmc();

    struct stat st;
    if (stat(FOLDER_PATH, &st) != 0) {
        mkdir("sdmc:/switch", 0777);
        mkdir("sdmc:/switch/encore", 0777);
        mkdir(FOLDER_PATH, 0777);
    }

    InitWindow(1280, 720, "Encore");
    SetTargetFPS(60);

    char* file_list[MAX_FILES];
    int file_count = 0;
    bool dir_error = false;

    DIR* dir = opendir(FOLDER_PATH);
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL && file_count < MAX_FILES) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }
            file_list[file_count] = strdup(entry->d_name);
            file_count++;
        }
        closedir(dir);
    } else {
        dir_error = true;
    }

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_PAUSE)) {
            break;
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawText("Songs in /switch/encore/Songs:", 20, 20, 20, BLACK);
        if (dir_error) {
            DrawText("Failed to open directory: " FOLDER_PATH, 20, 60, 20, RED);
        } else if (file_count == 0) {
            DrawText("No files found in Songs folder.", 20, 60, 20, RED);
        } else {
            for (int i = 0; i < file_count; i++) {
                char display_text[256];
                snprintf(display_text, sizeof(display_text), "%d: %s", i + 1, file_list[i]);
                DrawText(display_text, 20, 60 + i * 30, 20, BLACK);
            }
        }

        EndDrawing();
    }

    for (int i = 0; i < file_count; i++) {
        free(file_list[i]);
    }
    fsdevUnmountAll();
    CloseWindow();

    return 0;
}