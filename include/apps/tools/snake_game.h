#ifndef OSW_APP_SNAKE_GAME_H
#define OSW_APP_SNAKE_GAME_H

#include <osw_hal.h>
#include <osw_ui.h>

#include "osw_app.h"

class OswAppSnakeGame : public OswApp {
 public:
  OswAppSnakeGame(void) { ui = OswUI::getInstance(); };
  void setup(OswHal* hal);
  void loop(OswHal* hal);
  void stop(OswHal* hal);
  ~OswAppSnakeGame(){};

 private:
  OswUI* ui;

#define cellSize 10
#define snakeLength 30
//#define demo 1

  int score = 1;
  int snake[snakeLength][2] = {{10, 10}};

  bool gameRunning = true;
  const int gameWidth = 30;

  const int UP = 0;
  const int DOWN = 1;
  const int RIGHT = 2;
  const int LEFT = 3;
  
  int lastDirection = DOWN;

  int mealXCoord = 10;
  int mealYCoord = 10;

  bool scoreUpdated = false;

  int nbTurns = 0;
  int previousTime = 0;
  int gameStart = 0;

  const int yCells = gameWidth - 30 / cellSize;
  const int squaredWidth = 110 * 110;
  const int halfSize = cellSize / 2;

  float deltaSeconds = 0;

  void drawDirection(OswHal* hal, const int xDirection, const int yDirection);
  void drawDirectionArrow(OswHal* hal, const int direction);
  void drawPlayer(OswHal* hal);
  void drawGameState(OswHal* hal);
  void drawLunch(OswHal* hal);
  void drawGrid(OswHal* hal);
  void drawScore(OswHal* hal);

  // Game logics
  void snakeGame(OswHal* hal);
  void resetGame();
  void proceedEating();
  void spawnEat();
  bool coordsInGame(const int xCoord, const int yCoord);

  // Controls
  void accelerometerController(OswHal* hal, int& xDirection, int& yDirection);
  void autoController(int& xDirection, int& yDirection);

  void waitingRoom(OswHal* hal);

  void proceedSnakeCoords(const bool fastGame, const int xDirection, const int yDirection);
  bool touchItself();
  bool isTimeFinish() { return deltaSeconds * score > 1; }
};

#endif
