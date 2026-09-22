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
#include "settingsHandler.hpp"
#include "bitmaps.h"

// Shape 1 (Red Box) Variables
int boxSize = 40;
int boxX = 50, boxY = 220; // Adjusted to start in the lower section
int boxDx = 5, boxDy = 6;

// Shape 2 (Black Circle) Variables
int radius = 25;
int circleX = 200, circleY = 300;
int circleDx = -7, circleDy = -5;

uint updateCounter = 0;
unsigned long lastUpdate = 0;

int countDigits4(int x)
{
    int n = (x < 0) ? -x : x;

    if (n < 100)
    {
        return (n < 10) ? 1 : 2;
    }
    else
    {
        return (n < 1000) ? 3 : 4;
    }
}

void DisplayHandler::convertValueToString(char *valueStr, int len, SensorValue value, SensorId id)
{
    std::visit([&valueStr, len, id](const auto &arg)
               {
        using T = std::decay_t<decltype(arg)>;

        if constexpr (std::is_same_v<T, float>) {
            dtostrf(arg, 1, 1, valueStr);
        } 
        else if constexpr (std::is_same_v<T, int>) {
            if(id == SensorId::WindDirection){
                snprintf(valueStr, len, "%03d", arg);
            } else {
                snprintf(valueStr, len, "%d", arg);
            }
            
        } 
        else if constexpr (std::is_same_v<T, SensorValueString>) {
            snprintf(valueStr, len, "%s", arg.text);
        } }, value);
}

int DisplayHandler::currentSensorDrawn() const
{
    switch (currentLayout)
    {
    case DisplayLayout::ThreeColTwoRow:
    {
        return 7;
    }
    }
    return 0;
}

DisplayHandler::DisplayHandler(InstrumentDataModel &dataModel, const Settings &settings) : dataModel(dataModel), settings(settings),
                                                                                           lcd(LCD_CS, LCD_DC, LCD_RES, -1, -1), currentLayout(DisplayLayout::ThreeColTwoRow)
{
}

void DisplayHandler::init()
{
    lcd.begin(true);
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

        if (settings.getValue<bool>(SettingsKey::DisplayDateTime))
        {
            char dateStr[32];
            int hour = std::get<int>(dataModel.getSensorValue(SensorId::DateTimeHour));
            int minute = std::get<int>(dataModel.getSensorValue(SensorId::DateTimeMinute));
            if (settings.getValue<bool>(SettingsKey::DisplayTimeOnly))
            {
                snprintf(dateStr, sizeof(dateStr), "%02d:%02d", hour, minute);
            }
            else
            {
                int year = std::get<int>(dataModel.getSensorValue(SensorId::DateTimeYear));
                int month = std::get<int>(dataModel.getSensorValue(SensorId::DateTimeMonth));
                int day = std::get<int>(dataModel.getSensorValue(SensorId::DateTimeDay));
                snprintf(dateStr, sizeof(dateStr), "%04d.%02d.%02d %02d:%02d", year, month, day, hour, minute);
            }
            DisplayUtils::DrawTextCentered(lcd, 180, dateStr, COLOR_BLACK, COLOR_NEUTRAL, DisplayUtils::TextFont::David_Sans8pt7b, 1);
        }

        // Call the method and provide the display logic as the callback
        int lowerDataCount = 0;
        dataModel.drawActiveSensors([&](int index, const TextKey titleKey, SensorValue value, const char *unitText, SensorId id)
                                    {
                                        int valueLen = strlen(unitText);
                                        char valueStr[32];
                                        convertValueToString(valueStr, sizeof(valueStr), value, id);
                                        if (id == focusedSensor)
                                        {
                                            const int top_margin = 10;
                                            const int side_margin = 10;
                                            DisplayUtils::DrawTextCentered(lcd, -10, valueStr, COLOR_BLACK, COLOR_NEUTRAL, DisplayUtils::TextFont::SimplyMono_Bold50pt7b, 1);
                                            DisplayUtils::DrawText(lcd, DISPLAY_WIDTH - (valueLen * 12 + side_margin), top_margin, unitText, COLOR_RED, COLOR_NEUTRAL, DisplayUtils::TextFont::David_Sans8pt7b, 1);
                                            DisplayUtils::DrawText(lcd, side_margin, top_margin, Translator::get(titleKey), COLOR_BLACK, COLOR_NEUTRAL, DisplayUtils::TextFont::David_Sans8pt7b, 1);
                                        }
                                        else
                                        {
                                            int adjustment = (pageIndex > 0 && static_cast<int>(focusedSensor) < index) ? 1 : 0;
                                            if(pageIndex * (currentSensorDrawn() - 1) + adjustment > index) {
                                                return; // Skip drawing this sensor if it's not on the current page
                                            }
                                            // Calculate the row (0 to 2) and column (0 to 1) based on the index
                                            int row = lowerDataCount / 2;
                                            int col = lowerDataCount % 2;

                                            // Calculate the top-left X and Y coordinates for the current cell
                                            int cellX = (col * 149) + 5;
                                            int cellY = 200 + (row * 66) + 5;

                                            DisplayUtils::DrawText(lcd, cellX, cellY, Translator::get(titleKey), COLOR_BLACK, COLOR_NEUTRAL, DisplayUtils::TextFont::pf_ronda_seven8pt7b, 2);

                                            DisplayUtils::DrawText(lcd, cellX+10, cellY + 38, valueStr, COLOR_BLACK, COLOR_NEUTRAL, DisplayUtils::TextFont::David_Sans8pt7b, 2);

                                            int unitX = cellX + 149 - (valueLen * 12);

                                            // 5. Draw the Unit Text (Size 1 or 2)
                                            DisplayUtils::DrawText(lcd, unitX, cellY, unitText, COLOR_RED, COLOR_NEUTRAL, DisplayUtils::TextFont::pf_ronda_seven8pt7b, 2);
                                            ++lowerDataCount;
                                        } }); // End of callback
        break;
    }
    case DisplayLayout::WindPage:
    {

        DisplayUtils::DrawTextCentered(lcd, 15, "Wind", COLOR_BLACK, COLOR_NEUTRAL, DisplayUtils::TextFont::David_Sans8pt7b, 1);

        char valueStr[32];
        convertValueToString(valueStr, sizeof(valueStr), dataModel.getSensorValueCurrentUnit(SensorId::WindSpeedAWS), SensorId::WindSpeedAWS);

        const int wind = std::get<int>(dataModel.getSensorValueCurrentUnit(SensorId::WindDirectionAWA));

        char unitStr[32];
        dataModel.writeCurrentUnitString(unitStr, sizeof(unitStr), SensorId::WindSpeedAWS);

        lcd.drawBitmap(0, 0, epd_bitmap_wind_gimp, 300, 400, COLOR_BLACK);
        lcd.drawBitmapRotated(0, 0, epd_bitmap_wind_hand, 300, 400, COLOR_BLACK, wind);

        if (wind <= 90 || wind > 270)
        {
            DisplayUtils::DrawTextCentered(lcd, 265, valueStr, COLOR_BLACK, COLOR_NEUTRAL, DisplayUtils::TextFont::David_Sans8pt7b, 3);
            DisplayUtils::DrawTextCentered(lcd, 212, Translator::get(TextKey::SensorWindSpeedAWS), COLOR_BLACK, COLOR_NEUTRAL, DisplayUtils::TextFont::David_Sans8pt7b, 1);
        } else {
            DisplayUtils::DrawTextCentered(lcd, 165, valueStr, COLOR_BLACK, COLOR_NEUTRAL, DisplayUtils::TextFont::David_Sans8pt7b, 3);
            DisplayUtils::DrawTextCentered(lcd, 112, Translator::get(TextKey::SensorWindSpeedAWS), COLOR_BLACK, COLOR_NEUTRAL, DisplayUtils::TextFont::David_Sans8pt7b, 1);
        }

        
        DisplayUtils::DrawTextCentered(lcd, 295, unitStr, COLOR_BLACK, COLOR_NEUTRAL, DisplayUtils::TextFont::David_Sans8pt7b, 1);

        break;
    }
    default:
        break;
    }
}

void DisplayHandler::DrawSettingsPage(const SettingsDisplayStatus &status)
{
    lastUpdate = millis();
    lcd.clear(COLOR_NEUTRAL);

    DisplayUtils::DrawTextCentered(lcd, 15, "Settings", COLOR_BLACK, COLOR_NEUTRAL, DisplayUtils::TextFont::David_Sans8pt7b, 1);
    int y_pos = 40;
    const int fitItemCount = 12;
    switch (status.mode)
    {
    case SettingDisplayMode::SettingsList:
    {
        int y_pos = 40;
        for (size_t i = 0; i < settings.getSettingsCount(); i++)
        {
            const SettingDef &setting = settings.getSettingDef(i);
            const char *settingName = setting.GetTitleString();
            char valueStr[32];
            setting.GetOptionString(settings.getValueVariant(i), valueStr, sizeof(valueStr));
            if (status.isEditing && status.currentSettingIndex == i)
            {
                lcd.fillRect(2, y_pos - 4, 296, 24, COLOR_BLACK);
                DisplayUtils::DrawText(lcd, 10, y_pos, settingName, COLOR_NEUTRAL, COLOR_NEUTRAL, DisplayUtils::TextFont::pf_ronda_seven8pt7b, 2);
                DisplayUtils::DrawText(lcd, 255, y_pos, valueStr, COLOR_NEUTRAL, COLOR_NEUTRAL, DisplayUtils::TextFont::David_Sans8pt7b, 1);
            }
            else
            {
                DisplayUtils::DrawText(lcd, 10, y_pos, settingName, COLOR_BLACK, COLOR_NEUTRAL, DisplayUtils::TextFont::pf_ronda_seven8pt7b, 2);
                DisplayUtils::DrawText(lcd, 255, y_pos, valueStr, COLOR_RED, COLOR_NEUTRAL, DisplayUtils::TextFont::David_Sans8pt7b, 1);
                if (status.currentSettingIndex == i)
                {
                    lcd.drawRect(2, y_pos - 4, 296, 24, COLOR_BLACK);
                }
            }
            y_pos += 30;
        }
        break;
    }
    case SettingDisplayMode::SensorList:
    {
        const char *onText = Translator::get(TextKey::SettingOn);
        const char *offText = Translator::get(TextKey::SettingOff);
        const int startIndex = status.currentSettingIndex > fitItemCount ? status.currentSettingIndex - fitItemCount+1 : 0;
        for (size_t i = startIndex; i < static_cast<size_t>(SensorId::MAX_SENSORS) && i < status.currentSettingIndex + fitItemCount; i++)
        {
            const SensorId sensorId = static_cast<SensorId>(i);
            const char *sensorName = Translator::get(dataModel.getSensorTitleKey(sensorId));
            if (status.isEditing && status.currentSettingIndex == i)
            {
                lcd.fillRect(2, y_pos - 4, 296, 24, COLOR_BLACK);
                DisplayUtils::DrawText(lcd, 10, y_pos, sensorName, COLOR_NEUTRAL, COLOR_NEUTRAL, DisplayUtils::TextFont::pf_ronda_seven8pt7b, 2);
                DisplayUtils::DrawText(lcd, 255, y_pos, dataModel.isSensorEnabled(sensorId) ? onText : offText, COLOR_NEUTRAL, COLOR_NEUTRAL, DisplayUtils::TextFont::David_Sans8pt7b, 1);
            }
            else
            {
                DisplayUtils::DrawText(lcd, 10, y_pos, sensorName, COLOR_BLACK, COLOR_NEUTRAL, DisplayUtils::TextFont::pf_ronda_seven8pt7b, 2);
                DisplayUtils::DrawText(lcd, 255, y_pos, dataModel.isSensorEnabled(sensorId) ? onText : offText, COLOR_BLACK, COLOR_NEUTRAL, DisplayUtils::TextFont::David_Sans8pt7b, 1);
                if (status.currentSettingIndex == i)
                {
                    lcd.drawRect(2, y_pos - 4, 296, 24, COLOR_BLACK);
                }
            }
            y_pos += 30;
        }
        break;
    }
    default:
        break;
    }

    lcd.update();
    updateCounter++;
}

void DisplayHandler::ResetDisplay()
{
    lcd.clear(COLOR_NEUTRAL);
    lcd.update();
    delay(1000);
    lcd.clear(COLOR_WHITE);
    lcd.update();
    delay(1000);
    lcd.clear(COLOR_BLACK);
    lcd.update();
    delay(1000);
    lcd.clear(COLOR_RED);
    lcd.update();
    delay(1000);
    lcd.clear(COLOR_NEUTRAL);
    lcd.update();
    delay(1000);
    lcd.clear(COLOR_WHITE);
    lcd.update();
    delay(1000);
    lcd.clear(COLOR_BLACK);
    lcd.update();
    delay(1000);
    lcd.clear(COLOR_RED);
    lcd.update();
    delay(1000);
    lcd.clear(COLOR_NEUTRAL);
    lcd.update();
}

void DisplayHandler::stepFocusedSensor()
{
    do
    {
        focusedSensor = static_cast<SensorId>((static_cast<int>(focusedSensor) + 1) % static_cast<int>(SensorId::MAX_SENSORS));
    } while (!dataModel.isSensorEnabledAndHaveData(focusedSensor));
}

void DisplayHandler::nextDisplayPage()
{
    if (currentLayout == DisplayLayout::ThreeColTwoRow && (currentSensorDrawn() + (pageIndex * currentSensorDrawn() - pageIndex)) < dataModel.getActiveSensorCount())
    {
        pageIndex++;
    }
    else
    {
        pageIndex = 0;
        currentLayout = static_cast<DisplayLayout>((static_cast<int>(currentLayout) + 1) % static_cast<int>(DisplayLayout::MAX_LAYOUTs));
    }
}
