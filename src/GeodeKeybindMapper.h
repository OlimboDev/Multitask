#pragma once

#include <Geode/Geode.hpp>
#include <Geode/loader/SettingV3.hpp>
#include <Geode/loader/GameEvent.hpp>
#include <optional>

using namespace geode::prelude;

class GeodeKeybindMapper final {
public:
    struct KeyPair {
        enumKeyCodes geodeKey;
        int vkCode;
    };

    // Every cocos2d key that has a reasonable Windows VK equivalent.
    static inline const KeyPair s_keyTable[] = {
        {KEY_Backspace, VK_BACK},
        {KEY_Tab, VK_TAB},
        {KEY_Enter, VK_RETURN},
        {KEY_CapsLock, VK_CAPITAL},
        {KEY_Escape, VK_ESCAPE},
        {KEY_Space, VK_SPACE},
        {KEY_Insert, VK_INSERT},
        {KEY_Home, VK_HOME},
        {KEY_PageUp, VK_PRIOR},
        {KEY_Delete, VK_DELETE},
        {KEY_End, VK_END},
        {KEY_PageDown, VK_NEXT},

        {KEY_Left, VK_LEFT},
        {KEY_Right, VK_RIGHT},
        {KEY_Up, VK_UP},
        {KEY_Down, VK_DOWN},

        {KEY_Shift, VK_SHIFT},
        {KEY_Control, VK_CONTROL},
        {KEY_Alt, VK_MENU},

        {KEY_Pause, VK_PAUSE},
        {KEY_ScrollLock, VK_SCROLL},
        {KEY_PrintScreen, VK_SNAPSHOT},

        {KEY_NumPad0, VK_NUMPAD0},
        {KEY_NumPad1, VK_NUMPAD1},
        {KEY_NumPad2, VK_NUMPAD2},
        {KEY_NumPad3, VK_NUMPAD3},
        {KEY_NumPad4, VK_NUMPAD4},
        {KEY_NumPad5, VK_NUMPAD5},
        {KEY_NumPad6, VK_NUMPAD6},
        {KEY_NumPad7, VK_NUMPAD7},
        {KEY_NumPad8, VK_NUMPAD8},
        {KEY_NumPad9, VK_NUMPAD9},
        {KEY_Multiply, VK_MULTIPLY},
        {KEY_Add, VK_ADD},
        {KEY_Subtract, VK_SUBTRACT},
        {KEY_Decimal, VK_DECIMAL},
        {KEY_Divide, VK_DIVIDE},

        {KEY_F1, VK_F1},
        {KEY_F2, VK_F2},
        {KEY_F3, VK_F3},
        {KEY_F4, VK_F4},
        {KEY_F5, VK_F5},
        {KEY_F6, VK_F6},
        {KEY_F7, VK_F7},
        {KEY_F8, VK_F8},
        {KEY_F9, VK_F9},
        {KEY_F10, VK_F10},
        {KEY_F11, VK_F11},
        {KEY_F12, VK_F12},

        {KEY_Zero, 0x30},
        {KEY_One, 0x31},
        {KEY_Two, 0x32},
        {KEY_Three, 0x33},
        {KEY_Four, 0x34},
        {KEY_Five, 0x35},
        {KEY_Six, 0x36},
        {KEY_Seven, 0x37},
        {KEY_Eight, 0x38},
        {KEY_Nine, 0x39},

        {KEY_A, 0x41},
        {KEY_B, 0x42},
        {KEY_C, 0x43},
        {KEY_D, 0x44},
        {KEY_E, 0x45},
        {KEY_F, 0x46},
        {KEY_G, 0x47},
        {KEY_H, 0x48},
        {KEY_I, 0x49},
        {KEY_J, 0x4A},
        {KEY_K, 0x4B},
        {KEY_L, 0x4C},
        {KEY_M, 0x4D},
        {KEY_N, 0x4E},
        {KEY_O, 0x4F},
        {KEY_P, 0x50},
        {KEY_Q, 0x51},
        {KEY_R, 0x52},
        {KEY_S, 0x53},
        {KEY_T, 0x54},
        {KEY_U, 0x55},
        {KEY_V, 0x56},
        {KEY_W, 0x57},
        {KEY_X, 0x58},
        {KEY_Y, 0x59},
        {KEY_Z, 0x5A},

        {KEY_Semicolon, VK_OEM_1},
        {KEY_Slash, VK_OEM_2},
        {KEY_GraveAccent, VK_OEM_3},
        {KEY_LeftBracket, VK_OEM_4},
        {KEY_RightBracket, VK_OEM_6},
        {KEY_Apostrophe, VK_OEM_7},
    };


    static std::optional<int> virtualKeyFromSetting(std::string const& setting) {
        auto const keyBind = Mod::get()->getSettingValue<std::vector<Keybind>>(setting);
        if (keyBind.empty()) return std::nullopt;

        // Retrieve the very last key within the bind array (for jokers using something like "CTRL+SHIFT+J" WHEN WE SAID IT'S NOT SUPPORTED)
        enumKeyCodes const key = keyBind.back().key;

        for (auto const [geodeKey, vkCode]: s_keyTable) {
            if (geodeKey == key) return vkCode;
        }

        return std::nullopt;
    }
};
