#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/UILayer.hpp>

#include <windows.h>
#include <unordered_set>

#include "GeodeKeybindMapper.h"

using namespace geode::prelude;

static std::unordered_set<DWORD> s_pressedKeys;
static HHOOK s_hook = nullptr;

enum KeyAction {
    Left,
    Up,
    Right
};

void handleButton(
    WPARAM const wParam,
    KeyAction const keyAction,
    bool const isFirstPlayer = false
) {
    bool const isDown = wParam == WM_KEYDOWN;
    PlayerButton button;

    switch (keyAction) {
        case Left:  button = PlayerButton::Left; break;
        case Up:    button = PlayerButton::Jump; break;
        case Right: button = PlayerButton::Right; break;
        default:    return;
    }

    queueInMainThread([button, isDown, isFirstPlayer] {
        auto const PlayLayer = PlayLayer::get();
        if (!PlayLayer) return;

        PlayLayer->queueButton(static_cast<int>(button), isDown, !isFirstPlayer, false);

        if (static_cast<int>(button) != 1) return;

        auto const uiLayer = PlayLayer->getChildByType<UILayer>(0);
        if (!uiLayer) return;

        if (isFirstPlayer) {
            uiLayer->m_p1Jumping = isDown;
        } else {
            uiLayer->m_p2Jumping = isDown;
        }
    });
}

LRESULT CALLBACK KeyboardProc(
    int const code,
    WPARAM const wParam,
    LPARAM const lParam
) {
    if (!Mod::get()->getSettingValue<bool>("enabled")) {
        return CallNextHookEx(s_hook, code, wParam, lParam);
    }
    if (code != HC_ACTION) {
        return CallNextHookEx(s_hook, code, wParam, lParam);
    }

    auto const keyBoard = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
    DWORD const virtualKey = keyBoard->vkCode;

    bool const isKeyDown = wParam == WM_KEYDOWN;
    bool const isKeyUp = wParam == WM_KEYUP;

    log::info("Key event → vk: {}, down: {}, up: {}", virtualKey, isKeyDown, isKeyUp);

    if (isKeyDown) {
        if (s_pressedKeys.contains(virtualKey)) {
            return CallNextHookEx(s_hook, code, wParam, lParam);
        }
        s_pressedKeys.insert(virtualKey);
    }

    if (isKeyUp) {
        s_pressedKeys.erase(virtualKey);
    }

    struct KeyBinding {
        char const* setting;
        KeyAction action;
        bool isFirstPlayer;
    };

    static KeyBinding const s_bindings[] = {
        { "firstPlayerUpKey",     Up,    true  },
        { "firstPlayerLeftKey",   Left,  true  },
        { "firstPlayerRightKey",  Right, true  },
        { "secondPlayerUpKey",    Up,    false },
        { "secondPlayerLeftKey",  Left,  false },
        { "secondPlayerRightKey", Right, false },
    };

    for (const auto&[setting, action, isFirstPlayer] : s_bindings) {
        auto const key = GeodeKeybindMapper::virtualKeyFromSetting(
            setting
        );
        if (key && virtualKey == *key) {
            handleButton(wParam, action, isFirstPlayer);
            break;
        }
    }

    if (
        auto const pauseGameKey = GeodeKeybindMapper::virtualKeyFromSetting("pauseGameKey");
        !pauseGameKey || virtualKey != static_cast<DWORD>(*pauseGameKey)
    ) {
        return CallNextHookEx(s_hook, code, wParam, lParam);
    }

    if (wParam != WM_KEYDOWN) {
        return CallNextHookEx(s_hook, code, wParam, lParam);
    }

    queueInMainThread([] {
        auto const pl = PlayLayer::get();
        if (!pl) return;

        if (!pl->m_isPaused) {
            return;
        }
        auto const scene = CCDirector::get()->getRunningScene();
        if (auto const pauseLayer = scene->getChildByType<PauseLayer>(0)) pauseLayer->onResume(nullptr);
    });

    return CallNextHookEx(s_hook, code, wParam, lParam);
}

$on_mod(Loaded) {
    s_hook = SetWindowsHookEx(
        WH_KEYBOARD_LL,
        KeyboardProc,
        nullptr,
        0
    );

    if (!s_hook) {
        log::error("Hook failed: {}", GetLastError());
    }
}