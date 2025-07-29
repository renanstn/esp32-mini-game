#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define PLAYER_WIDTH 8
#define PLAYER_HEIGHT 8
#define PLAYER_Y_POSITION 50
#define BUZZER_PIN 18
#define POTENTIOMETER_PIN 36

static const unsigned char PROGMEM player[] = {
  B00011000,
  B00011000,
  B00011000,
  B11111111,
  B00011000,
  B00011000,
  B00011000,
  B11111111,
};

struct RoadItem {
  int x;
  int y;
  bool active;
};

float potentiometerRead;
int playerPosition;
int score = 0;
int spawnTime = 2000;
int obstaclesSpeed = 2;
bool hit = false;
unsigned long lastMillis = 0;
RoadItem leftRoadDetails[3];
RoadItem rightRoadDetails[3];
RoadItem obstacles[4];
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


int calculateXPositionOfRoadDetail(int yPosition, char side) {
  int result;
  if (side == 'l') {
    result = map(yPosition, 16, SCREEN_HEIGHT, 36, 0+6);
  } else if (side == 'r') {
    result = map(yPosition, 16, SCREEN_HEIGHT, SCREEN_WIDTH-1-36, SCREEN_WIDTH-1-6);
  }
  return result;
}

void setupDisplay() {
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.display();
  delay(1000);
  display.clearDisplay();
}

void setupRoadDetails() {
  leftRoadDetails[0] = RoadItem{calculateXPositionOfRoadDetail(16, 'l'), 16, true};
  leftRoadDetails[1] = RoadItem{calculateXPositionOfRoadDetail(32, 'l'), 32, true};
  leftRoadDetails[2] = RoadItem{calculateXPositionOfRoadDetail(48, 'l'), 48, true};
  rightRoadDetails[0] = RoadItem{calculateXPositionOfRoadDetail(16, 'r'), 16, true};
  rightRoadDetails[1] = RoadItem{calculateXPositionOfRoadDetail(32, 'r'), 32, true};
  rightRoadDetails[2] = RoadItem{calculateXPositionOfRoadDetail(48, 'r'), 48, true};
}

void setupObstacles() {
  obstacles[0] = RoadItem{32, 16, false};
  obstacles[1] = RoadItem{48, 16, false};
  obstacles[2] = RoadItem{64, 16, false};
  obstacles[3] = RoadItem{80, 16, false};
}

int getPlayerPosition(int potentiometerRead) {
  return (int)map(potentiometerRead, 0, 4095, 0, SCREEN_WIDTH - PLAYER_WIDTH - 1);
}

void drawPlayer(int playerPosition) {
  display.drawBitmap(
    playerPosition, 
    PLAYER_Y_POSITION, 
    player, 
    PLAYER_WIDTH, 
    PLAYER_HEIGHT, 
    WHITE
  );
}

void drawRoad() {
  // Draw extern road line
  display.drawLine(30, 16, 0, display.height()-1, WHITE);
  display.drawLine(display.width()-30-1, 16, display.width()-1, display.height()-1, WHITE);
  // Draw intern road line
  display.drawLine(31, 16, 0, display.height()-1+6, WHITE);
  display.drawLine(display.width()-31-1, 16, display.width()-1, display.height()-1+6, WHITE);
  // Draw road details
  for (int i=0; i<3; i++) {
    display.drawPixel(leftRoadDetails[i].x, leftRoadDetails[i].y, WHITE);
    display.drawPixel(rightRoadDetails[i].x, rightRoadDetails[i].y, WHITE);
  }
  // Animate road details
  for (int i=0; i<3; i++) {
    leftRoadDetails[i].y += 2;
    leftRoadDetails[i].x = calculateXPositionOfRoadDetail(leftRoadDetails[i].y, 'l');
    rightRoadDetails[i].y += 2;
    rightRoadDetails[i].x = calculateXPositionOfRoadDetail(rightRoadDetails[i].y, 'r');
    // Return details to top screen when they reach bottom
    if (leftRoadDetails[i].y > SCREEN_HEIGHT) {
      leftRoadDetails[i].y = 16;
      leftRoadDetails[i].x = calculateXPositionOfRoadDetail(leftRoadDetails[i].y, 'l');
      rightRoadDetails[i].y = 16;
      rightRoadDetails[i].x = calculateXPositionOfRoadDetail(rightRoadDetails[i].y, 'r');
    }
  }
}

void spawnObstacles() {
  if (millis() - lastMillis > spawnTime) {
    lastMillis = millis();
    // Select a random obstacle to activate
    obstacles[random(0, 4)].active = true;
  }
}

void moveObstacles() {
  for (int i=0; i<4; i++) {
    if (!obstacles[i].active) {
      continue;
    }
    obstacles[i].y += obstaclesSpeed;
    // Turn off and reset obstacles at screen bottom
    if (obstacles[i].y >= SCREEN_HEIGHT) {
      obstacles[i].active = false;
      obstacles[i].y = 16;
      // Increase score and difficulty
      score += 1;
      if (spawnTime > 20) {
        spawnTime -= 20;
      }
    }
  }
}

void drawObstacles() {
  for (int i=0; i<4; i++) {
    if (!obstacles[i].active) {
      continue;
    }
    display.drawBitmap(obstacles[i].x, obstacles[i].y, player, PLAYER_WIDTH, PLAYER_HEIGHT, WHITE);
  }
}

void drawScore() {
  display.setCursor(0, 0);
  display.print("Score: " + String(score));
}

bool isColliding(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {
  return !(x1 + w1 <= x2 || x2 + w2 <= x1 || y1 + h1 <= y2 || y2 + h2 <= y1);
}

bool checkCollisionWithObstacles(int playerPosition) {
  for (int i=0; i<4; i++) {
    RoadItem obstacle = obstacles[i];
    if (isColliding(playerPosition, PLAYER_Y_POSITION, PLAYER_WIDTH, PLAYER_HEIGHT, obstacle.x, obstacle.y, PLAYER_WIDTH, PLAYER_HEIGHT)) {
      return true;
    }
  }
  return false;
}

void gameOver() {
  display.clearDisplay();
  drawScore();
  display.setCursor(0, 16);
  display.print("* * * Game over * * *");
  display.setCursor(0, 32);
  display.print("Press reset button");
  display.setCursor(0, 40);
  display.print("to try again.");
  display.display();
  for(;;);
}

void setup()
{
  // Serial.begin(9600);
  setupDisplay();
  setupRoadDetails();
  setupObstacles();
}

void loop()
{
  potentiometerRead = analogRead(POTENTIOMETER_PIN);
  playerPosition = getPlayerPosition(potentiometerRead);
  playerPosition = constrain(playerPosition, 10, display.width()-1-PLAYER_WIDTH-10);
  display.clearDisplay();
  drawPlayer(playerPosition);
  drawRoad();
  spawnObstacles();
  moveObstacles();
  hit = checkCollisionWithObstacles(playerPosition);
  if (hit) {
    gameOver();
  }
  drawObstacles();
  drawScore();
  display.display();
  delay(15);
}
