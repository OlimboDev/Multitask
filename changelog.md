# 2.0.4
- Added Caching to handleVK so that it doesn't do for loops in for loops anymore.
- Added a donation link.
- Fixed a bit of ugly code that we missed.
- Updated Geode from 5.5.3 to 5.7.1.

# 2.0.3
- Added missing keybinds to GeodeKeybindMapper.
- Added mouse 4 and mouse 5 support.
- Split up the code into mouse and keyboard classes.
- Added support for multiple keybinds for the same action (e.g. jump can be both space and W).

# 2.0.2
- Added a check for when the window is focussed or not.
- Added a timestamp to our execution of queueButton.
- We no longer use queueInMainThread.

# 2.0.1
- Added the "cheat" tag as our technique technically allows for pause buffering.

# 2.0.0
- Completely reworked the mod so it would align better with the Geode guidelines.
- We now actually destroy the created hook after leaving the level to avoid left-over hooks.
- Running the game in fullscreen now won't enable our hook at all.
- Jump is now properly kept after death.

# 1.0.0
- Initial release of the Multitask mod.
