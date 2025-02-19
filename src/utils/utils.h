#pragma once

#include <memory>
#include <string>

#include <cstdint>

extern uint8_t SRGBComponentToRGBTable[];
extern uint8_t RGBComponentToSRGBTable[];

inline uint8_t SRGBComponentToRGB(uint8_t ci) {
    return SRGBComponentToRGBTable[ci];
}

inline uint8_t RGBComponentToSRGB(uint8_t ci) {
    return RGBComponentToSRGBTable[ci];
}

std::string GetSanitizedNameOfCurrentApplication();

void ApplyGameSpecificPatches();

void InitNotificationModule();