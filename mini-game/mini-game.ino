#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define PLAYER_WIDTH 8
#define PLAYER_HEIGHT 8
#define PLAYER_Y_POSITION 50
#define BUZZER_PIN 18

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
unsigned long lastMillis = 0;
RoadItem leftRoadDetails[3];
RoadItem rightRoadDetails[3];
RoadItem obstacles[4];
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


int calculateXPositionOfRoadDetail(int yPosition, char side)
{
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

  obstacles[0] = RoadItem{32, 16, false};
  obstacles[1] = RoadItem{48, 16, false};
  obstacles[2] = RoadItem{64, 16, false};
  obstacles[3] = RoadItem{80, 16, false};
}

void setup()
{
  Serial.begin(9600);
  setupDisplay();
  setupRoadDetails();
}


void loop()
{
  potentiometerRead = analogRead(A0);

  // Calculate player position
  playerPosition = (int)map(potentiometerRead, 0, 4095, 0, SCREEN_WIDTH - PLAYER_WIDTH - 1);

  // Limit player position in the road
  playerPosition = constrain(playerPosition, 10, display.width()-1-PLAYER_WIDTH-10);
  display.clearDisplay();

  // Draw player
  display.drawBitmap(playerPosition, PLAYER_Y_POSITION, player, PLAYER_WIDTH, PLAYER_HEIGHT, WHITE);

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

  // Spawn obstacles every X miliseconds
  if (millis() - lastMillis > spawnTime) {
    lastMillis = millis();
    // Select a random obstacle to activate
    obstacles[random(0, 4)].active = true;
  }

  // Move obstacles
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

  // Draw obstacles
  for (int i=0; i<4; i++) {
    if (!obstacles[i].active) {
      continue;
    }
    display.drawBitmap(obstacles[i].x, obstacles[i].y, player, PLAYER_WIDTH, PLAYER_HEIGHT, WHITE);
  }

  // Draw Score
  display.setCursor(0, 0);
  display.print("Score: " + String(score));

  // Render the buffer
  display.display();
  delay(15);
}
