#pragma once

#include <cstdint>
#include <padscore/kpad.h>
#include <vpad/input.h>


uint32_t remapVPADtoWiimote(uint32_t buttons);
uint32_t remapVPADtoClassic(uint32_t buttons);
uint32_t remapVPADtoPro(uint32_t buttons);

uint32_t remapWiiMoteButtons(uint32_t buttons);
uint32_t remapClassicButtons(uint32_t buttons);
uint32_t remapProButtons(uint32_t buttons);