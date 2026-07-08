#pragma once
#include "Osptek_BWR_42.h"
#include "fonts.hpp"

class DisplayUtils
{
public:
    enum class TextFont
    {
        Font5x7,
        David_Sans8pt7b,
        pf_ronda_seven8pt7b,
        M2020pt7b,
        SimplyMono_Bold50pt7b
    };

    static void DrawChar(OsptekBWR &lcd, const int16_t x, const int16_t y, const char c, const OspColor color, const OspColor bg, const uint8_t size = 1);
    static int16_t DrawCharGFX(OsptekBWR &lcd, int16_t x, int16_t y, unsigned char c, const OspColor color, const OspColor bg, const GFXfont *gfxFont, uint8_t size = 1);
    static void DrawStringGFX(OsptekBWR &lcd, int16_t x, int16_t y, const char *str, const OspColor color, const OspColor bg, const GFXfont *gfxFont, uint8_t size = 1);
    static const GFXfont* GetFont(TextFont textFont);
    static void DrawText(OsptekBWR &lcd, const int x, const int y, const char *text, const OspColor color, const OspColor bg, const TextFont textFont, const uint8_t size = 1);
    static void DrawTextCentered(OsptekBWR &lcd, const int y, const char *text, const OspColor color, const OspColor bg, const TextFont textFont, const uint8_t size = 1);
};