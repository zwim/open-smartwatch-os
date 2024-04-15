#pragma once
#include "./apps/tools/OswAppTwilightLua.h"

#include <gfx_util.h>
#include <OswAppV2.h>
#include <osw_hal.h>

#ifdef OSW_FEATURE_LUA
#include <osw_lua.h>

#define LUA_APP_PATH FS_MOUNT_POINT "/lua/apps/"
#define LUA_APP_SEARCH_PATH FS_MOUNT_POINT LUA_APP_PATH "?.lua"
#define LUA_LIB_SEARCH_PATH FS_MOUNT_POINT "/lua/libs/?.lua"
#define LUA_PACKAGE_CMD "package.path = package.path .. ';" LUA_APP_SEARCH_PATH ";" LUA_LIB_SEARCH_PATH "'"
#endif

class OswAppTwilightLua : public OswAppV2 {
  public:
    //OswAppTwilightLua();
    //virtual ~OswAppTwilightLua();

    const char* getAppId() override;
    const char* getAppName() override;
    const OswIcon& getAppIcon() override;

    void onStart() override;
    void onLoop() override;
    void onDraw() override;
    void onStop() override;


void cleanupLuaState();
void printLuaError();
bool getTwilights();
void drawTwilightInfo();

    bool changeRootAppIfNecessary();
  private:
    OswAppV2* previousRootApp = nullptr;
    static OswIconProgmem oswIcon;

    lua_State* luaState;
    float sunTimes[12];
};

