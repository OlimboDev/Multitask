#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/UILayer.hpp>
#include <Geode/modify/CCEGLView.hpp>

#include <windows.h>
#include <unordered_set>

#include "GeodeKeybindMapper.h"

using namespace geode::prelude;

static std::unordered_set<DWORD> s_pressedKeys;
static HHOOK s_keyboardHook = nullptr;
static HHOOK s_mouseHook    = nullptr;

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

static KeyBinding const s_bindings[] = {
    {"firstPlayerUpKey", Up, true},
    {"firstPlayerLeftKey", Left, true},
    {"firstPlayerRightKey", Right, true},
    {"secondPlayerUpKey", Up, false},
    {"secondPlayerLeftKey", Left, false},
    {"secondPlayerRightKey", Right, false},
};

bool isGDFocused() {
    return GetActiveWindow() == WindowFromDC(wglGetCurrentDC());
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

    for (const auto &[setting, action, isFirstPlayer]: s_bindings) {
        if (auto const key = GeodeKeybindMapper::virtualKeyFromSetting(setting); key && virtualKey == *key) {
            handleButton(wParam, action, isFirstPlayer);
            break;
        }
    }

    if (auto const pauseGameKey = GeodeKeybindMapper::virtualKeyFromSetting("pauseGameKey");
        pauseGameKey && virtualKey == *pauseGameKey && isKeyDown) {
            togglePause();
        }
}

void processInput(DWORD const virtualKey, bool const isKeyDown, bool const isKeyUp) {
    if (isKeyDown) {
        if (s_pressedKeys.contains(virtualKey)) return;
        s_pressedKeys.insert(virtualKey);
    }
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
            registerHook(s_keyboardHook, WH_KEYBOARD_LL, keyboardProc, "Keyboard");
            registerHook(s_mouseHook,    WH_MOUSE_LL,    mouseProc,    "Mouse");

            return true;
        }

        return true;
    }

    // ReSharper disable once CppHidingFunction
    void onQuit() {
        unregisterHook(s_keyboardHook, "keyboard");
        unregisterHook(s_mouseHook, "mouse");
        PlayLayer::onQuit();
    }
};
