#include <osw_hal.h>
#include <osw_ui.h>
#include <OswAppV2.h>

#ifdef OSW_FEATURE_LUA
#include <osw_hal.h>
#include <osw_lua.h>

#define LUA_APP_PATH FS_MOUNT_POINT "/lua/apps/"
#define LUA_APP_SEARCH_PATH FS_MOUNT_POINT LUA_APP_PATH "?.lua"
#define LUA_LIB_SEARCH_PATH FS_MOUNT_POINT "/lua/libs/?.lua"
#define LUA_PACKAGE_CMD "package.path = package.path .. ';" LUA_APP_SEARCH_PATH ";" LUA_LIB_SEARCH_PATH "'"
#endif


// if you want a cool background image, enable the following define
//#define GIF_BG

#ifdef GIF_BG
class OswAppGifPlayer;
#endif
class OswAppWatchfaceFitnessAnalog : public OswAppV2
{
public:
  constexpr static const char *APP_ID = "osw.wf.afit";

  const char *getAppId() override;
  const char *getAppName() override;

  void onStart();
  void onLoop() override;
  void onDraw() override;
  void onButton(Button id, bool up, ButtonStateNames state) override;
  void onStop() override;

  static uint32_t calculateDistance(uint32_t steps);

  bool getTwilights();
  void cleanupLuaState();
  void printLuaError();

  
  OswAppWatchfaceFitnessAnalog() {
    lastTime = 0;
    screen = 0;
    lastShortPressTime = 0;
    luaState = NULL;
  }

  ~OswAppWatchfaceFitnessAnalog() {
    cleanupLuaState();
  }


private:
  time_t lastTime;
  unsigned screen;
  unsigned long lastShortPressTime;

  lua_State* luaState;
  float sunTimes[12];

  void showFitnessTracking(OswHal *hal);
  void drawWatchFace(OswHal *hal, uint32_t hour, uint32_t minute, uint32_t second, bool afterNoon);
  void drawDateFace(OswHal *hal, uint32_t hour, uint32_t minute, uint32_t second, bool afterNoon);
  void drawFitnessFace(OswHal *hal, uint32_t hour, uint32_t minute, uint32_t second, bool afterNoon);
  void drawTwillightFace(OswHal *hal);

  void timeDisplay(OswHal *hal, uint32_t hour, uint32_t minute, uint32_t second);
  void timeDisplay(OswHal *hal, uint32_t hour, uint32_t minute, uint32_t second, bool afterNoon);
  void dateDisplay(OswHal *hal, uint32_t hour, uint32_t minute, uint32_t second, bool afterNoon);

  
#ifdef GIF_BG
    OswAppGifPlayer* bgGif = nullptr;
#endif
};
