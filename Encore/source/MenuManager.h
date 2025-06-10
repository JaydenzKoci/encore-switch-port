//
// Created by Jaydenz on 6/9/2025.
//

#ifndef MENUMANAGER_H
#define MENUMANAGER_H

#include <switch.h>
#include <raylib.h>
#include <filesystem>
#include <string>

enum MenuScreen {
    ATTRACT_SCREEN,
    MAIN_MENU,
    SONG_SELECT,
    SETTINGS
};

float Remap(float value, float in_min, float in_max, float out_min, float out_max);

class Assets {
public:
    Font josefinSansItalic;
    Font rubik;
    Font rubikBoldItalic;
    Font rubikItalic;
    Font redHatDisplayBlack;
    Texture2D encoreWhiteLogo;
    Texture2D highwayTexture;

    static Assets& getInstance();
    Font LoadFontFilter(const std::filesystem::path& fontPath);
    Texture2D LoadTextureFilter(const std::filesystem::path& texturePath);

private:
    Assets();
};

namespace Encore {
    class Settings {
    public:
        bool Fullscreen = false;
        int Framerate = 120;
    };

    class SettingsInit {
    public:
        void InitSettings(const std::filesystem::path& directory);
    };
}

class Units {
public:
    float LeftSide = 0;
    float RightSide = 0;

    float wpct(float percent);
    float hpct(float percent);
    float winpct(float percent);
    float hinpct(float percent);

    static Units& getInstance();

private:
    Units();
};

class GameMenu {
public:
    void DrawTopOvershell(float TopOvershell, Units& u);
    void DrawBottomOvershell(Units& u);
    void DrawAlbumArtBackground(Texture2D song, Assets& assets, Units& u);
    void DrawVersion(Assets& assets, Units& u, const std::string& version, const std::string& commitHash);
    void DrawFPS(Assets& assets, Units& u, int posX, int posY);
};

class MainMenu {
public:
    std::string SplashString;
    bool albumArtLoaded = false;
    Texture2D AlbumArtBackground;
    bool playersActive = false;

    void ChooseSplashText(const std::filesystem::path& directory, Assets& assets);
    void Load(const std::filesystem::path& directory, Assets& assets);
};

class MainMenuManager {
public:
    MainMenu mainMenu;
    int highlightedButton = 0;
    bool debug_enabled = true;

    void Load(const std::filesystem::path& directory, Assets& assets);
    void UpdateInput(Assets& assets, Units& u, class MenuManager& menuManager, const std::filesystem::path& directory);
    void Draw(Assets& assets, Units& u, GameMenu& gameMenu, class MenuManager& menuManager, const std::filesystem::path& directory);

private:
    void DrawAttractScreen(Assets& assets, Units& u, GameMenu& gameMenu);
    void DrawMainMenuScreen(Assets& assets, Units& u, GameMenu& gameMenu, class MenuManager& menuManager, const std::filesystem::path& directory);

    int dpad_hold_counter = 0;
    const int hold_delay = 8;
    const int hold_repeat = 2;
    const float joystick_deadzone = 0.15f;
    float last_joystick_time = 0.0f;
    const float joystick_delay = 0.1f;
};

class MenuManager {
public:
    MenuScreen currentScreen = ATTRACT_SCREEN;
    MainMenuManager mainMenuManager;
    GameMenu gameMenu;

    void SwitchScreen(MenuScreen screen);
    void UpdateInput(Assets& assets, Units& u, const std::filesystem::path& directory);
    void DrawMenu(Assets& assets, Units& u, const std::filesystem::path& directory);

private:
    void DrawSongSelectMenu(Assets& assets, Units& u);
    void DrawSettingsMenu(Assets& assets, Units& u);
};

#endif // MENUMANAGER_H