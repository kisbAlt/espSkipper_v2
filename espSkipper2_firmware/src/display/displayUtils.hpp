#pragma once
#include "Osptek_BWR_42.h"
#include "fonts.hpp"
enum class DisplayLayout
{
    ThreeColTwoRow
};

struct DisplayDataEntity
{
    char *title;
    char *unitText;
    float value;
    int id;
};

class DisplayUtils
{
public:
    static void DrawChar(OsptekBWR &lcd, const int16_t x, const int16_t y, const char c, const OspColor color, const OspColor bg, const uint8_t size = 1)
    {
        if (c < 32 || c > 126)
            return; // Drop characters outside standard ASCII

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
                { // Render background if it's different from the text color
                    if (size == 1)
                        lcd.drawPixel(x + i, y + j, bg);
                    else
                        lcd.fillRect(x + i * size, y + j * size, size, size, bg);
                }
            }
        }
    }
    static void DrawText(OsptekBWR &lcd, const int x, const int y, const char *text, const OspColor color, const OspColor bg, const uint8_t size = 1)
    {
        int16_t cursor_x = x;
        while (*text)
        {
            DrawChar(lcd, cursor_x, y, *text++, color, bg, size);
            cursor_x += (6 * size); // Move to the next character position (5 pixels wide + 1 pixel space)
        }
    }

    static void drawTextCentered(OsptekBWR &lcd, int cellX, int cellY, int cellW, int cellH, const char *text, OspColor color, OspColor bg, uint8_t size = 1)
    {
        // Standard character dimensions for size 1
        int charWidth = 6 * size;  // 5px font + 1px space
        int charHeight = 8 * size; // 8px font height

        int textWidth = strlen(text) * charWidth;
        int textHeight = charHeight;

        // Calculate top-left starting position to center the text box inside the cell
        int targetX = cellX + (cellW - textWidth) / 2;
        int targetY = cellY + (cellH - textHeight) / 2;

        // Call your library function
        DrawText(lcd, targetX, targetY, text, color, bg, size);
    }

    static void DrawLayout(OsptekBWR &lcd, const DisplayDataEntity datapoints[10], const int dataCount, const DisplayLayout layout)
    {
        
    }
};