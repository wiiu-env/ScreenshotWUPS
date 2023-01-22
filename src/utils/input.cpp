#include "input.h"

uint32_t remapVPADtoWiimote(uint32_t buttons) {
    uint32_t conv_buttons = 0;
    if (buttons & VPAD_BUTTON_A) {
        conv_buttons |= WPAD_BUTTON_A;
    }
    if (buttons & VPAD_BUTTON_B) {
        conv_buttons |= WPAD_BUTTON_B;
    }
    if (buttons & VPAD_BUTTON_UP) {
        conv_buttons |= WPAD_BUTTON_UP;
    }
    if (buttons & VPAD_BUTTON_DOWN) {
        conv_buttons |= WPAD_BUTTON_DOWN;
    }
    if (buttons & VPAD_BUTTON_LEFT) {
        conv_buttons |= WPAD_BUTTON_LEFT;
    }
    if (buttons & VPAD_BUTTON_RIGHT) {
        conv_buttons |= WPAD_BUTTON_RIGHT;
    }
    if (buttons & VPAD_BUTTON_PLUS) {
        conv_buttons |= WPAD_BUTTON_PLUS;
    }
    if (buttons & VPAD_BUTTON_MINUS) {
        conv_buttons |= WPAD_BUTTON_MINUS;
    }
    return conv_buttons;
}


uint32_t remapVPADtoClassic(uint32_t buttons) {
    uint32_t conv_buttons = 0;
    if (buttons & VPAD_BUTTON_A) {
        conv_buttons |= WPAD_CLASSIC_BUTTON_A;
    }
    if (buttons & VPAD_BUTTON_B) {
        conv_buttons |= WPAD_CLASSIC_BUTTON_B;
    }
    if (buttons & VPAD_BUTTON_X) {
        conv_buttons |= WPAD_CLASSIC_BUTTON_X;
    }
    if (buttons & VPAD_BUTTON_Y) {
        conv_buttons |= WPAD_CLASSIC_BUTTON_Y;
    }
    if (buttons & VPAD_BUTTON_L) {
        conv_buttons |= WPAD_CLASSIC_BUTTON_L;
    }
    if (buttons & VPAD_BUTTON_R) {
        conv_buttons |= WPAD_CLASSIC_BUTTON_R;
    }
    if (buttons & VPAD_BUTTON_ZL) {
        conv_buttons |= WPAD_CLASSIC_BUTTON_ZL;
    }
    if (buttons & VPAD_BUTTON_ZR) {
        conv_buttons |= WPAD_CLASSIC_BUTTON_ZR;
    }
    if (buttons & VPAD_BUTTON_UP) {
        conv_buttons |= WPAD_CLASSIC_BUTTON_UP;
    }
    if (buttons & VPAD_BUTTON_DOWN) {
        conv_buttons |= WPAD_CLASSIC_BUTTON_DOWN;
    }
    if (buttons & VPAD_BUTTON_LEFT) {
        conv_buttons |= WPAD_CLASSIC_BUTTON_LEFT;
    }
    if (buttons & VPAD_BUTTON_RIGHT) {
        conv_buttons |= WPAD_CLASSIC_BUTTON_RIGHT;
    }
    if (buttons & VPAD_BUTTON_PLUS) {
        conv_buttons |= WPAD_CLASSIC_BUTTON_PLUS;
    }
    if (buttons & VPAD_BUTTON_MINUS) {
        conv_buttons |= WPAD_CLASSIC_BUTTON_MINUS;
    }
    return conv_buttons;
}

uint32_t remapVPADtoPro(uint32_t buttons) {
    uint32_t conv_buttons = 0;
    if (buttons & VPAD_BUTTON_A) {
        conv_buttons |= WPAD_PRO_BUTTON_A;
    }
    if (buttons & VPAD_BUTTON_B) {
        conv_buttons |= WPAD_PRO_BUTTON_B;
    }
    if (buttons & VPAD_BUTTON_X) {
        conv_buttons |= WPAD_PRO_BUTTON_X;
    }
    if (buttons & VPAD_BUTTON_Y) {
        conv_buttons |= WPAD_PRO_BUTTON_Y;
    }
    if (buttons & VPAD_BUTTON_L) {
        conv_buttons |= WPAD_PRO_TRIGGER_L;
    }
    if (buttons & VPAD_BUTTON_R) {
        conv_buttons |= WPAD_PRO_TRIGGER_R;
    }
    if (buttons & VPAD_BUTTON_ZL) {
        conv_buttons |= WPAD_PRO_TRIGGER_ZL;
    }
    if (buttons & VPAD_BUTTON_ZR) {
        conv_buttons |= WPAD_PRO_TRIGGER_ZR;
    }
    if (buttons & VPAD_BUTTON_UP) {
        conv_buttons |= WPAD_PRO_BUTTON_UP;
    }
    if (buttons & VPAD_BUTTON_DOWN) {
        conv_buttons |= WPAD_PRO_BUTTON_DOWN;
    }
    if (buttons & VPAD_BUTTON_LEFT) {
        conv_buttons |= WPAD_PRO_BUTTON_LEFT;
    }
    if (buttons & VPAD_BUTTON_RIGHT) {
        conv_buttons |= WPAD_PRO_BUTTON_RIGHT;
    }
    if (buttons & VPAD_BUTTON_PLUS) {
        conv_buttons |= WPAD_PRO_BUTTON_PLUS;
    }
    if (buttons & VPAD_BUTTON_MINUS) {
        conv_buttons |= WPAD_PRO_BUTTON_MINUS;
    }
    if (buttons & VPAD_BUTTON_RESERVED_BIT) {
        conv_buttons |= WPAD_PRO_RESERVED;
    }
    return conv_buttons;
}

uint32_t remapWiiMoteButtons(uint32_t buttons) {
    uint32_t conv_buttons = 0;

    if (buttons & WPAD_BUTTON_LEFT) {
        conv_buttons |= VPAD_BUTTON_LEFT;
    }

    if (buttons & WPAD_BUTTON_RIGHT) {
        conv_buttons |= VPAD_BUTTON_RIGHT;
    }

    if (buttons & WPAD_BUTTON_DOWN) {
        conv_buttons |= VPAD_BUTTON_DOWN;
    }

    if (buttons & WPAD_BUTTON_UP) {
        conv_buttons |= VPAD_BUTTON_UP;
    }

    if (buttons & WPAD_BUTTON_PLUS) {
        conv_buttons |= VPAD_BUTTON_PLUS;
    }

    if (buttons & WPAD_BUTTON_B) {
        conv_buttons |= VPAD_BUTTON_B;
    }

    if (buttons & WPAD_BUTTON_A) {
        conv_buttons |= VPAD_BUTTON_A;
    }

    if (buttons & WPAD_BUTTON_MINUS) {
        conv_buttons |= VPAD_BUTTON_MINUS;
    }

    if (buttons & WPAD_BUTTON_HOME) {
        conv_buttons |= VPAD_BUTTON_HOME;
    }
    return conv_buttons;
}

uint32_t remapClassicButtons(uint32_t buttons) {
    uint32_t conv_buttons = 0;

    if (buttons & WPAD_CLASSIC_BUTTON_LEFT) {
        conv_buttons |= VPAD_BUTTON_LEFT;
    }
    if (buttons & WPAD_CLASSIC_BUTTON_RIGHT) {
        conv_buttons |= VPAD_BUTTON_RIGHT;
    }
    if (buttons & WPAD_CLASSIC_BUTTON_DOWN) {
        conv_buttons |= VPAD_BUTTON_DOWN;
    }
    if (buttons & WPAD_CLASSIC_BUTTON_UP) {
        conv_buttons |= VPAD_BUTTON_UP;
    }
    if (buttons & WPAD_CLASSIC_BUTTON_PLUS) {
        conv_buttons |= VPAD_BUTTON_PLUS;
    }
    if (buttons & WPAD_CLASSIC_BUTTON_X) {
        conv_buttons |= VPAD_BUTTON_X;
    }
    if (buttons & WPAD_CLASSIC_BUTTON_Y) {
        conv_buttons |= VPAD_BUTTON_Y;
    }
    if (buttons & WPAD_CLASSIC_BUTTON_B) {
        conv_buttons |= VPAD_BUTTON_B;
    }
    if (buttons & WPAD_CLASSIC_BUTTON_A) {
        conv_buttons |= VPAD_BUTTON_A;
    }
    if (buttons & WPAD_CLASSIC_BUTTON_MINUS) {
        conv_buttons |= VPAD_BUTTON_MINUS;
    }
    if (buttons & WPAD_CLASSIC_BUTTON_HOME) {
        conv_buttons |= VPAD_BUTTON_HOME;
    }
    if (buttons & WPAD_CLASSIC_BUTTON_ZR) {
        conv_buttons |= VPAD_BUTTON_ZR;
    }
    if (buttons & WPAD_CLASSIC_BUTTON_ZL) {
        conv_buttons |= VPAD_BUTTON_ZL;
    }
    if (buttons & WPAD_CLASSIC_BUTTON_R) {
        conv_buttons |= VPAD_BUTTON_R;
    }
    if (buttons & WPAD_CLASSIC_BUTTON_L) {
        conv_buttons |= VPAD_BUTTON_L;
    }
    return conv_buttons;
}

uint32_t remapProButtons(uint32_t buttons) {
    uint32_t conv_buttons = 0;

    if (buttons & WPAD_PRO_BUTTON_LEFT) {
        conv_buttons |= VPAD_BUTTON_LEFT;
    }
    if (buttons & WPAD_PRO_BUTTON_RIGHT) {
        conv_buttons |= VPAD_BUTTON_RIGHT;
    }
    if (buttons & WPAD_PRO_BUTTON_DOWN) {
        conv_buttons |= VPAD_BUTTON_DOWN;
    }
    if (buttons & WPAD_PRO_BUTTON_UP) {
        conv_buttons |= VPAD_BUTTON_UP;
    }
    if (buttons & WPAD_PRO_BUTTON_PLUS) {
        conv_buttons |= VPAD_BUTTON_PLUS;
    }
    if (buttons & WPAD_PRO_BUTTON_X) {
        conv_buttons |= VPAD_BUTTON_X;
    }
    if (buttons & WPAD_PRO_BUTTON_Y) {
        conv_buttons |= VPAD_BUTTON_Y;
    }
    if (buttons & WPAD_PRO_BUTTON_B) {
        conv_buttons |= VPAD_BUTTON_B;
    }
    if (buttons & WPAD_PRO_BUTTON_A) {
        conv_buttons |= VPAD_BUTTON_A;
    }
    if (buttons & WPAD_PRO_BUTTON_MINUS) {
        conv_buttons |= VPAD_BUTTON_MINUS;
    }
    if (buttons & WPAD_PRO_BUTTON_HOME) {
        conv_buttons |= VPAD_BUTTON_HOME;
    }
    if (buttons & WPAD_PRO_TRIGGER_ZR) {
        conv_buttons |= VPAD_BUTTON_ZR;
    }
    if (buttons & WPAD_PRO_TRIGGER_ZL) {
        conv_buttons |= VPAD_BUTTON_ZL;
    }
    if (buttons & WPAD_PRO_TRIGGER_R) {
        conv_buttons |= VPAD_BUTTON_R;
    }
    if (buttons & WPAD_PRO_TRIGGER_L) {
        conv_buttons |= VPAD_BUTTON_L;
    }
    if (buttons & WPAD_PRO_BUTTON_STICK_L) {
        conv_buttons |= VPAD_BUTTON_STICK_L;
    }
    if (buttons & WPAD_PRO_BUTTON_STICK_R) {
        conv_buttons |= VPAD_BUTTON_STICK_R;
    }
    if (buttons & WPAD_PRO_RESERVED) {
        conv_buttons |= VPAD_BUTTON_RESERVED_BIT;
    }
    return conv_buttons;
}