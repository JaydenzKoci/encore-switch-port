//
// Created by Jaydenz on 6/9/2025.
//

#include "MenuManager.h"
#include <raygui.h>
#include <random>
#include <fstream>

float Remap(float value, float in_min, float in_max, float out_min, float out_max) {
    return out_min + (value - in_min) * (out_max - out_min) / (in_max - in_min);
}

Assets& Assets::getInstance() {
    static Assets instance;
    return instance;
}

Assets::Assets() {}

Font Assets::LoadFontFilter(const std::filesystem::path& fontPath) {
    int fontSize = 128;
    Font font = LoadFontEx(fontPath.string().c_str(), fontSize, nullptr, 250);
    if (font.baseSize == 0) return font;
    font.baseSize = 128;
    font.glyphCount = 250;
    int fileSize = 0;
    unsigned char* fileData = LoadFileData(fontPath.string().c_str(), &fileSize);
    font.glyphs = LoadFontData(fileData, fileSize, 128, 0, 250, FONT_SDF);
    std::filesystem::path atlasFilePath = fontPath.parent_path() / (fontPath.stem().string() + ".png");
    Image atlas = GenImageFontAtlas(font.glyphs, &font.recs, 250, 128, 4, 0);
    ExportImage(atlas, atlasFilePath.string().c_str());
    font.texture = LoadTextureFromImage(atlas);
    UnloadImage(atlas);
    SetTextureFilter(font.texture, TEXTURE_FILTER_TRILINEAR);
    return font;
}

Texture2D Assets::LoadTextureFilter(const std::filesystem::path& texturePath) {
    Texture2D tex = LoadTexture(texturePath.string().c_str());
    GenTextureMipmaps(&tex);
    SetTextureFilter(tex, TEXTURE_FILTER_TRILINEAR);
    return tex;
}

namespace Encore {
    void SettingsInit::InitSettings(const std::filesystem::path& directory) {
    }
}

Units& Units::getInstance() {
    static Units instance;
    return instance;
}

Units::Units() {
    LeftSide = 0;
    RightSide = (float)GetScreenWidth();
}

float Units::wpct(float percent) { return GetScreenWidth() * percent; }
float Units::hpct(float percent) { return GetScreenHeight() * percent; }
float Units::winpct(float percent) { return GetScreenWidth() * percent; }
float Units::hinpct(float percent) { return GetScreenHeight() * percent; }

void GameMenu::DrawTopOvershell(float TopOvershell, Units& u) {
    DrawRectangleGradientV(
        0, u.hpct(TopOvershell) - 2, GetScreenWidth(), u.hinpct(0.025f),
        Color{0, 0, 0, 128}, Color{0, 0, 0, 0}
    );
    DrawRectangle(0, 0, GetScreenWidth(), u.hpct(TopOvershell), WHITE);
    DrawRectangle(
        0, 0, GetScreenWidth(), u.hpct(TopOvershell) - u.hinpct(0.005f),
        ColorBrightness(GetColor(0x181827FF), -0.25f)
    );
}

void GameMenu::DrawBottomOvershell(Units& u) {
    float BottomOvershell = GetScreenHeight() - u.hpct(0.15f);
    DrawRectangle(0, BottomOvershell, (float)GetScreenWidth(), (float)GetScreenHeight(), WHITE);
    DrawRectangle(
        0, BottomOvershell + u.hinpct(0.005f), (float)GetScreenWidth(), (float)GetScreenHeight(),
        ColorBrightness(GetColor(0x181827FF), -0.5f)
    );
}

void GameMenu::DrawAlbumArtBackground(Texture2D song, Assets& assets, Units& u) {
    float diagonalLength = sqrtf(
        (float)(GetScreenWidth() * GetScreenWidth()) + (float)(GetScreenHeight() * GetScreenHeight())
    );
    float RectXPos = GetScreenWidth() / 2;
    float RectYPos = diagonalLength / 2;
    DrawTexturePro(
        song,
        Rectangle{0, 0, (float)song.width, (float)song.width},
        Rectangle{RectXPos, -RectYPos * 2, diagonalLength * 2, diagonalLength * 2},
        {0, 0}, 45, WHITE
    );
}

void GameMenu::DrawVersion(Assets& assets, Units& u, const std::string& version, const std::string& commitHash) {
    std::string gitBranch = "main";
    DrawTextEx(
        assets.josefinSansItalic,
        TextFormat("%s-%s:%s", version.c_str(), gitBranch.c_str(), commitHash.c_str()),
        {u.wpct(0.0025f), u.hpct(0.0025f)},
        u.hinpct(0.025f), 0, WHITE
    );
}

void GameMenu::DrawFPS(Assets& assets, Units& u, int posX, int posY) {
    Color color = LIME;
    int fps = GetFPS();
    if (fps < 45 && fps >= 15) color = ORANGE;
    else if (fps < 20) color = RED;

    DrawTextEx(
        assets.josefinSansItalic, TextFormat("%2i FPS", fps),
        {(float)posX, (float)posY}, u.hinpct(0.025f), 0, color
    );
}

void MainMenu::ChooseSplashText(const std::filesystem::path& directory, Assets& assets) {
    std::ifstream splashes(directory / "ui/splashes.txt");
    if (!splashes.is_open()) {
        SplashString = "Welcome to Encore!";
        return;
    }

    std::random_device seed;
    std::mt19937 prng(seed());
    std::string line, result;
    for (std::size_t n = 0; std::getline(splashes, line); n++) {
        std::uniform_int_distribution<> dist(0, n);
        if (dist(prng) < 1) result = line;
    }
    SplashString = result;
    splashes.close();
}

void MainMenu::Load(const std::filesystem::path& directory, Assets& assets) {
    ChooseSplashText(directory, assets);
    AlbumArtBackground = assets.highwayTexture;
    albumArtLoaded = true;
}

void MainMenuManager::Load(const std::filesystem::path& directory, Assets& assets) {
    mainMenu.Load(directory, assets);
}

void MainMenuManager::UpdateInput(Assets& assets, Units& u, MenuManager& menuManager, const std::filesystem::path& directory) {
    float current_time = GetTime();
    float joystick_y = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y);

    if (debug_enabled) {
        printf("Highlighted Button: %d, FPS: %d\n", highlightedButton, GetFPS());
        if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_DOWN)) {
            printf("D-pad Held: (Up: %d, Down: %d)\n", IsKeyDown(KEY_UP), IsKeyDown(KEY_DOWN));
        }
        if (joystick_y != 0.0f) {
            printf("Joystick Y: %f (Deadzone: %f)\n", joystick_y, joystick_deadzone);
        }
        if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) printf("Button B Pressed\n");
        if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)) printf("Button A Pressed\n");
        if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_UP)) printf("Button X Pressed\n");
        if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_LEFT)) printf("Button Y Pressed\n");
        if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE_RIGHT)) printf("Button Plus Pressed\n");
    }

    int prev_highlighted = highlightedButton;
    if (IsKeyDown(KEY_DOWN)) {
        dpad_hold_counter++;
        if (dpad_hold_counter == 1 || (dpad_hold_counter > hold_delay && (dpad_hold_counter - hold_delay) % hold_repeat == 0)) {
            if (highlightedButton < 2) {
                highlightedButton++;
                if (debug_enabled) printf("D-pad Down: %d (Counter: %d)\n", highlightedButton, dpad_hold_counter);
            }
        }
    } else if (IsKeyDown(KEY_UP)) {
        dpad_hold_counter++;
        if (dpad_hold_counter == 1 || (dpad_hold_counter > hold_delay && (dpad_hold_counter - hold_delay) % hold_repeat == 0)) {
            if (highlightedButton > 0) {
                highlightedButton--;
                if (debug_enabled) printf("D-pad Up: %d (Counter: %d)\n", highlightedButton, dpad_hold_counter);
            }
        }
    } else {
        dpad_hold_counter = 0;
    }

    if (current_time - last_joystick_time > joystick_delay) {
        if (joystick_y > joystick_deadzone && highlightedButton < 2) {
            highlightedButton++;
            last_joystick_time = current_time;
            if (debug_enabled) printf("Joystick Down: %d (Y: %f)\n", highlightedButton, joystick_y);
        } else if (joystick_y < -joystick_deadzone && highlightedButton > 0) {
            highlightedButton--;
            last_joystick_time = current_time;
            if (debug_enabled) printf("Joystick Up: %d (Y: %f)\n", highlightedButton, joystick_y);
        }
    }

    if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE_RIGHT)) {
        if (debug_enabled) printf("PLUS pressed: Exiting\n");
        CloseWindow();
    }

    if (menuManager.currentScreen == ATTRACT_SCREEN) {
        if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)) {
            mainMenu.playersActive = true;
            menuManager.SwitchScreen(MAIN_MENU);
            if (debug_enabled) printf("A pressed: Enter main menu\n");
        }
    } else if (menuManager.currentScreen == MAIN_MENU) {
        if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)) {
            if (mainMenu.playersActive && highlightedButton == 0) {
                menuManager.SwitchScreen(SONG_SELECT);
                if (debug_enabled) printf("A pressed: Play\n");
            } else if (!mainMenu.playersActive && highlightedButton == 0) {
                mainMenu.playersActive = true;
                if (debug_enabled) printf("A pressed: Add player\n");
            } else if (highlightedButton == 1) {
                menuManager.SwitchScreen(SETTINGS);
                if (debug_enabled) printf("A pressed: Options\n");
            } else if (highlightedButton == 2) {
                mainMenu.playersActive = false;
                menuManager.SwitchScreen(ATTRACT_SCREEN);
                if (debug_enabled) printf("A pressed: Quit\n");
            }
        }
        if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) {
            mainMenu.playersActive = false;
            menuManager.SwitchScreen(ATTRACT_SCREEN);
            if (debug_enabled) printf("B pressed: Back to attract\n");
        }
        if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_LEFT)) {
            mainMenu.ChooseSplashText(directory, assets);
            if (debug_enabled) printf("Y pressed: Refresh splash\n");
        }
        if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_UP)) {
            mainMenu.playersActive = false;
            highlightedButton = 0;
            if (debug_enabled) printf("X pressed: Reset\n");
        }
    }
}

void MainMenuManager::DrawAttractScreen(Assets& assets, Units& u, GameMenu& gameMenu) {
    float SplashFontSize = u.hinpct(0.03f);
    Vector2 SplashSize = MeasureTextEx(assets.josefinSansItalic, mainMenu.SplashString.c_str(), SplashFontSize, 0);
    Color accentColor = ColorBrightness(ColorContrast(Color{255, 0, 255, 128}, -0.125f), -0.25f);

    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color{0, 0, 0, 128});

    float LogoHeight = u.winpct(0.1f);
    float LogoWidth = u.winpct(0.48f);
    Vector2 GreetSplashPos = {u.wpct(0.5f) - (SplashSize.x / 2), u.hpct(0.36f) + LogoHeight};
    Rectangle LogoRect = {u.wpct(0.5f) - (LogoWidth / 2), u.hpct(0.35f), LogoWidth, LogoHeight};

    DrawTextEx(assets.josefinSansItalic, mainMenu.SplashString.c_str(), GreetSplashPos, SplashFontSize, 0, WHITE);
    std::string hint = "Press A to select a profile";
    float HintWidth = MeasureTextEx(assets.rubik, hint.c_str(), SplashFontSize, 0).x;
    Vector2 HintPos = {u.wpct(0.5f) - (HintWidth / 2), u.hpct(0.5f) + LogoHeight};
    DrawTextEx(assets.rubik, hint.c_str(), HintPos, SplashFontSize, 0, WHITE);

    DrawTexturePro(
        assets.encoreWhiteLogo,
        {0, 0, (float)assets.encoreWhiteLogo.width, (float)assets.encoreWhiteLogo.height},
        LogoRect, {0, 0}, 0, WHITE
    );

    gameMenu.DrawTopOvershell(0.2f, u);
    gameMenu.DrawBottomOvershell(u);
}

void MainMenuManager::DrawMainMenuScreen(Assets& assets, Units& u, GameMenu& gameMenu, MenuManager& menuManager, const std::filesystem::path& directory) {
    float SplashFontSize = u.hinpct(0.03f);
    Vector2 SplashSize = MeasureTextEx(assets.josefinSansItalic, mainMenu.SplashString.c_str(), SplashFontSize, 0);
    Color accentColor = ColorBrightness(ColorContrast(Color{255, 0, 255, 128}, -0.125f), -0.25f);

    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color{0, 0, 0, 128});
    gameMenu.DrawTopOvershell(0.2f, u);
    gameMenu.DrawVersion(assets, u, "1.0.0", "abcdefg");

    float logoHeight = u.hinpct(0.15f);
    DrawRectangle(0, u.hpct(0.2f), u.LeftSide, u.hinpct(0.05f), accentColor);
    DrawRectangleGradientH(
        u.LeftSide, u.hpct(0.2f), SplashSize.x + u.winpct(0.1f), u.hinpct(0.05f),
        accentColor, Color{0, 0, 0, 0}
    );

    DrawTextEx(assets.josefinSansItalic, mainMenu.SplashString.c_str(), {u.wpct(0.01f), u.hpct(0.2125f)}, SplashFontSize, 0, WHITE);

    Rectangle LogoRect = {
        u.LeftSide + u.winpct(0.01f), u.hpct(0.035f),
        Remap(assets.encoreWhiteLogo.height, 0, assets.encoreWhiteLogo.width / 4.25, 0, u.winpct(0.5f)),
        logoHeight
    };
    DrawTexturePro(
        assets.encoreWhiteLogo,
        {0, 0, (float)assets.encoreWhiteLogo.width, (float)assets.encoreWhiteLogo.height},
        LogoRect, {0, 0}, 0, WHITE
    );

    GuiSetStyle(DEFAULT, TEXT_SIZE, (int)u.hinpct(0.08f));
    GuiSetFont(assets.redHatDisplayBlack);
    GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
    GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, 0xaaaaaaFF);
    GuiSetStyle(BUTTON, TEXT_COLOR_FOCUSED, 0xFFFFFFFF);
    GuiSetStyle(BUTTON, BORDER_WIDTH, 0);
    GuiSetStyle(BUTTON, BACKGROUND_COLOR, 0);
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x00000000);
    GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, 0x00000000);
    GuiSetStyle(BUTTON, BASE_COLOR_PRESSED, 0x00000000);
    GuiSetStyle(BUTTON, BORDER_COLOR_NORMAL, 0x00000000);
    GuiSetStyle(BUTTON, BORDER_COLOR_FOCUSED, 0x00000000);
    GuiSetStyle(BUTTON, BORDER_COLOR_PRESSED, 0x00000000);
    GuiSetStyle(DEFAULT, BACKGROUND_COLOR, 0x00000000);

    if (GuiButton({0, u.hpct(0.8f), u.LeftSide + SplashSize.x, u.hpct(0.05f)}, "")) {
        mainMenu.ChooseSplashText(directory, assets);
    }

    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, highlightedButton == 0 ? 0x181827FF : 0x00000000);
    if (mainMenu.playersActive) {
        GuiButton({u.wpct(0.02f), u.hpct(0.3f), u.winpct(0.2f), u.hinpct(0.08f)}, "Play");
    } else {
        GuiButton({u.wpct(0.02f), u.hpct(0.3f), u.winpct(0.2f), u.hinpct(0.08f)}, "Add a player");
    }

    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, highlightedButton == 1 ? 0x181827FF : 0x00000000);
    GuiButton({u.wpct(0.02f), u.hpct(0.39f), u.winpct(0.5), u.hinpct(0.08f)}, "Options");

    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, highlightedButton == 2 ? 0x181827FF : 0x00000000);
    GuiButton({u.wpct(0.02f), u.hpct(0.48f), u.winpct(0.2f), u.hinpct(0.08f)}, "Quit");

    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x181827FF);
    GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, ColorToInt(ColorBrightness(Color{255, 0, 255, 255}, -0.5)));
    GuiSetStyle(BUTTON, BASE_COLOR_PRESSED, ColorToInt(ColorBrightness(Color{255, 0, 255, 255}, -0.3)));
    GuiSetStyle(BUTTON, BORDER_COLOR_NORMAL, 0xFFFFFFFF);
    GuiSetStyle(BUTTON, BORDER_COLOR_FOCUSED, 0xFFFFFFFF);
    GuiSetStyle(BUTTON, BORDER_COLOR_PRESSED, 0xFFFFFFFF);
    GuiSetStyle(DEFAULT, BACKGROUND_COLOR, 0x505050ff);
    GuiSetStyle(BUTTON, BORDER_WIDTH, 2);

    GuiSetStyle(DEFAULT, TEXT_SIZE, (int)u.hinpct(0.03f));
    GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
    GuiSetFont(assets.rubik);

    gameMenu.DrawBottomOvershell(u);
}

void MainMenuManager::Draw(Assets& assets, Units& u, GameMenu& gameMenu, MenuManager& menuManager, const std::filesystem::path& directory) {
    if (mainMenu.albumArtLoaded) {
        gameMenu.DrawAlbumArtBackground(mainMenu.AlbumArtBackground, assets, u);
    }
    if (mainMenu.playersActive) {
        DrawMainMenuScreen(assets, u, gameMenu, menuManager, directory);
    } else {
        DrawAttractScreen(assets, u, gameMenu);
    }
    gameMenu.DrawFPS(assets, u, GetScreenWidth() - 100, 10);
}

void MenuManager::SwitchScreen(MenuScreen screen) {
    currentScreen = screen;
    if (screen == MAIN_MENU || screen == ATTRACT_SCREEN) {
        mainMenuManager.highlightedButton = 0;
    }
}

void MenuManager::UpdateInput(Assets& assets, Units& u, const std::filesystem::path& directory) {
    if (currentScreen == ATTRACT_SCREEN || currentScreen == MAIN_MENU) {
        mainMenuManager.UpdateInput(assets, u, *this, directory);
    }
}

void MenuManager::DrawMenu(Assets& assets, Units& u, const std::filesystem::path& directory) {
    switch (currentScreen) {
        case ATTRACT_SCREEN:
        case MAIN_MENU:
            mainMenuManager.Draw(assets, u, gameMenu, *this, directory);
            break;
        case SONG_SELECT:
            DrawSongSelectMenu(assets, u);
            break;
        case SETTINGS:
            DrawSettingsMenu(assets, u);
            break;
    }
}

void MenuManager::DrawSongSelectMenu(Assets& assets, Units& u) {
    ClearBackground(BLACK);
    DrawTextEx(
        assets.josefinSansItalic, "Song Selection",
        {(float)GetScreenWidth() / 2 - MeasureTextEx(assets.josefinSansItalic, "Song Selection", 40, 1).x / 2, 100},
        40, 1, WHITE
    );

    if (GuiButton({(float)GetScreenWidth() / 2 - 100, 200, 200, 50}, "Song 1")) {
        TraceLog(LOG_INFO, "Song 1 selected");
    }
    if (GuiButton({(float)GetScreenWidth() / 2 - 100, 300, 200, 50}, "Song 2")) {
        TraceLog(LOG_INFO, "Song 2 selected");
    }
    if (GuiButton({(float)GetScreenWidth() / 2 - 100, 400, 200, 50}, "Back")) {
        currentScreen = MAIN_MENU;
    }
}

void MenuManager::DrawSettingsMenu(Assets& assets, Units& u) {
    ClearBackground(BLACK);
    DrawTextEx(
        assets.josefinSansItalic, "Settings",
        {(float)GetScreenWidth() / 2 - MeasureTextEx(assets.josefinSansItalic, "Settings", 40, 1).x / 2, 100},
        40, 1, WHITE
    );

    static float volume = 0.5f;
    GuiSlider({(float)GetScreenWidth() / 2 - 100, 200, 200, 20}, "Volume", TextFormat("%.2f", volume), &volume, 0.0f, 1.0f);
    if (GuiButton({(float)GetScreenWidth() / 2 - 100, 400, 200, 50}, "Back")) {
        currentScreen = MAIN_MENU;
    }
}