# Encore Switch Port

[![Chat with us on Discord](https://cdn.jsdelivr.net/npm/@intergrav/devins-badges@3/assets/cozy/social/discord-plural_vector.svg)](https://discord.gg/GhkgVUAC9v)

Up-and-coming 4/5 key rhythm game.

Encore supports all switch controllers and keyboard. Songs formatted for other games will NOT work with Encore.

## Songs

Extract songs into the `/Songs` folder in your switch folder on your switch. They should consist of several files:
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

### External libraries used
[raylib-nx](https://github.com/luizpestana/raylib-nx)

[raygui](https://github.com/raysan5/raygui)