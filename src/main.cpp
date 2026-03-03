#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>

#include <windows.h>
#include <unordered_set>

#include "GeodeKeybindMapper.h"

using namespace geode::prelude;

std::unordered_set<DWORD> gPressedKeys;

HHOOK gHook = nullptr;


enum KeyAction {
	Left,
	Up,
	Right
};


void handlePlayLayerButton(PlayLayer* playLayer, WPARAM const wParam, KeyAction keyAction, bool isSecondPlayer = false) {
	if (wParam == WM_KEYDOWN) {
		queueInMainThread([playLayer, keyAction, isSecondPlayer] {
			switch (keyAction) {
				case Left:
					playLayer->handleButton(true, 0, isSecondPlayer);
					break;
				case Up:
					playLayer->handleButton(true, 1, isSecondPlayer);
					break;
				case Right:
					playLayer->handleButton(true, 2, isSecondPlayer);
					break;
				default:
					break;
			}
		});
	}

	if (wParam == WM_KEYUP) {
		queueInMainThread([playLayer, keyAction, isSecondPlayer] {
			switch (keyAction) {
				case Left:
					playLayer->handleButton(false, 0, isSecondPlayer);
					break;
				case Up:
					playLayer->handleButton(false, 1, isSecondPlayer);
					break;
				case Right:
					playLayer->handleButton(false, 2, isSecondPlayer);
					break;
				default:
					break;
			}
		});
	}
}

LRESULT CALLBACK KeyboardProc(int const code, WPARAM const wParam, LPARAM const lParam) {
	if (!Mod::get()->getSettingValue<bool>("enabled")) {
		return CallNextHookEx(gHook, code, wParam, lParam);
	}

	if (code == HC_ACTION) {
		auto const keyBoard = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
		DWORD const virtualKey = keyBoard->vkCode;

		bool const isKeyDown = wParam == WM_KEYDOWN;
		bool const isKeyUp = wParam == WM_KEYUP;

		if (isKeyDown) {
			if (gPressedKeys.contains(virtualKey)) {
				return CallNextHookEx(gHook, code, wParam, lParam);
			}
			gPressedKeys.insert(virtualKey);
		}

		if (isKeyUp) {
			gPressedKeys.erase(virtualKey);
		}

		auto const playLayer = PlayLayer::get();
		auto const scene = CCDirector::get()->getRunningScene();

		auto const firstPlayerUpKey    = GeodeKeybindMapper::virtualKeyFromSetting("firstPlayerUpKey");
		auto const firstPlayerLeftKey  = GeodeKeybindMapper::virtualKeyFromSetting("firstPlayerLeftKey");
		auto const firstPlayerRightKey = GeodeKeybindMapper::virtualKeyFromSetting("firstPlayerRightKey");

		auto const secondPlayerUpKey    = GeodeKeybindMapper::virtualKeyFromSetting("secondPlayerUpKey");
		auto const secondPlayerLeftKey  = GeodeKeybindMapper::virtualKeyFromSetting("secondPlayerLeftKey");
		auto const secondPlayerRightKey = GeodeKeybindMapper::virtualKeyFromSetting("secondPlayerRightKey");

		auto const pauseGameKey = GeodeKeybindMapper::virtualKeyFromSetting("pauseGameKey");

		if (playLayer) {
			if      (firstPlayerUpKey    && virtualKey ==* firstPlayerUpKey)    handlePlayLayerButton(playLayer, wParam, Up,    false);
			else if (firstPlayerLeftKey  && virtualKey ==* firstPlayerLeftKey)  handlePlayLayerButton(playLayer, wParam, Left,  false);
			else if (firstPlayerRightKey && virtualKey ==* firstPlayerRightKey) handlePlayLayerButton(playLayer, wParam, Right, false);
			else if (secondPlayerUpKey   && virtualKey ==* secondPlayerUpKey)   handlePlayLayerButton(playLayer, wParam, Up,    true);
			else if (secondPlayerLeftKey && virtualKey ==* secondPlayerLeftKey) handlePlayLayerButton(playLayer, wParam, Left,  true);
			else if (secondPlayerRightKey && virtualKey ==* secondPlayerRightKey) handlePlayLayerButton(playLayer, wParam, Right, true);

			if (pauseGameKey && virtualKey == static_cast<DWORD>(*pauseGameKey) && wParam == WM_KEYDOWN) {
				queueInMainThread([scene, playLayer] {
					if (!playLayer->m_isPaused) {
						playLayer->pauseGame(false);
					} else {
						playLayer->resume();
						auto const pauseLayer = scene->getChildByType<PauseLayer>(0);
						pauseLayer->onResume(nullptr);
					}
				});
			}
		}
	}

	return CallNextHookEx(gHook, code, wParam, lParam);
}

$on_mod(Loaded) {
	gHook = SetWindowsHookEx(
		WH_KEYBOARD_LL,
		KeyboardProc,
		nullptr,
		0
	);

	if (!gHook) {
		log::error("Hook failed: {}", GetLastError());
	}
}