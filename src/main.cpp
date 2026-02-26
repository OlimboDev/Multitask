/**
 * Include the Geode headers.
 */
#include <Geode/Geode.hpp>
#include <Geode/loader/SettingV3.hpp>
#include <Geode/loader/GameEvent.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include "GeodeKeybindMapper.h"

/**
 * Include the Windows header.
 */
#include <windows.h>

/**
 * Include our Geode to Windows mapper.
 */
#include "GeodeKeybindMapper.h"

/**
 * Brings cocos2d and all Geode namespaces to the current scope.
 */
using namespace geode::prelude;


/**
 * A global variable to store our hook handle.
 */
HHOOK g_hook = nullptr;


enum KeyAction {
	Left,
	Up,
	Right
};


void handlePlayLayerButton(PlayLayer* pl, WPARAM wParam, KeyAction keyAction, bool isSecondPlayer = false) {
	if (wParam == WM_KEYDOWN) {
		queueInMainThread([pl, keyAction, isSecondPlayer] {
			switch (keyAction) {
				case KeyAction::Left:
					pl->handleButton(true, 0, isSecondPlayer);
					break;
				case KeyAction::Up:
					pl->handleButton(true, 1, isSecondPlayer);
					break;
				case KeyAction::Right:
					pl->handleButton(true, 2, isSecondPlayer);
					break;
				default:
					break;
			}
		});
	}

	if (wParam == WM_KEYUP) {
		queueInMainThread([pl, keyAction, isSecondPlayer] {
			switch (keyAction) {
				case KeyAction::Left:
					pl->handleButton(false, 0, isSecondPlayer);
					break;
				case KeyAction::Up:
					pl->handleButton(false, 1, isSecondPlayer);
					break;
				case KeyAction::Right:
					pl->handleButton(false, 2, isSecondPlayer);
					break;
				default:
					break;
			}
		});
	}
}


LRESULT CALLBACK KeyboardProc(int code, WPARAM wParam, LPARAM lParam) {
    if (code == HC_ACTION) {
        auto* kb = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
        DWORD vk = kb->vkCode;

        auto player_layer = PlayLayer::get();

        auto first_player_up_key    = GeodeKeybindMapper::vkFromSetting(Mod::get()->getSettingValue<std::vector<geode::Keybind>>("firstPlayerUpKey"));
        auto first_player_left_key  = GeodeKeybindMapper::vkFromSetting(Mod::get()->getSettingValue<std::vector<geode::Keybind>>("firstPlayerLeftKey"));
        auto first_player_right_key = GeodeKeybindMapper::vkFromSetting(Mod::get()->getSettingValue<std::vector<geode::Keybind>>("firstPlayerRightKey"));

        auto second_player_up_key    = GeodeKeybindMapper::vkFromSetting(Mod::get()->getSettingValue<std::vector<geode::Keybind>>("secondPlayerUpKey"));
        auto second_player_left_key  = GeodeKeybindMapper::vkFromSetting(Mod::get()->getSettingValue<std::vector<geode::Keybind>>("secondPlayerLeftKey"));
        auto second_player_right_key = GeodeKeybindMapper::vkFromSetting(Mod::get()->getSettingValue<std::vector<geode::Keybind>>("secondPlayerRightKey"));

    	auto pause_game_key = GeodeKeybindMapper::vkFromSetting(Mod::get()->getSettingValue<std::vector<geode::Keybind>>("pauseGameKey"));

        if (player_layer) {
            if      (first_player_up_key    && vk == *first_player_up_key)    handlePlayLayerButton(player_layer, wParam, KeyAction::Up,    false);
            else if (first_player_left_key  && vk == *first_player_left_key)  handlePlayLayerButton(player_layer, wParam, KeyAction::Left,  false);
            else if (first_player_right_key && vk == *first_player_right_key) handlePlayLayerButton(player_layer, wParam, KeyAction::Right, false);
            else if (second_player_up_key   && vk == *second_player_up_key)   handlePlayLayerButton(player_layer, wParam, KeyAction::Up,    true);
            else if (second_player_left_key && vk == *second_player_left_key) handlePlayLayerButton(player_layer, wParam, KeyAction::Left,  true);
            else if (second_player_right_key && vk == *second_player_right_key) handlePlayLayerButton(player_layer, wParam, KeyAction::Right, true);
        }

        if (pause_game_key && vk == static_cast<DWORD>(*pause_game_key) && wParam == WM_KEYDOWN) {
            queueInMainThread([] {
                if (auto pl = PlayLayer::get()) {
                    if (!pl->m_isPaused) {
                        pl->pauseGame(false);
                    } else {
                        pl->resume();
                        const auto scene = CCDirector::get()->getRunningScene();
                        const auto pauseLayer = scene->getChildByType<PauseLayer>(0);
                        pauseLayer->onResume(nullptr);
                    }
                }
            });
        }
    }

    return CallNextHookEx(g_hook, code, wParam, lParam);
}

class $modify(PlayLayer) {
	void pauseGame(bool unfocused) {
		if (unfocused) return;
		PlayLayer::pauseGame(unfocused);
	}

	void publicPauseGame(bool unfocused) {
		PlayLayer::pauseGame(unfocused);
	}
};

$on_mod(Loaded) {
	g_hook = SetWindowsHookEx(
		WH_KEYBOARD_LL,
		KeyboardProc,
		nullptr,
		0
	);

	if (!g_hook) {
		log::error("Hook failed: {}", GetLastError());
	}
}