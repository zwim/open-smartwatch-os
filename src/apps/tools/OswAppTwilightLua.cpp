
#include "./apps/tools/OswAppTwilightLua.h"

#include "assets/img/icons/osw.png.h"


#include <gfx_util.h>
#include <OswAppV1.h>
#include <osw_hal.h>

const int middleX = 120;
const int middleY = 120;
const int screenWidth = 240;

void OswAppTwilightLua::onStart() {}


void OswAppTwilightLua::onDraw() {
    drawTwilightInfo();
}


const char* OswAppTwilightLua::getAppId() {
    return "osw.tutorial";
}

const char* OswAppTwilightLua::getAppName() {
    return "OSW Tutorial";
}

const OswIcon& OswAppTwilightLua::getAppIcon() {
    return osw_png;
}


void OswAppTwilightLua::cleanupLuaState() {
    if (luaState) {
        lua_close(luaState);
        luaState = NULL;
    }
}

inline void OswAppTwilightLua::printLuaError() {
    if (luaState) 
        OSW_LOG_E(lua_tostring(luaState, -1));
}

bool OswAppTwilightLua::getTwilights() {
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
        OSW_LOG_E("Oh my god, the lua stack has grown from %d to %d\n", luaInitialStackSize, lua_gettop(luaState));
    }

        OSW_LOG_I("xxx Oh my god, the lua stack has grown from %d to %d\n", luaInitialStackSize, lua_gettop(luaState));

    return true;
}


void OswAppTwilightLua::drawTwilightInfo() {
    const int fontHeight = 1;
    const int lineHeight = 10;
    const int x_start = 40;
    const int y_start = 40;
    OswHal* hal = OswHal::getInstance();

    hal->gfx()->setTextSize(fontHeight);

    ui->resetTextColors();
    hal->gfx()->setTextCursor(x_start, y_start + 0 * lineHeight);
    hal->gfx()->printf("Rise: %f", sunTimes[5]);

    hal->gfx()->setTextCursor(x_start, y_start + 1 * lineHeight);
    hal->gfx()->print("Y: ");

    hal->gfx()->setTextCursor(x_start, y_start + 2 * lineHeight);
    hal->gfx()->print("Y: ");

    hal->gfx()->setTextCursor(x_start, y_start + 3 * lineHeight);
    hal->gfx()->print("Z: ");
}


void OswAppTwilightLua::onLoop() {
    OswHal* hal = OswHal::getInstance();

    // to better understand the accelerometer values use the debug function
    // debug(hal);



}

void OswAppTwilightLua::onStop() {
    cleanupLuaState();
}
