#include "apps/watchfaces/OswAppWatchfaceFitnessAnalog.h"
#include "apps/watchfaces/OswAppWatchfaceDigital.h"
#include "apps/watchfaces/OswAppWatchface.h"

#ifdef GIF_BG
#include "./apps/_experiments/gif_player.h"
#endif

#include <config.h>
#include <gfx_util.h>
#include <OswAppV1.h>
#include <osw_config_keys.h>
#include <osw_hal.h>
#include <time.h>

#define CENTER_X (DISP_W / 2)
#define CENTER_Y (DISP_H / 2)

#ifdef GIF_BG
OswAppGifPlayer* bgGif = new OswAppGifPlayer();
#endif

void OswAppWatchfaceFitnessAnalog::cleanupLuaState() {
    if (luaState) {
        lua_close(luaState);
        luaState = NULL;
    }
}

inline void OswAppWatchfaceFitnessAnalog::printLuaError() {
    if (luaState) 
        OSW_LOG_E(lua_tostring(luaState, -1));
}

bool OswAppWatchfaceFitnessAnalog::getTwilights() {
    cleanupLuaState();
    luaState = luaL_newstate();

    if (!luaState) {
        OSW_LOG_E("luaL_newstate failed\n");
        return false;
    }
        
    luaL_openlibs(luaState);
//    halToLua(luaState);

    //Include search paths
    luaL_dostring(luaState, LUA_PACKAGE_CMD);

    if (luaL_dofile(luaState, LUA_APP_PATH "twilight.lua")) {
        printLuaError();
        cleanupLuaState();
        return false;
    }

//    lua_getnumber(lua_getglobal("varname"))

    hal->updateTimezoneOffsets();
    int luaInitialStackSize = lua_gettop(luaState);

    uint32_t year = 0;
    uint32_t month = 0;
    uint32_t day = 0;
    time_t timeZone = 0;

    timeZone = hal->getTimezoneOffsetPrimary();

    hal->getDate(timeZone, &day, &month, &year);

    uint32_t timeZone_offset_h = timeZone / 3600;

    //  setParams(latitude, longitude, timezone, altitude, year, month, day, dst)
    lua_getglobal(luaState, "setParams");

    lua_pushnumber(luaState, 47.51f);
    lua_pushnumber(luaState, 12.09f);
    lua_pushnumber(luaState, timeZone_offset_h);
    lua_pushnumber(luaState, 520.0f);
    lua_pushnumber(luaState, year);
    lua_pushnumber(luaState, month);
    lua_pushnumber(luaState, day);
    lua_pushboolean(luaState, false); // true

    if (lua_pcall(luaState, 8, 0, 0)) {
        printf("xxxxx8\n");
        printLuaError();
        cleanupLuaState();
        return false;
    }

    for (int i = 1; i <= 11; ++i) {
        lua_getglobal(luaState, "getTimes");
        lua_pushnumber(luaState, i);
        if (lua_pcall(luaState, 1, 1, 0)) {
            printf("xxxx33333333 knock out\n");
            printLuaError();
            cleanupLuaState();
            return false;
        }
        printf("xxxxx SunTime.times[%d]=%f\n", i, lua_tonumber(luaState, -1));
        lua_pop(luaState, 1);
    }

/*
    lua_getglobal(luaState, "SunTime");
    lua_getfield(luaState, -1, "times"); 
    for (int i = 1; i <=11; ++i) {
        if ( lua_geti(luaState, -1, i) == LUA_TNUMBER) { // Times
            sunTimes[i] = lua_tonumber(luaState, -1);
            printf("xxxxx SunTime.times[%d]=%f\n", i, sunTimes[i]);
        } else {
            sunTimes[i] = -10000;
        }

    }
    lua_pop(luaState, 11+2);
*/


    if (luaInitialStackSize != lua_gettop(luaState)) {
        Serial.printf("Oh my god, the lua stack has grown from %d to %d\n", luaInitialStackSize, lua_gettop(luaState));
    }

        Serial.printf("xxx Oh my god, the lua stack has grown from %d to %d\n", luaInitialStackSize, lua_gettop(luaState));

    return true;
}

inline uint32_t OswAppWatchfaceFitnessAnalog::calculateDistance(uint32_t steps) {
    float userHeight = OswConfigAllKeys::configHeight.get();
    float avgDist;
    if (userHeight < 180)
        avgDist = userHeight * 0.40f;
    else
        avgDist = userHeight * 0.48f;

    return steps * avgDist * 0.01f;  // cm -> m
}

void OswAppWatchfaceFitnessAnalog::showFitnessTracking(OswHal *hal) {
    uint32_t steps = hal->environment()->getStepsToday();
    uint32_t dists = OswAppWatchfaceFitnessAnalog::calculateDistance(steps);

    uint32_t stepsTarget = OswConfigAllKeys::stepsPerDay.get();
    uint32_t distTarget = OswConfigAllKeys::distPerDay.get();

    uint8_t arcRadius = 6;

#ifdef OSW_EMULATOR
    steps = 4000;
    dists = 3000;
#endif

    {   // draw step arc
        int32_t angle_val = 180.0f * (float)min(steps, stepsTarget) / (float)stepsTarget;
        uint16_t color = ui->getWarningColor();
        uint16_t dimmed_color = changeColor(color, 0.25f);
        hal->gfx()->drawCircleAA(CENTER_X, CENTER_Y, 92 +arcRadius, arcRadius*2, dimmed_color, 90, 270-angle_val);
        hal->gfx()->drawCircleAA(CENTER_X, CENTER_Y -92, arcRadius, 0, dimmed_color);

        hal->gfx()->drawCircleAA(CENTER_X, CENTER_Y, 92 +arcRadius, arcRadius*2, steps > stepsTarget ? dimmed_color : color, 270-angle_val, 270);
        hal->gfx()->drawCircleAA(CENTER_X, CENTER_Y +92, arcRadius, 0, steps > stepsTarget ? dimmed_color : color);
        int x = CENTER_X + cosf((270-angle_val)*PI/180) * 92.0f;
        int y = CENTER_Y - sinf((270-angle_val)*PI/180) * 92.0f;
        hal->gfx()->drawCircleAA(x, y, arcRadius, 0, color);
    }

    {   // draw distance arc
        int32_t angle_val = 180.0f * (float)min(dists, distTarget) / (float)distTarget;
        uint16_t color = ui->getInfoColor();
        uint16_t dimmed_color = changeColor(color, 0.25f);
        hal->gfx()->drawCircleAA(CENTER_X, CENTER_Y, 75 +arcRadius, arcRadius*2, dimmed_color, 90, 270-angle_val);
        hal->gfx()->drawCircleAA(CENTER_X, CENTER_Y -75, arcRadius, 0, dimmed_color, 0.25f);

        hal->gfx()->drawCircleAA(CENTER_X, CENTER_Y, 75 +arcRadius, arcRadius*2, steps > stepsTarget ? dimmed_color: color, 270-angle_val, 270);
        hal->gfx()->drawCircleAA(CENTER_X, CENTER_Y +75, arcRadius, 0, steps > stepsTarget ?  dimmed_color: color);
        int x = CENTER_X + cosf((270-angle_val)*PI/180) * 75.0f;
        int y = CENTER_Y - sinf((270-angle_val)*PI/180) * 75.0f;
        hal->gfx()->drawCircleAA(x, y, arcRadius, 0, color);
    }

    hal->gfx()->setTextSize(1);
    hal->gfx()->setTextLeftAligned();

    hal->gfx()->setTextColor(dimColor(ui->getWarningColor(), 25));
    hal->gfx()->setTextCursor(CENTER_X + 12, 8+23);
    hal->gfx()->print(steps);
    hal->gfx()->setTextCursor(CENTER_X + 12, DISP_H-23);
    hal->gfx()->print(LANG_WATCHFACE_FITNESS_STEP);

    hal->gfx()->setTextColor(dimColor(ui->getInfoColor(), 24));
    hal->gfx()->setTextCursor(CENTER_X + 12, 8+40);
    hal->gfx()->print(dists);
    hal->gfx()->setTextCursor(CENTER_X + 12, DISP_H-40);
    hal->gfx()->print(LANG_WATCHFACE_FITNESS_DISTANCE);
}

void OswAppWatchfaceFitnessAnalog::drawWatchFace(OswHal *hal, uint32_t hour, uint32_t minute, uint32_t second, bool afterNoon) {

    hal->gfx()->drawCircleAA(CENTER_X, CENTER_Y, 120, 10, rgb565(35, 35, 35));
    hal->gfx()->drawCircleAA(CENTER_X, CENTER_Y, 120-10, 7, rgb565(30, 30, 30));

    // Indices
    hal->gfx()->drawMinuteTicks(CENTER_X, CENTER_Y, 116, 112, ui->getForegroundDimmedColor(), true);
    hal->gfx()->drawHourTicks(CENTER_X, CENTER_Y, 117, 106, ui->getForegroundColor(), true);

    // Hours
    hal->gfx()->drawThickTick(CENTER_X, CENTER_Y,  0, 16, (int)(360.0f / 12.0f * (hour + minute / 60.0f)), 3, ui->getForegroundColor(), true, STRAIGHT_END);
    hal->gfx()->drawThickTick(CENTER_X, CENTER_Y, 16, 60, (int)(360.0f / 12.0f * (hour + minute / 60.0f)), 7, ui->getForegroundColor(), true);

    // Minutes
    hal->gfx()->drawThickTick(CENTER_X, CENTER_Y,  0, 16, (int)(360.0f / 60.0f * (minute + second / 60.0f)), 3, ui->getForegroundColor(), true, STRAIGHT_END);
    hal->gfx()->drawThickTick(CENTER_X, CENTER_Y, 16, 105, (int)(360.0f / 60.0f * (minute + second / 60.0f)), 7, ui->getForegroundColor(), true);

#ifndef GIF_BG
    // Seconds
    hal->gfx()->fillCircleAA(CENTER_X, CENTER_Y, 6, ui->getDangerColor());
    hal->gfx()->drawThickTick(CENTER_X, CENTER_Y, -16, 110, 360 / 60 * second, 3, ui->getDangerColor(), true);
#endif
}

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
    hal->gfx()->setTextCursor(CENTER_X - 70, 170);
    OswAppWatchfaceDigital::dateOutput(yearInt, monthInt, dayInt);

    hal->gfx()->setTextSize(4);
    hal->gfx()->setTextLeftAligned();
    hal->gfx()->setTextCursor(CENTER_X - 35, CENTER_Y);

    hal->gfx()->printDecimal(hour, 2);
    hal->gfx()->print(":");
    hal->gfx()->printDecimal(minute, 2);

    hal->gfx()->setTextSize(2);
    hal->gfx()->setTextLeftAligned();
    hal->gfx()->setTextCursor(215, CENTER_Y);
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
    hal->gfx()->setTextSize(2);
    hal->gfx()->setTextLeftAligned();
    hal->gfx()->setTextCursor(DISP_W * 0.2f, DISP_H * 0.2f);
    hal->gfx()->print(hal->environment()->getTemperature(), 1);
    hal->gfx()->print("C");
#endif
}

void OswAppWatchfaceFitnessAnalog::drawFitnessFace(OswHal *hal, uint32_t hour, uint32_t minute, uint32_t second, bool afterNoon) {

    uint32_t steps = hal->environment()->getStepsToday();
    uint32_t dists = OswAppWatchfaceFitnessAnalog::calculateDistance(steps);

#ifdef OSW_EMULATOR
    steps = 10213;
    dists = 23444;
#endif

    // Steps
    hal->gfx()->setTextSize(3);
    hal->gfx()->setTextColor(ui->getWarningColor());
    hal->gfx()->setTextLeftAligned();
    hal->gfx()->setTextCursor(CENTER_X - 80, CENTER_Y - 10);
    hal->gfx()->print(LANG_FITNESS_STEP);
    hal->gfx()->print(": ");
    hal->gfx()->print(steps);

    hal->gfx()->setTextSize(2);
    hal->gfx()->setTextColor(ui->getInfoColor());
    hal->gfx()->setTextLeftAligned();
    hal->gfx()->setTextCursor(CENTER_X - 80, CENTER_Y + 20);
    hal->gfx()->print(LANG_FITNESS_DISTANCE);
    hal->gfx()->print(": ");
    hal->gfx()->print(dists);
}

const char* OswAppWatchfaceFitnessAnalog::getAppId() {
    return OswAppWatchfaceFitnessAnalog::APP_ID;
}

const char* OswAppWatchfaceFitnessAnalog::getAppName() {
    return LANG_AFIT;
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

#ifdef GIF_BG
    this->bgGif = new OswAppGifPlayer();
    this->bgGif->setup();
#endif

    this->viewFlags = (OswAppV2::ViewFlags)(this->viewFlags | OswAppV2::ViewFlags::NO_FPS_LIMIT ); // this causes draw() to be called upon every loop() call -> preventing the loss of input events (like the button just went down)
    // Listen to ALL the typical button events (while not processed, it allows animations to be shown for them)

    this->getTwilights();
}

void OswAppWatchfaceFitnessAnalog::onLoop() {
    OswAppV2::onLoop();

    this->needsRedraw = this->needsRedraw or time(nullptr) != this->lastTime; // redraw every second
}

void OswAppWatchfaceFitnessAnalog::onDraw() {
#ifndef OSW_EMULATOR
    unsigned long old_micros = micros();
#else
    unsigned long old_millis = millis();
#endif

    OswAppV2::onDraw();

#ifdef GIF_BG
    if(this->bgGif != nullptr)
        this->bgGif->loop();
#endif

    OswHal* hal = OswHal::getInstance();

    uint32_t second = 0;
    uint32_t minute = 0;
    uint32_t hour = 0;
    bool afterNoon;
    hal->getLocalTime(&hour, &minute, &second, &afterNoon);

    if (screen > 0 && millis() - lastShortPressTime > 5000) {
        screen = 0;
    }

    if (screen == 0) {
        #if OSW_PLATFORM_ENVIRONMENT_ACCELEROMETER == 1
            showFitnessTracking(hal);
        #endif

        drawWatchFace(hal, hour, minute, second, afterNoon);
    } else if (screen == 1) {
        drawDateFace(hal, hour, minute, second, afterNoon);

    } else if (screen == 2) {
        drawFitnessFace(hal, hour, minute, second, afterNoon);
    } else {
        screen = 0;
    }



    this->lastTime = time(nullptr);

#ifndef OSW_EMULATOR
    unsigned long ms_for_onDraw = (micros()-old_micros)/1000;
#else
    unsigned long ms_for_onDraw = millis()-old_millis;
#endif
//    OSW_LOG_I("Time to draw ", ms_for_onDraw, " ms");
}

void OswAppWatchfaceFitnessAnalog::onButton(Button id, bool up, OswAppV2::ButtonStateNames state) {
    OswAppV2::onButton(id, up, state);

    if(!up) return;

    // Process short presses
    if (state == OswAppV2::ButtonStateNames::SHORT_PRESS) {
        if (screen < 3) {
            ++screen;
            lastShortPressTime = millis();
        }
        return;
    }

    // Do only the long press
    if(OswAppWatchface::onButtonDefaults(*this, id, up, state))
        return; // if the button was handled by the defaults, we are done here
}

void OswAppWatchfaceFitnessAnalog::onStop() {
    OswAppV2::onStop(); // always make sure to call the base class method!
            // This is where you de-initialize stuff, gets called when another app is shown

    cleanupLuaState();
#ifdef GIF_BG
    if(this->bgGif != nullptr) {
        this->bgGif->stop();
        delete this->bgGif;
        this->bgGif = nullptr;
    }
#endif
}
