#include "displayHandler.hpp"
#include <Arduino.h>
#include "Osptek_BWR_42.h"
#include "fonts.hpp"
#include "Osptek_BWR_42.h"
#include "displayUtils.hpp"
// #include "SparkFunLIS3DH.h"
#include "Wire.h"
#include "SPI.h"
#include "fonts.hpp"

// Shape 1 (Red Box) Variables
int boxSize = 40;
int boxX = 50, boxY = 220; // Adjusted to start in the lower section
int boxDx = 5, boxDy = 6;

// Shape 2 (Black Circle) Variables
int radius = 25;
int circleX = 200, circleY = 300;
int circleDx = -7, circleDy = -5;

int updateCounter = 0;
unsigned long lastUpdate = 0;

void DisplayHandler::convertValueToString(char *valueStr, SensorValue value)
{
    std::visit([&valueStr](const auto &arg)
               {
        using T = std::decay_t<decltype(arg)>;

        if constexpr (std::is_same_v<T, float>) {
            dtostrf(arg, 1, 1, valueStr);
        } 
        else if constexpr (std::is_same_v<T, int>) {
            snprintf(valueStr, sizeof(valueStr), "%d", arg);
        } 
        else if constexpr (std::is_same_v<T, const char*>) {
            snprintf(valueStr, sizeof(valueStr), "%s", arg);
        } }, value);
}

DisplayHandler::DisplayHandler(InstrumentDataModel &dataModel) : dataModel(dataModel), lcd(LCD_CS, LCD_DC, LCD_RES, -1, -1), currentLayout(DisplayLayout::ThreeColTwoRow)
{
}

void DisplayHandler::init()
{
    lcd.begin(false);
    lcd.clear(COLOR_NEUTRAL);
    lcd.update();
}

void DisplayHandler::updateDisplay()
{
    lastUpdate = millis();
    lcd.clear(COLOR_NEUTRAL);
    DrawLayout();
    lcd.update();
    updateCounter++;
}

void DisplayHandler::DrawLayout()
{
    switch (currentLayout)
    {
    case DisplayLayout::ThreeColTwoRow:
    {
        lcd.drawRect(0, 200, 300, 1, COLOR_BLACK);
        lcd.drawRect(0, 266, 300, 1, COLOR_BLACK);
        lcd.drawRect(0, 332, 300, 1, COLOR_BLACK);

        lcd.drawRect(149, 200, 1, 200, COLOR_BLACK); // Middle border (0 + 2 + 147)

        // Call the method and provide the display logic as the callback
        int lowerDataCount = 0;
        dataModel.drawActiveSensors([&](int index, const char *title, SensorValue value, SensorUnit unit, SensorId id)
                                    {
                                        const char *unitText = unit.GetString();
                                        int valueLen = strlen(unitText);
                                        char valueStr[32];
                                        convertValueToString(valueStr, value);
                                        if (id == focusedSensor)
                                        {
                                            const int top_margin = 10;
                                            const int side_margin = 10;
                                            DisplayUtils::DrawTextCentered(lcd, -10, valueStr, COLOR_BLACK, COLOR_NEUTRAL, DisplayUtils::TextFont::SimplyMono_Bold50pt7b, 1);
                                            DisplayUtils::DrawText(lcd, DISPLAY_WIDTH - (valueLen * 12 + side_margin), top_margin, unitText, COLOR_RED, COLOR_NEUTRAL, DisplayUtils::TextFont::David_Sans8pt7b, 1);
                                            DisplayUtils::DrawText(lcd, side_margin, top_margin, title, COLOR_BLACK, COLOR_NEUTRAL, DisplayUtils::TextFont::David_Sans8pt7b, 1);
                                        }
                                        else
                                        {
                                            // Calculate the row (0 to 2) and column (0 to 1) based on the index
                                            int row = lowerDataCount / 2;
                                            int col = lowerDataCount % 2;

                                            // Calculate the top-left X and Y coordinates for the current cell
                                            int cellX = (col * 149) + 5;
                                            int cellY = 200 + (row * 66) + 5;

                                            DisplayUtils::DrawText(lcd, cellX, cellY, title, COLOR_BLACK, COLOR_NEUTRAL, DisplayUtils::TextFont::pf_ronda_seven8pt7b, 2);

                                            DisplayUtils::DrawText(lcd, cellX+10, cellY + 38, valueStr, COLOR_BLACK, COLOR_NEUTRAL, DisplayUtils::TextFont::David_Sans8pt7b, 2);

                                            int unitX = cellX + 149 - (valueLen * 12);

                                            // 5. Draw the Unit Text (Size 1 or 2)
                                            DisplayUtils::DrawText(lcd, unitX, cellY, unitText, COLOR_RED, COLOR_NEUTRAL, DisplayUtils::TextFont::pf_ronda_seven8pt7b, 2);
                                            ++lowerDataCount;
                                        }
                                    }); // End of callback
        break;
    }
    default:
        break;
    }
}
