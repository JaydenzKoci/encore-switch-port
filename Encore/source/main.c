#include <raylib.h>
#include <switch.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"

// Constants
#define MAX_PATH 1024
#define MAX_NAME 256
#define MAX_DISPLAY_TEXT 2048

// Song information structure
typedef struct {
    char title[512];
    char midiFile[512];
    char folder[256];
    char displayText[MAX_DISPLAY_TEXT];
    int isValid;
} SongInfo;

// Static filesystem variables
static FsFileSystem *fs = NULL;
static FsDir dir;
static int dirOpened = 0;

// Check if file exists
static int fileExists(const char *path) {
    if (!fs) {
        printf("fileExists: Filesystem not initialized\n");
        return 0;
    }
    FsFile file;
    Result rc = fsFsOpenFile(fs, path, FsOpenMode_Read, &file);
    if (R_SUCCEEDED(rc)) {
        fsFileClose(&file);
        printf("fileExists: Found %s\n", path);
        return 1;
    }
    printf("fileExists: %s not found, error: 0x%08x\n", path, rc);
    return 0;
}

// Read file into buffer
static char* readFile(const char *path, size_t *size) {
    if (!fs) {
        printf("readFile: Filesystem not initialized\n");
        return NULL;
    }
    FsFile file;
    Result rc = fsFsOpenFile(fs, path, FsOpenMode_Read, &file);
    if (R_FAILED(rc)) {
        printf("readFile: Failed to open %s: 0x%08x\n", path, rc);
        return NULL;
    }
    s64 fileSize;
    rc = fsFileGetSize(&file, &fileSize);
    if (R_FAILED(rc)) {
        printf("readFile: Failed to get size of %s: 0x%08x\n", path, rc);
        fsFileClose(&file);
        return NULL;
    }
    char *buffer = (char *)malloc(fileSize + 1);
    if (!buffer) {
        printf("readFile: Failed to allocate buffer for %s\n", path);
        fsFileClose(&file);
        return NULL;
    }
    size_t bytesRead;
    rc = fsFileRead(&file, 0, buffer, fileSize, FsReadOption_None, &bytesRead);
    fsFileClose(&file);
    if (R_FAILED(rc) || bytesRead != (size_t)fileSize) {
        printf("readFile: Failed to read %s: 0x%08x, read %zu/%ld bytes\n", path, rc, bytesRead, fileSize);
        free(buffer);
        return NULL;
    }
    buffer[fileSize] = '\0';
    *size = fileSize;
    printf("readFile: Read %s (%zu bytes)\n", path, *size);
    return buffer;
}

// Initialize filesystem and directory
static int initSongDetector(void) {
    consoleInit(NULL);
    printf("initSongDetector: Starting\n");

    Result rc = fsInitialize();
    if (R_FAILED(rc)) {
        printf("initSongDetector: Failed to initialize filesystem: 0x%08x\n", rc);
        consoleUpdate(NULL);
        return 0;
    }
    printf("initSongDetector: Filesystem initialized\n");

    fs = fsdevGetDeviceFileSystem("sdmc");
    if (!fs) {
        printf("initSongDetector: Failed to get SDMC filesystem\n");
        fsExit();
        consoleUpdate(NULL);
        return 0;
    }
    printf("initSongDetector: SDMC filesystem acquired\n");

    rc = fsFsOpenDirectory(fs, "/switch/encore/Songs", FsDirOpenMode_ReadDirs, &dir);
    if (R_FAILED(rc)) {
        printf("initSongDetector: Failed to open /switch/encore/Songs: 0x%08x\n", rc);
        fsExit();
        consoleUpdate(NULL);
        return 0;
    }
    dirOpened = 1;
    printf("initSongDetector: /switch/encore/Songs opened\n");
    consoleUpdate(NULL);
    return 1;
}

// Get next valid song
static int getNextSongInfo(SongInfo *info) {
    if (!dirOpened || !info) {
        printf("getNextSongInfo: Invalid state or input\n");
        consoleUpdate(NULL);
        return 0;
    }

    FsDirectoryEntry dirEntry;
    s64 entriesRead;
    char basePath[MAX_PATH] = "/switch/encore/Songs";

    while (R_SUCCEEDED(fsDirRead(&dir, &entriesRead, 1, &dirEntry)) && entriesRead > 0) {
        if (dirEntry.type != FsDirEntryType_Dir) {
            printf("getNextSongInfo: Skipping non-dir %s\n", dirEntry.name);
            continue;
        }
        if (strlen(dirEntry.name) >= MAX_NAME) {
            printf("getNextSongInfo: Directory name %s too long\n", dirEntry.name);
            continue;
        }

        char subPath[MAX_PATH];
        snprintf(subPath, sizeof(subPath), "%s/%s", basePath, dirEntry.name);
        if (strlen(subPath) > MAX_PATH - 11) {
            printf("getNextSongInfo: subPath %s too long\n", subPath);
            continue;
        }
        printf("getNextSongInfo: Checking dir %s\n", subPath);

        char infoPath[MAX_PATH];
        snprintf(infoPath, sizeof(infoPath), "%s/info.json", subPath);
        if (!fileExists(infoPath)) {
            printf("getNextSongInfo: info.json not found in %s\n", subPath);
            continue;
        }

        size_t size;
        char *jsonData = readFile(infoPath, &size);
        if (!jsonData) {
            printf("getNextSongInfo: Failed to read %s\n", infoPath);
            continue;
        }

        cJSON *json = cJSON_Parse(jsonData);
        free(jsonData);
        if (!json) {
            printf("getNextSongInfo: Failed to parse JSON in %s\n", infoPath);
            continue;
        }

        cJSON *title = cJSON_GetObjectItem(json, "title");
        cJSON *midi = cJSON_GetObjectItem(json, "midi");

        if (midi && cJSON_IsString(midi) && midi->valuestring && midi->valuestring[0]) {
            if (strlen(midi->valuestring) >= MAX_NAME) {
                printf("getNextSongInfo: MIDI name %s too long\n", midi->valuestring);
                cJSON_Delete(json);
                continue;
            }

            char midiPath[MAX_PATH];
            if (strlen(subPath) + strlen(midi->valuestring) + 1 >= MAX_PATH) {
                printf("getNextSongInfo: MIDI path for %s/%s too long\n", subPath, midi->valuestring);
                cJSON_Delete(json);
                continue;
            }
            snprintf(midiPath, sizeof(midiPath), "%s/%s", subPath, midi->valuestring);
            if (fileExists(midiPath)) {
                info->isValid = 1;
                const char *titleStr = (title && cJSON_IsString(title) && title->valuestring && title->valuestring[0]) ? title->valuestring : "Unknown";
                if (strlen(titleStr) >= sizeof(info->title)) {
                    printf("getNextSongInfo: Title %s too long, truncating\n", titleStr);
                    char truncatedTitle[sizeof(info->title)];
                    strncpy(truncatedTitle, titleStr, sizeof(info->title) - 1);
                    truncatedTitle[sizeof(info->title) - 1] = '\0';
                    snprintf(info->title, sizeof(info->title), "%s", truncatedTitle);
                } else {
                    snprintf(info->title, sizeof(info->title), "%s", titleStr);
                }
                if (strlen(midi->valuestring) >= sizeof(info->midiFile)) {
                    printf("getNextSongInfo: MIDI file %s too long, truncating\n", midi->valuestring);
                    char truncatedMidi[sizeof(info->midiFile)];
                    strncpy(truncatedMidi, midi->valuestring, sizeof(info->midiFile) - 1);
                    truncatedMidi[sizeof(info->midiFile) - 1] = '\0';
                    snprintf(info->midiFile, sizeof(info->midiFile), "%s", truncatedMidi);
                } else {
                    snprintf(info->midiFile, sizeof(info->midiFile), "%s", midi->valuestring);
                }
                if (strlen(dirEntry.name) >= sizeof(info->folder)) {
                    printf("getNextSongInfo: Folder name %s too long\n", dirEntry.name);
                    cJSON_Delete(json);
                    continue;
                }
                snprintf(info->folder, sizeof(info->folder), "%s", dirEntry.name);
                int written = snprintf(info->displayText, sizeof(info->displayText),
                                      "Song: %s\nMIDI: %s\nFolder: %s",
                                      info->title, info->midiFile, info->folder);
                if (written >= (int)sizeof(info->displayText)) {
                    printf("getNextSongInfo: displayText truncated\n");
                    info->displayText[sizeof(info->displayText) - 1] = '\0';
                }
                printf("getNextSongInfo: Found valid song: %s\n", info->title);
                cJSON_Delete(json);
                consoleUpdate(NULL);
                return 1;
            } else {
                printf("getNextSongInfo: MIDI file %s not found\n", midiPath);
            }
        } else {
            printf("getNextSongInfo: Invalid or missing MIDI field in %s\n", infoPath);
        }
        cJSON_Delete(json);
    }

    info->isValid = 0;
    snprintf(info->displayText, sizeof(info->displayText), "No songs found");
    printf("getNextSongInfo: No valid songs found\n");
    consoleUpdate(NULL);
    return 0;
}

// Cleanup resources
static void cleanupSongDetector(void) {
    printf("cleanupSongDetector: Starting\n");
    if (dirOpened) {
        fsDirClose(&dir);
        dirOpened = 0;
        printf("cleanupSongDetector: Directory closed\n");
    }
    if (fs) {
        fsExit();
        fs = NULL;
        printf("cleanupSongDetector: Filesystem cleaned\n");
    }
    consoleExit(NULL);
    printf("cleanupSongDetector: Console exited\n");
}

int main(void) {
    consoleInit(NULL);
    printf("main.c: Starting\n");

    bool songDetectorInitialized = false;
    printf("main.c: Calling initSongDetector\n");
    if (!initSongDetector()) {
        printf("main.c: initSongDetector failed\n");
        consoleUpdate(NULL);
    } else {
        songDetectorInitialized = true;
        printf("main.c: initSongDetector succeeded\n");
    }

    printf("main.c: Calling InitWindow\n");
    InitWindow(1280, 720, "raylib-nx Encore");
    if (!IsWindowReady()) {
        printf("main.c: InitWindow failed\n");
        consoleUpdate(NULL);
        if (songDetectorInitialized) cleanupSongDetector();
        svcSleepThread(5e+9);
        consoleExit(NULL);
        return 1;
    }
    printf("main.c: InitWindow succeeded\n");
    SetTargetFPS(60);
    consoleUpdate(NULL);

    SongInfo song = {0};
    const char *errorMessage = songDetectorInitialized ? NULL : "Failed to initialize song detector. Check SD card.";
    if (songDetectorInitialized) {
        printf("main.c: Calling getNextSongInfo\n");
        getNextSongInfo(&song);
        printf("main.c: getNextSongInfo returned, isValid: %d\n", song.isValid);
        consoleUpdate(NULL);
    }

    while (!WindowShouldClose()) {
        if (songDetectorInitialized && (IsKeyPressed(KEY_SPACE) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE))) {
            printf("main.c: SPACE or A pressed\n");
            if (!getNextSongInfo(&song)) {
                printf("main.c: No more songs, resetting\n");
                cleanupSongDetector();
                if (!initSongDetector()) {
                    printf("main.c: Failed to reinitialize song detector\n");
                    errorMessage = "Failed to reinitialize song detector.";
                    songDetectorInitialized = false;
                } else {
                    getNextSongInfo(&song);
                }
            }
            printf("main.c: getNextSongInfo returned, isValid: %d\n", song.isValid);
            consoleUpdate(NULL);
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);
        if (errorMessage) {
            DrawText(errorMessage, 50, 50, 20, RED);
            DrawText("Press A to exit", 50, 80, 20, RED);
        } else {
            DrawText(song.displayText, 100, 100, 20, BLACK);
            DrawText("Press SPACE or A to cycle songs", 100, 50, 20, BLACK);
        }
        EndDrawing();
    }

    printf("main.c: Cleaning up\n");
    if (songDetectorInitialized) cleanupSongDetector();
    CloseWindow();
    consoleExit(NULL);
    return 0;
}