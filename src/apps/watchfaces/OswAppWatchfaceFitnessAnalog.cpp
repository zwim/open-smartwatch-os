#include "apps/watchfaces/OswAppWatchfaceFitnessAnalog.h"
#include "apps/watchfaces/OswAppWatchfaceDigital.h"
#include "apps/watchfaces/OswAppWatchface.h"

#include <config.h>
#include <gfx_util.h>
#include <OswAppV1.h>
#include <osw_config_keys.h>
#include <osw_hal.h>
#include <time.h>

uint32_t OswAppWatchfaceFitnessAnalog::calculateDistance(uint32_t steps) {
    float userHeight = OswConfigAllKeys::configHeight.get();
    float avgDist = ((userHeight * 0.37f) + (userHeight * 0.45f) + (userHeight - 100.0f)) / 3.0f;
    return steps * avgDist * 0.01f + 0.5f ;  // cm -> m
}

void OswAppWatchfaceFitnessAnalog::showFitnessTracking(OswHal *hal) {
    uint32_t steps = hal->environment()->getStepsToday();
    uint32_t dists = OswAppWatchfaceFitnessAnalog::calculateDistance(steps);

    uint32_t stepsTarget = OswConfigAllKeys::stepsPerDay.get();
    uint32_t distTarget = OswConfigAllKeys::distPerDay.get();

    uint8_t arcRadius = 6;
    uint16_t yellow = rgb565(50,255,0);

#ifdef OSW_EMULATOR
    steps = 4000;
#endif

    int32_t angel_val = 180.0f * (float)min(steps, stepsTarget) / (float)stepsTarget;
    hal->gfx()->drawArc(DISP_W / 2, DISP_H / 2, 180 + angel_val, 360,
        90, 92, arcRadius, changeColor(yellow, 0.25f));
    hal->gfx()->drawArc(DISP_W / 2, DISP_H / 2, 180, 180 + angel_val, 
        90, 92, arcRadius, steps > stepsTarget ? changeColor(yellow, 6.25 ): yellow, true);

    angel_val = 180.0f * (float) min(dists, distTarget) / (float)distTarget;
    hal->gfx()->drawArc(DISP_W / 2, DISP_H / 2, 180 + angel_val, 360, 
        90, 75, arcRadius, changeColor(ui->getInfoColor(), 0.25f));
    hal->gfx()->drawArc(DISP_W / 2, DISP_H / 2, 180, 180 + angel_val, 
        90, 75, arcRadius, dists > distTarget  ? changeColor(ui->getSuccessColor(),2.25) : ui->getInfoColor(), true);

    hal->gfx()->setTextSize(1);
    hal->gfx()->setTextLeftAligned();

    hal->gfx()->setTextColor(dimColor(yellow, 25));
    hal->gfx()->setTextCursor(DISP_W / 2 + 12, 8+23);
    hal->gfx()->print(steps);
    hal->gfx()->setTextCursor(DISP_W / 2 + 12, DISP_H-23);
    hal->gfx()->print(LANG_WATCHFACE_FITNESS_STEP);

    hal->gfx()->setTextColor(dimColor(ui->getInfoColor(), 24));
    hal->gfx()->setTextCursor(DISP_W / 2 + 12, 8+40);
    hal->gfx()->print(dists);
    hal->gfx()->setTextCursor(DISP_W / 2 + 12, DISP_H-40);
    hal->gfx()->print(LANG_WATCHFACE_FITNESS_DISTANCE);
}

void OswAppWatchfaceFitnessAnalog::drawWatchFace(OswHal *hal, uint32_t hour, uint32_t minute, uint32_t second, bool afterNoon) {
    // Indices
    hal->gfx()->drawMinuteTicks(DISP_W / 2, DISP_H / 2, 116, 112, ui->getForegroundDimmedColor(), true);
    hal->gfx()->drawHourTicks(DISP_W / 2, DISP_H / 2, 117, 107, ui->getForegroundColor(), true);

    // Hours
    hal->gfx()->drawThickTick(DISP_W / 2, DISP_H / 2,  0, 60, 360.0f / 12.0f * (1.0f * hour + minute / 60.0f), 3, ui->getForegroundColor(), false, true);
    hal->gfx()->drawThickTick(DISP_W / 2, DISP_H / 2, 16, 60, 360.0f / 12.0f * (1.0f * hour + minute / 60.0f), 7, ui->getForegroundColor(), false, true);

    // Minutes
    hal->gfx()->drawThickTick(DISP_W / 2, DISP_H / 2,  0, 105, 360.0f / 60.0f * (1.0f * minute + second / 60.0f), 3, ui->getForegroundColor(), false, true);
    hal->gfx()->drawThickTick(DISP_W / 2, DISP_H / 2, 16, 105, 360.0f / 60.0f * (1.0f * minute + second / 60.0f), 7, ui->getForegroundColor(), false, true);

#ifndef GIF_BG
    // Seconds
    hal->gfx()->fillCircleAA(DISP_W / 2, DISP_H / 2, 6, ui->getDangerColor());
    hal->gfx()->drawThickTick(DISP_W / 2, DISP_H / 2, -16, 110, 360.0f / 60.0f * second, 3, ui->getDangerColor(), false, true);
//    hal->gfx()->drawThickTick(DISP_W / 2, DISP_H / 2, -16, 110, 360.0f / 60.0f * 32, 3, ui->getDangerColor(), false, true);
//    hal->gfx()->drawThickTick(DISP_W / 2, DISP_H / 2, -16, 110, 360.0f / 60.0f * 27, 3, ui->getDangerColor(), false, true);
//    hal->gfx()->drawThickTick(DISP_W / 2, DISP_H / 2, -16, 110, 360.0f / 60.0f * 57, 3, ui->getDangerColor(), false, true);
//    hal->gfx()->drawThickTick(DISP_W / 2, DISP_H / 2, -16, 110, 360.0f / 60.0f * 8, 3, ui->getDangerColor(), false, true);
    hal->gfx()->drawPixel(DISP_W / 2, DISP_H / 2, 0);
#endif

/*    hal->gfx()->drawThickLineAA(100, 50, 160, 70, 9, rgb565(250,0,250));

    hal->gfx()->drawThickLineAA(50, 100, 70, 150  , 10, rgb565(255,0,200));

    hal->gfx()->drawThickLineAA(160, 70+20, 100, 95, 10, rgb565(250,0,250));

    hal->gfx()->drawThickLineAA(70, 200, 80, 150  , 10, rgb565(255,0,200));

    hal->gfx()->drawThickLineAA(120+10, 120 ,120+20, 120+90 , 10, rgb565(255,0,200));
    hal->gfx()->drawThickLineAA(120-10, 120 ,120-20, 120+90 , 10, rgb565(255,0,200));
*/



   // hal->gfx()->drawThickLineAA(50, 50, 150, 60, 10, rgb565(255,0,200));
   // hal->gfx()->drawCircleAA(100, 100, 10, 0, rgb565(0,255,0));

   // hal->gfx()->drawCircleAA(150, 150, 12, 6, rgb565(0,55,255));

}

#ifdef GIF_BG
OswAppGifPlayer* bgGif = new OswAppGifPlayer();
#endif

void OswAppWatchfaceFitnessAnalog::drawDateFace(OswHal *hal, uint32_t hour, uint32_t minute, uint32_t second, bool afterNoon) {
    const char* weekday = hal->getLocalWeekday();

    hal->gfx()->setTextSize(2);
    hal->gfx()->setTextRightAligned();
    hal->gfx()->setTextCursor(205, 75);
    OswAppWatchfaceDigital::displayWeekDay3(weekday);

    // Date
    uint32_t dayInt = 0;
    uint32_t monthInt = 0;
    uint32_t yearInt = 0;
    hal->getLocalDate(&dayInt, &monthInt, &yearInt);
    hal->gfx()->setTextSize(3);
    hal->gfx()->setTextLeftAligned();
    hal->gfx()->setTextCursor(DISP_W / 2 - 70, 170);
    OswAppWatchfaceDigital::dateOutput(yearInt, monthInt, dayInt);

    hal->gfx()->setTextSize(4);
    hal->gfx()->setTextLeftAligned();
    hal->gfx()->setTextCursor(DISP_W / 2 - 35, DISP_H / 2);

    hal->gfx()->printDecimal(hour, 2);
    hal->gfx()->print(":");
    hal->gfx()->printDecimal(minute, 2);

    hal->gfx()->setTextSize(2);
    hal->gfx()->setTextLeftAligned();
    hal->gfx()->setTextCursor(215, DISP_H / 2);
    hal->gfx()->printDecimal(second,2);

    if (!OswConfigAllKeys::timeFormat.get()) {
        const char am[] = "AM";
        const char pm[] = "PM";
        hal->gfx()->setTextCursor(215, 130);
        if (afterNoon) {
            hal->gfx()->print(pm);
        } else {
            hal->gfx()->print(am);
        }
    }

#if OSW_PLATFORM_ENVIRONMENT_TEMPERATURE == 1
/*    
    printStatus("Temperature", String(hal->environment()->getTemperature() + String("C")).c_str());
    for(auto& d : *OswTemperatureProvider::getAllTemperatureDevices())
        printStatus((String("  ") + d->getName()).c_str(), String(d->getTemperature() + String("C")).c_str());
*/
    hal->gfx()->setTextSize(2);
    hal->gfx()->setTextLeftAligned();
    hal->gfx()->setTextCursor(DISP_W * 0.2f, DISP_H * 0.2f);
    hal->gfx()->print(hal->environment()->getTemperature(), 1);
    hal->gfx()->print("C");
#endif
}

const char* OswAppWatchfaceFitnessAnalog::getAppId() {
    return OswAppWatchfaceFitnessAnalog::APP_ID;
}

const char* OswAppWatchfaceFitnessAnalog::getAppName() {
    return "AFIT"; //LANG_FITNESS;
}

void OswAppWatchfaceFitnessAnalog::onStart() {
    OswAppV2::onStart();
    OswAppWatchface::addButtonDefaults(this->knownButtonStates);

    // Report that we support short presses on all buttons
    this->knownButtonStates[Button::BUTTON_SELECT] = ButtonStateNames::SHORT_PRESS ;
    this->knownButtonStates[Button::BUTTON_UP] = ButtonStateNames::SHORT_PRESS;
    this->knownButtonStates[Button::BUTTON_DOWN] = ButtonStateNames::SHORT_PRESS;

    // Here is a snippet to also "support" double presses (on BUTTON_SELECT) - note that this WILL DELAY the reporting of any short press events on that button (as it may needs to wait for the second press)
    this->knownButtonStates[Button::BUTTON_SELECT] = (OswAppV2::ButtonStateNames) (this->knownButtonStates[Button::BUTTON_SELECT] | OswAppV2::ButtonStateNames::DOUBLE_PRESS); // OR to set the bit
    this->knownButtonStates[Button::BUTTON_UP] = (OswAppV2::ButtonStateNames) (this->knownButtonStates[Button::BUTTON_UP] | OswAppV2::ButtonStateNames::DOUBLE_PRESS); // OR to set the bit
    this->knownButtonStates[Button::BUTTON_DOWN] = (OswAppV2::ButtonStateNames) (this->knownButtonStates[Button::BUTTON_DOWN] | OswAppV2::ButtonStateNames::DOUBLE_PRESS); // OR to set the bit

    this->lastTime = time(nullptr); // use
}

void OswAppWatchfaceFitnessAnalog::onLoop() {
    OswAppV2::onLoop();

    this->needsRedraw = this->needsRedraw or time(nullptr) != this->lastTime; // redraw every second
}

void OswAppWatchfaceFitnessAnalog::onDraw() {
    OswAppV2::onDraw();

    OswHal* hal = OswHal::getInstance();
    uint32_t second = 0;
    uint32_t minute = 0;
    uint32_t hour = 0;
    bool afterNoon;
    hal->getLocalTime(&hour, &minute, &second, &afterNoon);

    if (this->screen == 0) {
        #if OSW_PLATFORM_ENVIRONMENT_ACCELEROMETER == 1
            showFitnessTracking(hal);
        #endif

        drawWatchFace(hal, hour, minute, second, afterNoon);
    } else if (this->screen == 1) {
        drawDateFace(hal, hour, minute, second, afterNoon);

        static int wait_time = 1;
        if (wait_time >= 0)
            --wait_time;
        else {
            this->screen = 0;
            wait_time = 1;
        }
    }
        
    this->lastTime = time(nullptr);
}

void OswAppWatchfaceFitnessAnalog::onButton(Button id, bool up, OswAppV2::ButtonStateNames state) {
    OswAppV2::onButton(id, up, state);

    if(!up) return;

    this->lastTime = time(nullptr); // reset the timeout, as the user interacted with the device

    if (state == OswAppV2::ButtonStateNames::DOUBLE_PRESS) {
        if (this->screen < 1)
            ++this->screen;
        return;       
    }

    if(OswAppWatchface::onButtonDefaults(*this, id, up, state))
        return; // if the button was handled by the defaults, we are done here
}

/**
 * Fast RGB565 pixel blending
 * @param fg      The foreground color in uint16_t RGB565 format
 * @param bg      The background color in uint16_t RGB565 format
 * @param alpha   The alpha in range 0-255
 **/
uint16_t alphaBlendRGB565( uint32_t fg, uint32_t bg, uint8_t alpha ){
    alpha = ( alpha + 4 ) >> 3;
    bg = (bg | (bg << 16)) & 0b00000111111000001111100000011111;
    fg = (fg | (fg << 16)) & 0b00000111111000001111100000011111;
    uint32_t result = ((((fg - bg) * alpha) >> 5) + bg) & 0b00000111111000001111100000011111;
    return (uint16_t)((result >> 16) | result);
}


void OswAppWatchfaceFitnessAnalog::onStop() {
    OswAppV2::onStop(); // always make sure to call the base class method!

    printf("xxx Experiments ...");
    
    // This is where you de-initialize stuff, gets called when another app is shown
}