#include "displayUtils.hpp"

void DisplayUtils::DrawChar(OsptekBWR &lcd, const int16_t x, const int16_t y, const char c, const OspColor color, const OspColor bg, const uint8_t size)
{
    if (c < 32 || c > 126)
        return; // drop characters outside standard ASCII

    for (int8_t i = 0; i < 5; i++)
    {
        uint8_t line = font5x7[c - 32][i];
        for (int8_t j = 0; j < 8; j++, line >>= 1)
        {
            if (line & 1)
            {
                if (size == 1)
                    lcd.drawPixel(x + i, y + j, color);
                else
                    lcd.fillRect(x + i * size, y + j * size, size, size, color);
            }
            else if (bg != color)
            {
                if (size == 1)
                    lcd.drawPixel(x + i, y + j, bg);
                else
                    lcd.fillRect(x + i * size, y + j * size, size, size, bg);
            }
        }
    }
}

int16_t DisplayUtils::DrawCharGFX(OsptekBWR &lcd, int16_t x, int16_t y, unsigned char c, const OspColor color, const OspColor bg, const GFXfont *gfxFont, uint8_t size) 
{
    if (c < gfxFont->first || c > gfxFont->last) return 0;

    c -= gfxFont->first;
    GFXglyph *glyph  = &(gfxFont->glyph[c]);
    uint8_t  *bitmap = gfxFont->bitmap;

    uint16_t bo = glyph->bitmapOffset;
    uint8_t  w  = glyph->width,
            h  = glyph->height;
    int8_t   xo = glyph->xOffset,
            yo = glyph->yOffset;

    int16_t yBaseline = y + gfxFont->yAdvance; 

    uint8_t  bits = 0, bit = 0;

    for(int16_t yy = 0; yy < h; yy++) {
        for(int16_t xx = 0; xx < w; xx++) {
            if(!(bit++ & 7)) {
                bits = bitmap[bo++];
            }
            if(bits & 0x80) {
                if(size == 1) {
                    lcd.drawPixel(x + xo + xx, yBaseline + yo + yy, color);
                } else {
                    lcd.fillRect(x + (xo + xx) * size, yBaseline + (yo + yy) * size, size, size, color);
                }
            } else if (bg != color) {
                if(size == 1) {
                    lcd.drawPixel(x + xo + xx, yBaseline + yo + yy, bg);
                } else {
                    lcd.fillRect(x + (xo + xx) * size, yBaseline + (yo + yy) * size, size, size, bg);
                }
            }
            bits <<= 1;
        }
    }
    
    // return the distance the cursor needs to move for the next character
    return glyph->xAdvance * size;
}

void DisplayUtils::DrawStringGFX(OsptekBWR &lcd, int16_t x, int16_t y, const char *str, const OspColor color, const OspColor bg, const GFXfont *gfxFont, uint8_t size) 
{
    int16_t cursorX = x;
    while (*str) {
        // draw the character and advance the cursor by its specific width
        cursorX += DrawCharGFX(lcd, cursorX, y, *str, color, bg, gfxFont, size);
        str++;
    }
}

const GFXfont* DisplayUtils::GetFont(TextFont textFont)
{
    switch (textFont)
    {
    case TextFont::M2020pt7b:
        return &m2020pt7b;
    case TextFont::David_Sans8pt7b:
        return &David_Sans8pt7b;
    case TextFont::pf_ronda_seven8pt7b:
        return &pf_ronda_seven4pt7b;
    case TextFont::SimplyMono_Bold50pt7b:
        return &SimplyMono_Bold50pt7b;
    default:
        return nullptr;
    }
} 

void DisplayUtils::DrawText(OsptekBWR &lcd, const int x, const int y, const char *text, const OspColor color, const OspColor bg, const TextFont textFont, const uint8_t size)
{
    const GFXfont *selectedFont = GetFont(textFont);
    if (!selectedFont) {
        return; // Font not found
    }
    DrawStringGFX(lcd, x, y, text, color, bg, selectedFont, size);
}

void DisplayUtils::DrawTextCentered(OsptekBWR &lcd, const int y, const char *text, const OspColor color, const OspColor bg, const TextFont textFont, const uint8_t size)
{
    const GFXfont *selectedFont = GetFont(textFont);
    if (!selectedFont) {
        return; // Font not found
    }

    // calculate the total pixel width of the string
    int16_t stringWidth = 0;
    const char *str = text;
    while (*str)
    {
        unsigned char c = *str;
        if (c >= selectedFont->first && c <= selectedFont->last)
        {
            c -= selectedFont->first;
            GFXglyph *glyph = &(selectedFont->glyph[c]);
            stringWidth += glyph->xAdvance * size;
        }
        str++;
    }

    // calculate the centered X coordinate for a 300px display
    int16_t startX = (DISPLAY_WIDTH - stringWidth) / 2;
    
    // if the string is wider than the display, start at 0
    if (startX < 0) {
        startX = 0;
    }
    DrawStringGFX(lcd, startX, y, text, color, bg, selectedFont, size);
}