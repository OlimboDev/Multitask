#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/UILayer.hpp>
#include <Geode/modify/CCEGLView.hpp>

#include <windows.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "GeodeKeybindMapper.h"

using namespace geode::prelude;

enum KeyAction {
    Left,
    Up,
    Right
};

struct KeyBinding {
    char const *setting;
    KeyAction action;
    bool isFirstPlayer;
};

struct CachedBinding {
    KeyAction action;
    bool isFirstPlayer;
};

static KeyBinding const s_bindings[] = {
    {"firstPlayerUpKey", Up, true},
    {"firstPlayerLeftKey", Left, true},
    {"firstPlayerRightKey", Right, true},
    {"secondPlayerUpKey", Up, false},
    {"secondPlayerLeftKey", Left, false},
    {"secondPlayerRightKey", Right, false},
};

static std::unordered_set<DWORD> s_pressedKeys;
static std::unordered_map<DWORD, std::vector<CachedBinding>> s_vkBindingCache;
static std::unordered_set<DWORD> s_pausedKeys;
static HHOOK s_keyboardHook = nullptr;
static HHOOK s_mouseHook    = nullptr;
static HWND s_gameWindow    = nullptr;

static void rebuildBindings() {
    s_vkBindingCache.clear();
    s_pausedKeys.clear();

    for (auto const &[setting, action, isFirstPlayer]: s_bindings) {
        for (int const key : GeodeKeybindMapper::virtualKeysFromSetting(setting)) {
            s_vkBindingCache[key].emplace_back(action, isFirstPlayer);
        }
    }

    for (int const key : GeodeKeybindMapper::virtualKeysFromSetting("pauseGameKey")) {
        s_pausedKeys.insert(static_cast<DWORD>(key));
    }
}

bool isGDFocused() {
    return GetActiveWindow() == s_gameWindow;
}

bool exitCheck(int const code) {
    if (!Mod::get()->getSettingValue<bool>("enabled")) return true;
    if (isGDFocused()) return true;
    if (code != HC_ACTION) return true;
    return false;
}

void handleButton(WPARAM const wParam, KeyAction const keyAction, bool const isFirstPlayer = false) {
    bool const isDown = wParam == WM_KEYDOWN;
    PlayerButton button;

    switch (keyAction) {
        case Left: button = PlayerButton::Left;
            break;
        case Up: button = PlayerButton::Jump;
            break;
        case Right: button = PlayerButton::Right;
            break;
        default: return;
    }

    auto const playLayer = PlayLayer::get();
    if (!playLayer) return;

    playLayer->queueButton(static_cast<int>(button), isDown, !isFirstPlayer, getInputTimestamp());

    if (button != PlayerButton::Jump) return;
    auto const uiLayer = playLayer->getChildByType<UILayer>(0);
    if (!uiLayer) return;

    if (isFirstPlayer) uiLayer->m_p1Jumping = isDown;
    else uiLayer->m_p2Jumping = isDown;
}

void togglePause() {
    auto const scene = CCDirector::get()->getRunningScene();
    auto const playLayer = PlayLayer::get();
    if (!playLayer) return;

    if (playLayer->m_isPaused) {
        playLayer->resume();
        if (auto const pauseLayer = scene->getChildByType<PauseLayer>(0)) pauseLayer->onResume(nullptr);
        return;
    }

    playLayer->pauseGame(false);
}

void handleVK(DWORD const virtualKey, bool const isKeyDown) {
    WPARAM const wParam = isKeyDown ? WM_KEYDOWN : WM_KEYUP;

    if (auto const it = s_vkBindingCache.find(virtualKey); it != s_vkBindingCache.end()) {
        for (auto const &[action, isFirstPlayer] : it->second) {
            handleButton(wParam, action, isFirstPlayer);
        }
    }

    if (isKeyDown && s_pausedKeys.contains(virtualKey)) togglePause();
}

void processInput(DWORD const virtualKey, bool const isKeyDown, bool const isKeyUp) {
    if (isKeyDown && !s_pressedKeys.insert(virtualKey).second) return;
    if (isKeyUp) s_pressedKeys.erase(virtualKey);
    handleVK(virtualKey, isKeyDown);
}

LRESULT CALLBACK keyboardProc(int const code, WPARAM const wParam, LPARAM const lParam) {
    if (exitCheck(code)) return CallNextHookEx(s_keyboardHook, code, wParam, lParam);

    auto const keyBoard = reinterpret_cast<KBDLLHOOKSTRUCT *>(lParam);
    DWORD const virtualKey = keyBoard->vkCode;

    bool const isKeyDown = wParam == WM_KEYDOWN;
    bool const isKeyUp = wParam == WM_KEYUP;

    processInput(virtualKey, isKeyDown, isKeyUp);

    return CallNextHookEx(s_keyboardHook, code, wParam, lParam);
}

LRESULT CALLBACK mouseProc(int const code, WPARAM const wParam, LPARAM const lParam) {
    if (exitCheck(code)) return CallNextHookEx(s_mouseHook, code, wParam, lParam);

    auto const mouse = reinterpret_cast<MSLLHOOKSTRUCT *>(lParam);
    DWORD const virtualKey = (HIWORD(mouse->mouseData) == XBUTTON1)
    ? VK_XBUTTON1
    : VK_XBUTTON2;

    bool const isKeyDown = wParam == WM_XBUTTONDOWN;
    bool const isKeyUp   = wParam == WM_XBUTTONUP;

    processInput(virtualKey, isKeyDown, isKeyUp);

    return CallNextHookEx(s_mouseHook, code, wParam, lParam);
}

static void registerHook(HHOOK &outHook, int const type, LRESULT (CALLBACK *proc)(int, WPARAM, LPARAM), char const *name) {
    outHook = SetWindowsHookEx(type, proc, nullptr, 0);
    if (!outHook) log::error("{} hook failed: {}", name, GetLastError());
    else          log::debug("{} hook created", name);
}

static void unregisterHook(HHOOK &hook, char const *name) {
    if (!hook) return;
    if (UnhookWindowsHookEx(hook)) log::debug("{} hook was destroyed", name);
    else log::warn("{} hook was not destroyed: {}", name, GetLastError());
    hook = nullptr;
}

class $modify(PlayLayer) {
    // ReSharper disable once CppHidingFunction
    bool init(GJGameLevel *level, bool const p1, bool const p2) {
        if (!PlayLayer::init(level, p1, p2)) return false;

        if (!CCEGLView::get()->getIsFullscreen()) {
            s_gameWindow = WindowFromDC(wglGetCurrentDC());
            rebuildBindings();
            registerHook(s_keyboardHook, WH_KEYBOARD_LL, keyboardProc, "GD_Keyboard");
            registerHook(s_mouseHook,    WH_MOUSE_LL,    mouseProc,    "GD_Mouse");
        }

        return true;
    }

    // ReSharper disable once CppHidingFunction
    void onQuit() {
        unregisterHook(s_keyboardHook, "GD_Keyboard");
        unregisterHook(s_mouseHook, "GD_Mouse");
        s_pressedKeys.clear();
        PlayLayer::onQuit();
    }
};
