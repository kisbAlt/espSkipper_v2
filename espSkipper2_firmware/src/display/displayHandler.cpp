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

void DisplayHandler::convertValueToString(char* valueStr, SensorValue value)
{
    std::visit([&valueStr](const auto& arg) {
        using T = std::decay_t<decltype(arg)>;

        if constexpr (std::is_same_v<T, float>) {
            dtostrf(arg, 1, 1, valueStr);
        } 
        else if constexpr (std::is_same_v<T, int>) {
            snprintf(valueStr, sizeof(valueStr), "%d", arg);
        } 
        else if constexpr (std::is_same_v<T, const char*>) {
            snprintf(valueStr, sizeof(valueStr), "%s", arg);
        }
    }, value);
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

void DisplayHandler::updateDisplay(const DisplayDataEntity datapoints[10], const int dataCount)
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
        lcd.drawRect(0, 398, 300, 1, COLOR_BLACK);

        // 2. Draw Vertical Lines (Width = 2, Height = 200)
        lcd.drawRect(0, 200, 1, 200, COLOR_BLACK);   // Left border
        lcd.drawRect(149, 200, 1, 200, COLOR_BLACK); // Middle border (0 + 2 + 147)
        lcd.drawRect(298, 200, 1, 200, COLOR_BLACK); // Right border (149 + 2 + 147)

        // Call the method and provide the display logic as the callback
        dataModel.drawActiveSensors([&](int index, const char *title, SensorValue value, SensorUnit unit)
                                    {
                                        // Calculate the row (0 to 2) and column (0 to 1) based on the index
                                        int row = index / 2;
                                        int col = index % 2;

                                        // Calculate the top-left X and Y coordinates for the current cell
                                        int cellX = (col * 149) + 5;
                                        int cellY = 200 + (row * 66) + 5;

                                        DisplayUtils::DrawText(lcd, cellX, cellY, title, COLOR_BLACK, COLOR_NEUTRAL, 2);

                                        char valueStr[32];
                                        convertValueToString(valueStr, value);

                                        DisplayUtils::DrawText(lcd, cellX, cellY + 25, valueStr, COLOR_BLACK, COLOR_NEUTRAL, 4);

                                        const char *unitText = "test";
                                        int valueLen = strlen(unitText);
                                        int unitX = cellX + 149 - (valueLen * 12);

                                        // 5. Draw the Unit Text (Size 1 or 2)
                                        DisplayUtils::DrawText(lcd, unitX, cellY, unitText, COLOR_RED, COLOR_NEUTRAL, 2); }); // End of callback
        break;
    default:
        break;
    }
    }
}
