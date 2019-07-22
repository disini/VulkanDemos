﻿#include "GenericPlatform/InputManager.h"

void InputManager::Init()
{
	s_KeyboardTypesMap[0x00B] = KeyboardType::KEY_0;
    s_KeyboardTypesMap[0x002] = KeyboardType::KEY_1;
    s_KeyboardTypesMap[0x003] = KeyboardType::KEY_2;
    s_KeyboardTypesMap[0x004] = KeyboardType::KEY_3;
    s_KeyboardTypesMap[0x005] = KeyboardType::KEY_4;
    s_KeyboardTypesMap[0x006] = KeyboardType::KEY_5;
    s_KeyboardTypesMap[0x007] = KeyboardType::KEY_6;
    s_KeyboardTypesMap[0x008] = KeyboardType::KEY_7;
    s_KeyboardTypesMap[0x009] = KeyboardType::KEY_8;
    s_KeyboardTypesMap[0x00A] = KeyboardType::KEY_9;
    s_KeyboardTypesMap[0x01E] = KeyboardType::KEY_A;
    s_KeyboardTypesMap[0x030] = KeyboardType::KEY_B;
    s_KeyboardTypesMap[0x02E] = KeyboardType::KEY_C;
    s_KeyboardTypesMap[0x020] = KeyboardType::KEY_D;
    s_KeyboardTypesMap[0x012] = KeyboardType::KEY_E;
    s_KeyboardTypesMap[0x021] = KeyboardType::KEY_F;
    s_KeyboardTypesMap[0x022] = KeyboardType::KEY_G;
    s_KeyboardTypesMap[0x023] = KeyboardType::KEY_H;
    s_KeyboardTypesMap[0x017] = KeyboardType::KEY_I;
    s_KeyboardTypesMap[0x024] = KeyboardType::KEY_J;
    s_KeyboardTypesMap[0x025] = KeyboardType::KEY_K;
    s_KeyboardTypesMap[0x026] = KeyboardType::KEY_L;
    s_KeyboardTypesMap[0x032] = KeyboardType::KEY_M;
    s_KeyboardTypesMap[0x031] = KeyboardType::KEY_N;
    s_KeyboardTypesMap[0x018] = KeyboardType::KEY_O;
    s_KeyboardTypesMap[0x019] = KeyboardType::KEY_P;
    s_KeyboardTypesMap[0x010] = KeyboardType::KEY_Q;
    s_KeyboardTypesMap[0x013] = KeyboardType::KEY_R;
    s_KeyboardTypesMap[0x01F] = KeyboardType::KEY_S;
    s_KeyboardTypesMap[0x014] = KeyboardType::KEY_T;
    s_KeyboardTypesMap[0x016] = KeyboardType::KEY_U;
    s_KeyboardTypesMap[0x02F] = KeyboardType::KEY_V;
    s_KeyboardTypesMap[0x011] = KeyboardType::KEY_W;
    s_KeyboardTypesMap[0x02D] = KeyboardType::KEY_X;
    s_KeyboardTypesMap[0x015] = KeyboardType::KEY_Y;
    s_KeyboardTypesMap[0x02C] = KeyboardType::KEY_Z;

    s_KeyboardTypesMap[0x028] = KeyboardType::KEY_APOSTROPHE;
    s_KeyboardTypesMap[0x02B] = KeyboardType::KEY_BACKSLASH;
    s_KeyboardTypesMap[0x033] = KeyboardType::KEY_COMMA;
    s_KeyboardTypesMap[0x00D] = KeyboardType::KEY_EQUAL;
    s_KeyboardTypesMap[0x029] = KeyboardType::KEY_GRAVE_ACCENT;
    s_KeyboardTypesMap[0x01A] = KeyboardType::KEY_LEFT_BRACKET;
    s_KeyboardTypesMap[0x00C] = KeyboardType::KEY_MINUS;
    s_KeyboardTypesMap[0x034] = KeyboardType::KEY_PERIOD;
    s_KeyboardTypesMap[0x01B] = KeyboardType::KEY_RIGHT_BRACKET;
    s_KeyboardTypesMap[0x027] = KeyboardType::KEY_SEMICOLON;
    s_KeyboardTypesMap[0x035] = KeyboardType::KEY_SLASH;
    s_KeyboardTypesMap[0x056] = KeyboardType::KEY_WORLD_2;

    s_KeyboardTypesMap[0x00E] = KeyboardType::KEY_BACKSPACE;
    s_KeyboardTypesMap[0x153] = KeyboardType::KEY_DELETE;
    s_KeyboardTypesMap[0x14F] = KeyboardType::KEY_END;
    s_KeyboardTypesMap[0x01C] = KeyboardType::KEY_ENTER;
    s_KeyboardTypesMap[0x001] = KeyboardType::KEY_ESCAPE;
    s_KeyboardTypesMap[0x147] = KeyboardType::KEY_HOME;
    s_KeyboardTypesMap[0x152] = KeyboardType::KEY_INSERT;
    s_KeyboardTypesMap[0x15D] = KeyboardType::KEY_MENU;
    s_KeyboardTypesMap[0x151] = KeyboardType::KEY_PAGE_DOWN;
    s_KeyboardTypesMap[0x149] = KeyboardType::KEY_PAGE_UP;
    s_KeyboardTypesMap[0x045] = KeyboardType::KEY_PAUSE;
    s_KeyboardTypesMap[0x146] = KeyboardType::KEY_PAUSE;
    s_KeyboardTypesMap[0x039] = KeyboardType::KEY_SPACE;
    s_KeyboardTypesMap[0x00F] = KeyboardType::KEY_TAB;
    s_KeyboardTypesMap[0x03A] = KeyboardType::KEY_CAPS_LOCK;
    s_KeyboardTypesMap[0x145] = KeyboardType::KEY_NUM_LOCK;
    s_KeyboardTypesMap[0x046] = KeyboardType::KEY_SCROLL_LOCK;
    s_KeyboardTypesMap[0x03B] = KeyboardType::KEY_F1;
    s_KeyboardTypesMap[0x03C] = KeyboardType::KEY_F2;
    s_KeyboardTypesMap[0x03D] = KeyboardType::KEY_F3;
    s_KeyboardTypesMap[0x03E] = KeyboardType::KEY_F4;
    s_KeyboardTypesMap[0x03F] = KeyboardType::KEY_F5;
    s_KeyboardTypesMap[0x040] = KeyboardType::KEY_F6;
    s_KeyboardTypesMap[0x041] = KeyboardType::KEY_F7;
    s_KeyboardTypesMap[0x042] = KeyboardType::KEY_F8;
    s_KeyboardTypesMap[0x043] = KeyboardType::KEY_F9;
    s_KeyboardTypesMap[0x044] = KeyboardType::KEY_F10;
    s_KeyboardTypesMap[0x057] = KeyboardType::KEY_F11;
    s_KeyboardTypesMap[0x058] = KeyboardType::KEY_F12;
    s_KeyboardTypesMap[0x064] = KeyboardType::KEY_F13;
    s_KeyboardTypesMap[0x065] = KeyboardType::KEY_F14;
    s_KeyboardTypesMap[0x066] = KeyboardType::KEY_F15;
    s_KeyboardTypesMap[0x067] = KeyboardType::KEY_F16;
    s_KeyboardTypesMap[0x068] = KeyboardType::KEY_F17;
    s_KeyboardTypesMap[0x069] = KeyboardType::KEY_F18;
    s_KeyboardTypesMap[0x06A] = KeyboardType::KEY_F19;
    s_KeyboardTypesMap[0x06B] = KeyboardType::KEY_F20;
    s_KeyboardTypesMap[0x06C] = KeyboardType::KEY_F21;
    s_KeyboardTypesMap[0x06D] = KeyboardType::KEY_F22;
    s_KeyboardTypesMap[0x06E] = KeyboardType::KEY_F23;
    s_KeyboardTypesMap[0x076] = KeyboardType::KEY_F24;
    s_KeyboardTypesMap[0x038] = KeyboardType::KEY_LEFT_ALT;
    s_KeyboardTypesMap[0x01D] = KeyboardType::KEY_LEFT_CONTROL;
    s_KeyboardTypesMap[0x02A] = KeyboardType::KEY_LEFT_SHIFT;
    s_KeyboardTypesMap[0x15B] = KeyboardType::KEY_LEFT_SUPER;
    s_KeyboardTypesMap[0x137] = KeyboardType::KEY_PRINT_SCREEN;
    s_KeyboardTypesMap[0x138] = KeyboardType::KEY_RIGHT_ALT;
    s_KeyboardTypesMap[0x11D] = KeyboardType::KEY_RIGHT_CONTROL;
    s_KeyboardTypesMap[0x036] = KeyboardType::KEY_RIGHT_SHIFT;
    s_KeyboardTypesMap[0x15C] = KeyboardType::KEY_RIGHT_SUPER;
    s_KeyboardTypesMap[0x150] = KeyboardType::KEY_DOWN;
    s_KeyboardTypesMap[0x14B] = KeyboardType::KEY_LEFT;
    s_KeyboardTypesMap[0x14D] = KeyboardType::KEY_RIGHT;
    s_KeyboardTypesMap[0x148] = KeyboardType::KEY_UP;

    s_KeyboardTypesMap[0x052] = KeyboardType::KEY_KP_0;
    s_KeyboardTypesMap[0x04F] = KeyboardType::KEY_KP_1;
    s_KeyboardTypesMap[0x050] = KeyboardType::KEY_KP_2;
    s_KeyboardTypesMap[0x051] = KeyboardType::KEY_KP_3;
    s_KeyboardTypesMap[0x04B] = KeyboardType::KEY_KP_4;
    s_KeyboardTypesMap[0x04C] = KeyboardType::KEY_KP_5;
    s_KeyboardTypesMap[0x04D] = KeyboardType::KEY_KP_6;
    s_KeyboardTypesMap[0x047] = KeyboardType::KEY_KP_7;
    s_KeyboardTypesMap[0x048] = KeyboardType::KEY_KP_8;
    s_KeyboardTypesMap[0x049] = KeyboardType::KEY_KP_9;
    s_KeyboardTypesMap[0x04E] = KeyboardType::KEY_KP_ADD;
    s_KeyboardTypesMap[0x053] = KeyboardType::KEY_KP_DECIMAL;
    s_KeyboardTypesMap[0x135] = KeyboardType::KEY_KP_DIVIDE;
    s_KeyboardTypesMap[0x11C] = KeyboardType::KEY_KP_ENTER;
    s_KeyboardTypesMap[0x037] = KeyboardType::KEY_KP_MULTIPLY;
    s_KeyboardTypesMap[0x04A] = KeyboardType::KEY_KP_SUBTRACT;
}