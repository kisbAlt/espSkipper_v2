#include "displayHandler.hpp"
#include <Arduino.h>
#include "fonts.hpp"
#include "Osptek_BWR_42.h"
#include "displayUtils.hpp"
#include "Wire.h"
#include "SPI.h"
#include "settingsHandler.hpp"
#include "bitmaps.h"

void DisplayHandler::ConvertValueToString(char *valueStr, int len, SensorValue value, SensorId id)
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

int DisplayHandler::CurrentSensorDrawn() const
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
                                                                                           lcd(LCD_CS, LCD_DC, LCD_RES, -1, -1), 
                                                                                           currentLayout(DisplayLayout::ThreeColTwoRow)
{
}

void DisplayHandler::Init()
{
    lcd.begin(true);
    lcd.clear(COLOR_NEUTRAL);
    lcd.update();
}

void DisplayHandler::UpdateDisplay()
{
    lcd.clear(COLOR_NEUTRAL);
    DrawLayout();
    lcd.update();
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

        if (settings.GetValue<bool>(SettingsKey::DisplayDateTime))
        {
            char dateStr[32];
            int hour = std::get<int>(dataModel.GetSensorValue(SensorId::DateTimeHour));
            int minute = std::get<int>(dataModel.GetSensorValue(SensorId::DateTimeMinute));
            if (settings.GetValue<bool>(SettingsKey::DisplayTimeOnly))
            {
                snprintf(dateStr, sizeof(dateStr), "%02d:%02d", hour, minute);
            }
            else
            {
                int year = std::get<int>(dataModel.GetSensorValue(SensorId::DateTimeYear));
                int month = std::get<int>(dataModel.GetSensorValue(SensorId::DateTimeMonth));
                int day = std::get<int>(dataModel.GetSensorValue(SensorId::DateTimeDay));
                snprintf(dateStr, sizeof(dateStr), "%04d.%02d.%02d %02d:%02d", year, month, day, hour, minute);
            }
            DisplayUtils::DrawTextCentered(lcd, 180, dateStr, COLOR_BLACK, COLOR_NEUTRAL, DisplayUtils::TextFont::David_Sans8pt7b, 1);
        }

        int lowerDataCount = 0;
        dataModel.drawActiveSensors([&](int index, const TextKey titleKey, SensorValue value, const char *unitText, SensorId id)
                                    {
                                        int valueLen = strlen(unitText);
                                        char valueStr[32];
                                        ConvertValueToString(valueStr, sizeof(valueStr), value, id);
                                        if (id == focusedSensor)
                                        {
                                            const int topMargin = 10;
                                            const int sideMargin = 10;
                                            DisplayUtils::DrawTextCentered(lcd, -10, valueStr, COLOR_BLACK, COLOR_NEUTRAL, DisplayUtils::TextFont::SimplyMono_Bold50pt7b, 1);
                                            DisplayUtils::DrawText(lcd, OSP_LCD_WIDTH - (valueLen * 12 + sideMargin), topMargin, unitText, COLOR_RED, COLOR_NEUTRAL, DisplayUtils::TextFont::David_Sans8pt7b, 1);
                                            DisplayUtils::DrawText(lcd, sideMargin, topMargin, Translator::Get(titleKey), COLOR_BLACK, COLOR_NEUTRAL, DisplayUtils::TextFont::David_Sans8pt7b, 1);
                                        }
                                        else
                                        {
                                            int adjustment = (pageIndex > 0 && static_cast<int>(focusedSensor) < index) ? 1 : 0;
                                            if(pageIndex * (CurrentSensorDrawn() - 1) + adjustment > index) {
                                                return; // Skip drawing this sensor if it's not on the current page
                                            }
                                            int row = lowerDataCount / 2;
                                            int col = lowerDataCount % 2;

                                            int cellX = (col * 149) + 5;
                                            int cellY = 200 + (row * 66) + 5;

                                            DisplayUtils::DrawText(lcd, cellX, cellY, Translator::Get(titleKey), COLOR_BLACK, COLOR_NEUTRAL, DisplayUtils::TextFont::pf_ronda_seven8pt7b, 2);

                                            DisplayUtils::DrawText(lcd, cellX+10, cellY + 38, valueStr, COLOR_BLACK, COLOR_NEUTRAL, DisplayUtils::TextFont::David_Sans8pt7b, 2);

                                            int unitX = cellX + 149 - (valueLen * 12);

                                            DisplayUtils::DrawText(lcd, unitX, cellY, unitText, COLOR_RED, COLOR_NEUTRAL, DisplayUtils::TextFont::pf_ronda_seven8pt7b, 2);
                                            ++lowerDataCount;
                                        } });
        break;
    }
    case DisplayLayout::WindPage:
    {

        DisplayUtils::DrawTextCentered(lcd, 15, Translator::Get(TextKey::SensorWind), COLOR_BLACK, COLOR_NEUTRAL, DisplayUtils::TextFont::David_Sans8pt7b, 1);

        char valueStr[32];
        ConvertValueToString(valueStr, sizeof(valueStr), dataModel.GetSensorValueCurrentUnit(SensorId::WindSpeedAWS), SensorId::WindSpeedAWS);

        const int wind = std::get<int>(dataModel.GetSensorValueCurrentUnit(SensorId::WindDirectionAWA));

        char unitStr[32];
        dataModel.WriteCurrentUnitString(unitStr, sizeof(unitStr), SensorId::WindSpeedAWS);

        lcd.drawBitmap(0, 0, epd_bitmap_wind_gimp, 300, 400, COLOR_BLACK);
        lcd.drawBitmapRotated(0, 0, epd_bitmap_wind_hand, 300, 400, COLOR_BLACK, wind);

        if (wind <= 90 || wind > 270)
        {
            DisplayUtils::DrawTextCentered(lcd, 265, valueStr, COLOR_BLACK, COLOR_NEUTRAL, DisplayUtils::TextFont::David_Sans8pt7b, 3);
            DisplayUtils::DrawTextCentered(lcd, 212, Translator::Get(TextKey::SensorWindSpeedAWS), COLOR_BLACK, COLOR_NEUTRAL, DisplayUtils::TextFont::David_Sans8pt7b, 1);
        } else {
            DisplayUtils::DrawTextCentered(lcd, 165, valueStr, COLOR_BLACK, COLOR_NEUTRAL, DisplayUtils::TextFont::David_Sans8pt7b, 3);
            DisplayUtils::DrawTextCentered(lcd, 112, Translator::Get(TextKey::SensorWindSpeedAWS), COLOR_BLACK, COLOR_NEUTRAL, DisplayUtils::TextFont::David_Sans8pt7b, 1);
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
    lcd.clear(COLOR_NEUTRAL);

    DisplayUtils::DrawTextCentered(lcd, 15, Translator::Get(TextKey::Settings), COLOR_BLACK, COLOR_NEUTRAL, DisplayUtils::TextFont::David_Sans8pt7b, 1);
    int yPos = 40;
    const int fitItemCount = 12;
    switch (status.mode)
    {
    case SettingDisplayMode::SettingsList:
    {
        int yPos = 40;
        for (size_t i = 0; i < settings.GetSettingsCount(); i++)
        {
            const SettingDef &setting = settings.GetSettingDef(i);
            const char *settingName = setting.GetTitleString();
            char valueStr[32];
            setting.GetOptionString(settings.GetValueVariant(i), valueStr, sizeof(valueStr));
            if (status.isEditing && status.currentSettingIndex == i)
            {
                lcd.fillRect(2, yPos - 4, 296, 24, COLOR_BLACK);
                DisplayUtils::DrawText(lcd, 10, yPos, settingName, COLOR_NEUTRAL, COLOR_NEUTRAL, DisplayUtils::TextFont::pf_ronda_seven8pt7b, 2);
                DisplayUtils::DrawText(lcd, 255, yPos, valueStr, COLOR_NEUTRAL, COLOR_NEUTRAL, DisplayUtils::TextFont::David_Sans8pt7b, 1);
            }
            else
            {
                DisplayUtils::DrawText(lcd, 10, yPos, settingName, COLOR_BLACK, COLOR_NEUTRAL, DisplayUtils::TextFont::pf_ronda_seven8pt7b, 2);
                DisplayUtils::DrawText(lcd, 255, yPos, valueStr, COLOR_RED, COLOR_NEUTRAL, DisplayUtils::TextFont::David_Sans8pt7b, 1);
                if (status.currentSettingIndex == i)
                {
                    lcd.drawRect(2, yPos - 4, 296, 24, COLOR_BLACK);
                }
            }
            yPos += 30;
        }
        break;
    }
    case SettingDisplayMode::SensorList:
    {
        const char *onText = Translator::Get(TextKey::SettingOn);
        const char *offText = Translator::Get(TextKey::SettingOff);
        const int startIndex = status.currentSettingIndex > fitItemCount ? status.currentSettingIndex - fitItemCount+1 : 0;
        for (size_t i = startIndex; i < static_cast<size_t>(SensorId::MAX_SENSORS) && i < status.currentSettingIndex + fitItemCount; i++)
        {
            const SensorId sensorId = static_cast<SensorId>(i);
            const char *sensorName = Translator::Get(dataModel.GetSensorTitleKey(sensorId));
            if (status.isEditing && status.currentSettingIndex == i)
            {
                lcd.fillRect(2, yPos - 4, 296, 24, COLOR_BLACK);
                DisplayUtils::DrawText(lcd, 10, yPos, sensorName, COLOR_NEUTRAL, COLOR_NEUTRAL, DisplayUtils::TextFont::pf_ronda_seven8pt7b, 2);
                DisplayUtils::DrawText(lcd, 255, yPos, dataModel.IsSensorEnabled(sensorId) ? onText : offText, COLOR_NEUTRAL, COLOR_NEUTRAL, DisplayUtils::TextFont::David_Sans8pt7b, 1);
            }
            else
            {
                DisplayUtils::DrawText(lcd, 10, yPos, sensorName, COLOR_BLACK, COLOR_NEUTRAL, DisplayUtils::TextFont::pf_ronda_seven8pt7b, 2);
                DisplayUtils::DrawText(lcd, 255, yPos, dataModel.IsSensorEnabled(sensorId) ? onText : offText, COLOR_BLACK, COLOR_NEUTRAL, DisplayUtils::TextFont::David_Sans8pt7b, 1);
                if (status.currentSettingIndex == i)
                {
                    lcd.drawRect(2, yPos - 4, 296, 24, COLOR_BLACK);
                }
            }
            yPos += 30;
        }
        break;
    }
    default:
        break;
    }

    lcd.update();
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

void DisplayHandler::StepFocusedSensor()
{
    do
    {
        focusedSensor = static_cast<SensorId>((static_cast<int>(focusedSensor) + 1) % static_cast<int>(SensorId::MAX_SENSORS));
    } while (!dataModel.IsSensorEnabledAndHasData(focusedSensor));
}

void DisplayHandler::NextDisplayPage()
{
    if (currentLayout == DisplayLayout::ThreeColTwoRow && (CurrentSensorDrawn() + (pageIndex * CurrentSensorDrawn() - pageIndex)) < dataModel.GetActiveSensorCount())
    {
        pageIndex++;
    }
    else
    {
        pageIndex = 0;
        currentLayout = static_cast<DisplayLayout>((static_cast<int>(currentLayout) + 1) % static_cast<int>(DisplayLayout::MAX_LAYOUTS));
    }
}
