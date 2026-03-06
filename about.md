# <c-229faf>Multitask</c>

## <c-137e9e>What is Multitask?</c>

**Multitask** is a Geode mod that lets you play **Geometry Dash** while the game is out of focus. This means you can:

- Watch videos or browse the web while practicing levels
- Multitask without interrupting your gameplay
- Keep grinding those hard levels without being locked to the game window

---

## <c-0d5a8c>Important Notes</c>

### <c-ff6b6b>⚠️ Required Window Mode</c>

The mod only works in **windowed or borderless** mode. Fullscreen is not supported as it automatically hides the game when not in focus.

### <c-ffd700>⚙️ Configure Your Keybinds</c>

We **highly recommend** changing the default keybinds before using the mod. The defaults may conflict with other applications or games you have running.

### <c-4ecdc4>🎮 Keybind Behavior</c>

The custom keybinds **only work when the game is not in focus**. When the game is in focus, controls behave normally and the mod won't intercept your inputs.

---

## <c-95a5a6>Known Issues</c>

### Jump Input After Respawn

When respawning in a level while holding the jump button, there's a chance the jump won't register immediately after respawn. This occurs because the mod cannot force a jump during the first few frames after respawning.

**Current workaround**: The mod waits 5 frames before calling the jump function, which helps but doesn't completely solve the issue.