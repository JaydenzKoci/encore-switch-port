# Encore Switch Port | ![PortProgress](https://geps.dev/progress/3)

[![Encore for PC](icon.jpg)](https://github.com/Encore-Developers/Encore)

Up-and-coming 4/5 key rhythm game.

Encore supports all switch controllers and keyboard. Songs formatted for other games will NOT work with Encore.

[![Chat with us on Discord](https://cdn.jsdelivr.net/npm/@intergrav/devins-badges@3/assets/cozy/social/discord-plural_vector.svg)](https://discord.gg/GhkgVUAC9v)

## How To Install
You download any of the releases and take the switch folder and drag and drop it into your switch sd card. After that, you can optionally download [Encore.nsp](https://github.com/JaydenzKoci/encore-switch-port/raw/refs/heads/main/Encore.nsp) and add it to your switch home screen through using Tinfoil or Goldleaf.

## Songs

Extract songs into the `switch/encore/Songs` folder on your switch sd card. They should consist of several files:
Please check [CHARTS.md](https://github.com/Encore-Developers/Encore/blob/main/CHARTS.md) for more song information.
- info.json - the information file required for Encore to read audio, difficulties, the name, and file names.
- lead.ogg (or similar)
- bass.ogg (or similar)
- drums.ogg (or similar)
- vocals.ogg (or similar)
- backing.ogg (or similar)
- notes.mid (typically named after the song's name)

### Contributing

If you have any bugs or suggestions for Encore, please consider making an issue.
For code contributions, you'll need the latest version of Visual Studio or CLion. It's possible that you can use VSCode, but none of us develop with it. These IDEs should install the C++ development toolchains.

## Credits    
Crown icon from [Font Awesome](https://fontawesome.com/) (icon source: [crown](https://fontawesome.com/icons/crown?f=classic&s=solid))

### External libraries used
[raylib-nx](https://github.com/luizpestana/raylib-nx)

[raygui](https://github.com/raysan5/raygui)

[RapidJSON](https://github.com/Tencent/rapidjson)

[midifile](https://github.com/craigsapp/midifile)

[nicolausYes's easing-functions](https://github.com/nicolausYes/easing-functions)

[PicoSHA2](https://github.com/okdshin/PicoSHA2)

[osu! resources](https://github.com/ppy/osu-resources/)
