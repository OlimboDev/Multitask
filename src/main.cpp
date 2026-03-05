#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>


#include <windows.h>
#include <unordered_set>

#include "GeodeKeybindMapper.h"

using namespace geode::prelude;

std::unordered_set<DWORD> gPressedKeys;
bool bIsInDeadDelay = false;
int iDeathDelayFrames = 3;
int iCurrentDeathDelayFrame = 0;

HHOOK gHook = nullptr;


enum KeyAction {
	Left,
	Up,
	Right
};


void handleButton(WPARAM const wParam, KeyAction keyAction, bool const isFirstPlayer = false) {
	auto const baseGameLayer = GJBaseGameLayer::get();
	if (!baseGameLayer) {
		return;
	}

	if (wParam == WM_KEYDOWN) {
		queueInMainThread([baseGameLayer, keyAction, isFirstPlayer] {
			switch (keyAction) {
				case Left:
					baseGameLayer->handleButton(true, 0, isFirstPlayer);
					break;
				case Up:
					baseGameLayer->handleButton(true, 1, isFirstPlayer);
					break;
				case Right:
					baseGameLayer->handleButton(true, 2, isFirstPlayer);
					break;
				default:
					break;
			}
		});
	}

	if (wParam == WM_KEYUP) {
		queueInMainThread([baseGameLayer, keyAction, isFirstPlayer] {
			switch (keyAction) {
				case Left:
					baseGameLayer->handleButton(false, 0, isFirstPlayer);
					break;
				case Up:
					baseGameLayer->handleButton(false, 1, isFirstPlayer);
					break;
				case Right:
					baseGameLayer->handleButton(false, 2, isFirstPlayer);
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

		// Did we die? If so we shouldn't perform the safeguard for clicking until we are at least 1 frame into the level, otherwise we won't be able to click at all.
		if (isKeyDown && PlayLayer::get() && PlayLayer::get()->m_playerDied) {
			bIsInDeadDelay = true;
		}

		if (bIsInDeadDelay) {
			if (iCurrentDeathDelayFrame < iDeathDelayFrames) {
				iCurrentDeathDelayFrame++;
			} else {
				bIsInDeadDelay = false;
				iCurrentDeathDelayFrame = 0;
			}
		} else {
			if (isKeyDown) {
				if (gPressedKeys.contains(virtualKey)) {
					return CallNextHookEx(gHook, code, wParam, lParam);
				}
				gPressedKeys.insert(virtualKey);
			}

			if (isKeyUp) {
				gPressedKeys.erase(virtualKey);
			}
		}

		auto const scene = CCDirector::get()->getRunningScene();

		auto const firstPlayerUpKey    = GeodeKeybindMapper::virtualKeyFromSetting("firstPlayerUpKey");
		auto const firstPlayerLeftKey  = GeodeKeybindMapper::virtualKeyFromSetting("firstPlayerLeftKey");
		auto const firstPlayerRightKey = GeodeKeybindMapper::virtualKeyFromSetting("firstPlayerRightKey");

		auto const secondPlayerUpKey    = GeodeKeybindMapper::virtualKeyFromSetting("secondPlayerUpKey");
		auto const secondPlayerLeftKey  = GeodeKeybindMapper::virtualKeyFromSetting("secondPlayerLeftKey");
		auto const secondPlayerRightKey = GeodeKeybindMapper::virtualKeyFromSetting("secondPlayerRightKey");

		auto const pauseGameKey = GeodeKeybindMapper::virtualKeyFromSetting("pauseGameKey");

		if      (firstPlayerUpKey    && virtualKey ==* firstPlayerUpKey)    handleButton(wParam, Up,    true);
		else if (firstPlayerLeftKey  && virtualKey ==* firstPlayerLeftKey)  handleButton(wParam, Left,  true);
		else if (firstPlayerRightKey && virtualKey ==* firstPlayerRightKey) handleButton(wParam, Right, true);
		else if (secondPlayerUpKey   && virtualKey ==* secondPlayerUpKey)   handleButton(wParam, Up,    false);
		else if (secondPlayerLeftKey && virtualKey ==* secondPlayerLeftKey) handleButton(wParam, Left,  false);
		else if (secondPlayerRightKey && virtualKey ==* secondPlayerRightKey) handleButton(wParam, Right, false);

		if (pauseGameKey && virtualKey == static_cast<DWORD>(*pauseGameKey) && wParam == WM_KEYDOWN) {
			queueInMainThread([scene] {
				if (const auto playLayer = PlayLayer::get()) {
					if (!playLayer->m_isPaused) {
						playLayer->pauseGame(false);
					} else {
						playLayer->resume();
						auto const pauseLayer = scene->getChildByType<PauseLayer>(0);
						pauseLayer->onResume(nullptr);
					}
				}
			});
		}
	}

	return CallNextHookEx(gHook, code, wParam, lParam);
}

class $modify(PlayLayer) {
	void resetLevel() {
		PlayLayer::resetLevel();
		gPressedKeys.clear();
	}
};

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