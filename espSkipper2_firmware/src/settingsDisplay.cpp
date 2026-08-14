#include "SettingsDisplay.hpp"
#include "settingsHandler.hpp"
#include "display/displayHandler.hpp"

SettingsDisplay::SettingsDisplay(Settings &settings, DisplayHandler &displayHandler)
        : settings(settings), displayHandler(displayHandler)
{
}


void SettingsDisplay::handleButtonPress(ButtonEvent btnEvent, ButtonName btnName)
{
    std::lock_guard<std::mutex> lock(uiMutex);
    lastButtonEvent = btnEvent;
    lastButtonName = btnName;
    dirty = true;
    cv.notify_one();
}

void SettingsDisplay::drawSettingsUI()
{
    bool running = true;
        
        // Lock mutex to set initial state
        {
            std::lock_guard<std::mutex> lock(uiMutex);
            dirty = true;
        }

        while (running) {
            ButtonEvent currButtonEvent;
            ButtonName currButtonName;

            // Wait for a button press while asleep
            {
                std::unique_lock<std::mutex> lock(uiMutex);
                
                // This puts the Display Thread to sleep (0% CPU).
                // It wakes up ONLY when injectButton() calls cv.notify_one()
                cv.wait(lock, [this] { return dirty; });

                // Capture the state quickly and release the lock
                currButtonEvent = lastButtonEvent;
                currButtonName = lastButtonName;
                dirty = false;
            }


            // Draw the updated UI
            displayHandler.updateDisplay(UpdatePage::SETTINGS_SCREEN);
        }
}
