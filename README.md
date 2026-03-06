# Multitask

> A Geode mod that lets you play Geometry Dash while the game is out of focus

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Geode](https://img.shields.io/badge/Geode-Mod-blue.svg)](https://geode-sdk.org/)

## Usage

Once installed, the mod allows you to control Geometry Dash using custom keybinds when the game window is not in focus. This means you can:

- Watch videos or browse the web while practicing levels
- Multitask without interrupting your gameplay
- Keep grinding those hard levels without being locked to the game window

## Compatibility
- Windows 11
- Geode v5.3.0
- Geometry Dash 2.2081
> Lower versions are untested but may work

### Important Notes

#### ⚠️ Required Window Mode

The mod only works in **windowed or borderless** mode. Fullscreen is not supported as it automatically hides the game when not in focus.

#### ⚙️ Configure Your Keybinds

We **highly recommend** changing the default keybinds before using the mod. The defaults may conflict with other applications or games you have running.

#### 🎮 Keybind Behavior

The custom keybinds **only work when the game is not in focus**. When the game is in focus, controls behave normally and the mod won't intercept your inputs.

## Known Issues

### Jump Input After Respawn

When respawning in a level while holding the jump button, there's a chance the jump won't register immediately after respawn. This occurs because the mod cannot force a jump during the first few frames after respawning.

**Current workaround**: The mod waits 5 frames before calling the jump function, which helps but doesn't completely solve the issue.

**Want to help out?** If you're a developer and have ideas for fixing this, we'd love your contribution!<br/> You may fork the project and make a branch which we can merge in using a pull request. ❤️

## Contributing

Contributions are welcome!
<br/>
If you have any questions, suggestions, or issues, feel free to open an issue or contact us on Discord.
<br/>
[MishaOpstal](https://discord.com/users/366199736765513729), [Chillsy.](https://discord.com/users/442395392244711444)

## License

This project is licensed under the MIT License – see the [LICENSE](LICENSE) file for details.