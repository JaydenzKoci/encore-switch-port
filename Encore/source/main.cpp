//
// Created by Jaydenz on 6/9/2025.
//

#define RAYGUI_IMPLEMENTATION
#include <switch.h>
#include <raylib.h>
#include <raygui.h>
#include <cstdio>
#include <filesystem>
#include "MenuManager.h"

// Global instances
Assets& menuAss = Assets::getInstance();
Units u = Units::getInstance();
Encore::Settings TheGameSettings;
Encore::SettingsInit TheSettingsInitializer;
MenuManager TheMenuManager;

void DrawLoadingScreen(float progress) {
    BeginDrawing();
    ClearBackground(BLACK);
    DrawText("Loading...", 1280 / 2 - MeasureText("Loading...", 40) / 2, 720 / 2 - 60, 40, WHITE);
    Rectangle progressBarBounds = { 1280 / 2 - 200, 720 / 2, 400, 30 };
    GuiProgressBar(progressBarBounds, nullptr, TextFormat("%.0f%%", progress * 100), &progress, 0.0f, 1.0f);
    EndDrawing();
}

int main(int argc, char* argv[]) {
    fsdevMountSdmc();

    InitWindow(1280, 720, "Encore");
    SetTargetFPS(TheGameSettings.Framerate);
    SetExitKey(0);

    const int totalAssets = 7;
    int loadedAssets = 0;
    float progress = 0.0f;

    std::filesystem::path fontPath = "sdmc:/switch/encore/Fonts/";
    menuAss.josefinSansItalic = menuAss.LoadFontFilter(fontPath / "josefin_sans_italic.ttf");
    loadedAssets++; progress = (float)loadedAssets / totalAssets; DrawLoadingScreen(progress);
    if (menuAss.josefinSansItalic.baseSize == 0) {
        printf("Failed to load josefin_sans_italic.ttf\n");
        CloseWindow();
        fsdevUnmountDevice("sdmc");
        return 1;
    }

    menuAss.rubik = menuAss.LoadFontFilter(fontPath / "rubik.ttf");
    loadedAssets++; progress = (float)loadedAssets / totalAssets; DrawLoadingScreen(progress);
    if (menuAss.rubik.baseSize == 0) {
        printf("Failed to load rubik.ttf\n");
        CloseWindow();
        fsdevUnmountDevice("sdmc");
        return 1;
    }

    menuAss.rubikBoldItalic = menuAss.LoadFontFilter(fontPath / "rubik_bold_italic.ttf");
    loadedAssets++; progress = (float)loadedAssets / totalAssets; DrawLoadingScreen(progress);
    if (menuAss.rubikBoldItalic.baseSize == 0) {
        printf("Failed to load rubik_bold_italic.ttf\n");
        CloseWindow();
        fsdevUnmountDevice("sdmc");
        return 1;
    }

    menuAss.rubikItalic = menuAss.LoadFontFilter(fontPath / "rubik_italic.ttf");
    loadedAssets++; progress = (float)loadedAssets / totalAssets; DrawLoadingScreen(progress);
    if (menuAss.rubikItalic.baseSize == 0) {
        printf("Failed to load rubik_italic.ttf\n");
        CloseWindow();
        fsdevUnmountDevice("sdmc");
        return 1;
    }

    menuAss.redHatDisplayBlack = menuAss.LoadFontFilter(fontPath / "red_hat_display_black.ttf");
    loadedAssets++; progress = (float)loadedAssets / totalAssets; DrawLoadingScreen(progress);
    if (menuAss.redHatDisplayBlack.baseSize == 0) {
        printf("Failed to load red_hat_display_black.ttf\n");
        CloseWindow();
        fsdevUnmountDevice("sdmc");
        return 1;
    }

    std::filesystem::path texturePath = "sdmc:/switch/encore/Assets/textures/";
    menuAss.encoreWhiteLogo = menuAss.LoadTextureFilter(texturePath / "encore_white_logo.png");
    loadedAssets++; progress = (float)loadedAssets / totalAssets; DrawLoadingScreen(progress);

    menuAss.highwayTexture = menuAss.LoadTextureFilter(texturePath / "highway.png");
    loadedAssets++; progress = (float)loadedAssets / totalAssets; DrawLoadingScreen(progress);

    GuiSetFont(menuAss.rubik);
    GuiSetStyle(DEFAULT, TEXT_SIZE, 24);
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x181818FF);

    std::filesystem::path directory = "sdmc:/switch/encore/Assets";
    TheSettingsInitializer.InitSettings(directory);

    TheMenuManager.mainMenuManager.Load(directory, menuAss);

    while (!WindowShouldClose()) {
        TheMenuManager.UpdateInput(menuAss, u, directory);
        BeginDrawing();
        ClearBackground(BLACK);
        TheMenuManager.DrawMenu(menuAss, u, directory);
        EndDrawing();
    }
    
    UnloadFont(menuAss.josefinSansItalic);
    UnloadFont(menuAss.rubik);
    UnloadFont(menuAss.rubikBoldItalic);
    UnloadFont(menuAss.rubikItalic);
    UnloadFont(menuAss.redHatDisplayBlack);
    UnloadTexture(menuAss.encoreWhiteLogo);
    UnloadTexture(menuAss.highwayTexture);
    CloseWindow();
    fsdevUnmountDevice("sdmc");
    return 0;
}